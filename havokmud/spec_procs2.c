#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"

/*
 * external vars 
 */


extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern int      top_of_world;
extern struct int_app_type int_app[26];

extern const struct class_def classes[MAX_CLASS];
extern char    *dirs[];

extern int      gSeason;

int             monkpreproom = 550;
int             druidpreproom = 500;


/*
 * extern procedures 
 */

#define MONK_CHALLENGE_ROOM 551
#define DRUID_CHALLENGE_ROOM 501

#define Bandits_Path   2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10
#define Fountain_Level 20

#define CMD_SAY 17
#define CMD_ASAY 169

#define TONGUE_ITEM 22

#define GIVE 72
#define GAIN 243

#define PRISON_ROOM 2639
#define PRISON_LET_OUT_ROOM 2640

#define HOLDING_MAX  10         /* max mobs that can be in tank :) */
#define HOLDING_TANK 60         /* room number to drop the mobs in */

#define NOD  35
#define DRUID_MOB 600
#define MONK_MOB  650
#define FLEE 151

#define ENTER 7
#define ATTACK_ROOM 3004

#define WHO_TO_HUNT  6112       /* green dragon */
#define WHERE_TO_SIT 3007       /* tavern */
#define WHO_TO_CALL  3063       /* mercenary */

#define AST_MOB_NUM 2715

int ghost(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting && 
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                          ch->specials.fighting, 0);

        return TRUE;
    }
    return FALSE;
}

int druid_protector(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    static int      b = 1;      
    /* 
     * use this as a switch, to avoid double
     * challenges 
     */

    if (cmd) {
        if (cmd <= 6 && cmd >= 1 && IS_PC(ch)) {
            if (b) {
                b = 0;
                send_to_char("Basil Great Druid looks at you\n\r", ch);
                if (ch->in_room == Bandits_Path && cmd == 1 &&
                    BASIL_GATEKEEPER_MAX_LEVEL < GetMaxLevel(ch) &&
                    !IS_IMMORTAL(ch)) {
                    if (!check_soundproof(ch)) {
                        act("Basil the Great Druid tells you 'Begone "
                            "Unbelievers!'", TRUE, ch, 0, 0, TO_CHAR);
                    }
                    act("Basil Great Druid grins evilly.", TRUE, ch, 0,
                        0, TO_CHAR);
                    return (TRUE);
                }
            } else {
                b = 1;
            }
            return (FALSE);
        }                       
        /* 
         * cmd 1 - 6 
         */
        return (FALSE);
    } 
    
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        } else {
            FighterMove(ch);
        }
    }
    return (FALSE);
}

int Magic_Fountain(struct char_data *ch, int cmd, char *arg,
                   struct room_data *rp, int type)
{

    char           *buf;

    if (cmd == 11) {
        /* 
         * drink 
         */

        arg = get_argument(arg, &buf);

        if (!buf || (strcasecmp(buf, "fountain") && strcasecmp(buf, "water"))) {
            return (FALSE);
        }

        send_to_char("You drink from the fountain\n\r", ch);
        act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);

        if (GET_COND(ch, THIRST) > 20) {
            act("You do not feel thirsty.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        if (GET_COND(ch, FULL) > 20) {
            act("You do are full.", FALSE, ch, 0, 0, TO_CHAR);
            return (TRUE);
        }

        GET_COND(ch, THIRST) = 24;
        GET_COND(ch, FULL) += 1;

        switch (number(0, 40)) {

            /*
             * Lets try and make 1-10 Good, 11-26 Bad, 27-40 Nothing 
             */
        case 1:
            cast_refresh(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 2:
            cast_stone_skin(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
            cast_cure_serious(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 4:
            cast_cure_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 5:
            cast_armor(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
            cast_bless(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 7:
            cast_invisibility(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 8:
            cast_strength(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 9:
            cast_remove_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 10:
            cast_true_seeing(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

            /*
             * Time for the nasty Spells 
             */

        case 11:
            cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 12:
            cast_teleport(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 13:
            cast_web(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 14:
            cast_curse(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 15:
            cast_blindness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 16:
            cast_weakness(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
            cast_poison(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 18:
            cast_cause_light(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 19:
            cast_cause_critic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 20:
            cast_dispel_magic(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 21:
            cast_magic_missile(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 22:
            cast_faerie_fire(Fountain_Level, ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 23:
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 24:
            cast_burning_hands(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            break;
        case 25:
            cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 26:
            cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

        default:
            send_to_char("The fluid tastes like dry sand in your mouth.\n\r",
                         ch);
            break;
        }
        return (TRUE);
    }

    return (FALSE);
}


int DruidAttackSpells(struct char_data *ch, struct char_data *vict,
                      int level)
{
    switch (level) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        act("$n utters the words 'yow!'", 1, ch, 0, 0, TO_ROOM);
        cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        if (!IS_SET(vict->M_immune, AFF_POISON) &&
            !IS_AFFECTED(vict, AFF_POISON)) {
            act("$n utters the words 'yuk'", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'ouch'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
        }
        return (FALSE);
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        act("$n utters the words 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
        cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
        if (!IS_SET(vict->M_immune, IMM_FIRE)) {
            act("$n utters the words 'fwoosh'", 1, ch, 0, 0, TO_ROOM);
            cast_firestorm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
        }
        return (FALSE);
        break;
    default:
        act("$n utters the words 'kazappapapapa'", 1, ch, 0, 0, TO_ROOM);
        cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    }
}

int Summoner(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *targ = 0;
    struct char_list *i;
    char            buf[255];
    extern char     EasySummon;
    char           *name;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (TRUE);
    }
    /*
     * wait till at 75% of hitpoints. 
     */
    if (GET_HIT(ch) > ((GET_MAX_HIT(ch) * 3) / 4)) {
        /*
         * check for hatreds 
         */
        if (IS_SET(ch->hatefield, HATE_CHAR) && ch->hates.clist) {
            for (i = ch->hates.clist; i; i = i->next) {
                if (i->op_ch) {
                    /* 
                     * if there is a char_ptr 
                     */
                    targ = i->op_ch;
                    if (IS_PC(targ)) {
                        sprintf(buf, "You hate %s\n\r", targ->player.name);
                        send_to_char(buf, ch);
                        break;
                    }
                }
            }
        }

        if (targ) {
            name = strdup(targ->player.name);
            act("$n utters the words 'Your ass is mine!'", 1, ch, 0, 0,
                TO_ROOM);
            if (EasySummon == 1) {
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_summon(GetMaxLevel(ch), ch, targ, 0);
                } else if (GET_MAX_HIT(targ) <= GET_HIT(ch)) {
                    ch->skills[SKILL_SUMMON].learned = 100;
                    sprintf( buf, "summon %s", name );
                    command_interpreter(ch, buf);
                    ch->points.mana = 100;
                } else {
                    ch->skills[SKILL_PORTAL].learned = 100;
                    sprintf( buf, "portal %s", name );
                    command_interpreter(ch, buf);
                    ch->points.mana = 100;
                }
            } else {
                if (GetMaxLevel(ch) > 32 && number(0, 3)) {
                    command_interpreter(ch, "say Curses!  Foiled again!\n\r");
                    if( name ) {
                        free( name );
                    }
                    return (0);
                }

                /*
                 * Easy Summon was turned off and they were > 32nd level 
                 * so we portal to them! 
                 */
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_portal(GetMaxLevel(ch), ch, targ, 0);
                    command_interpreter(ch, "enter portal");
                } else {
                    /* 
                     * its a psi summoner, do his stuff
                     *
                     * with easy_summon turned off must portal, so.. 
                     */
                    if (!ch->skills[SKILL_PORTAL].learned) {
                        ch->skills[SKILL_PORTAL].learned = 100;
                    }
                    sprintf(buf, "portal %s", name);
                    command_interpreter(ch, targ->player.name);
                    ch->points.mana = 100;
                }
            }

            if (targ->in_room == ch->in_room) {
                if (NumCharmedFollowersInRoom(ch) > 0) {
                    sprintf(buf, "order followers kill %s", GET_NAME(targ));
                    command_interpreter(ch, buf);
                }
                act("$n says, 'And now my young $N... You will DIE!", 0,
                    ch, 0, targ, TO_ROOM);
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_dispel_magic(GetMaxLevel(ch), ch, targ, 0);
                } else {
                    sprintf(buf, "blast %s", name);
                    command_interpreter(ch, buf);
                }
            }

            if( name ) {
                free( name );
            }
            return (FALSE);
        } else {
            return (FALSE);
        }
    } else {
        return (FALSE);
    }
}
/*---------------end of summoner---------------*/

int monk(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        MonkMove(ch);
    }
    return (FALSE);
}


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

int magic_user(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;

    byte            lspell;
    char            buf[200];

    if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    if (!MobCastCheck(ch, 0)) {
#if 0        
        if(ch->specials.fighting) { 
            act("$n reevaluates his priorities, and decides it's"
                " best to get out.", 1, ch, 0, 0, TO_ROOM);
            do_flee(ch, NULL, 0); 
        } else { 
            /* 
             * mages get funky skillz 
             */
             act("$n quickly prepares a potion of dispel magic, "
                 "and quaffs it!", 1, ch, 0, 0, TO_ROOM); 
             spell_dispel_magic(level, ch, ch, 0); } 
#endif         
        return (TRUE);
    }

    if (!ch->specials.fighting && !IS_PC(ch)) {
        if (GET_POS(ch) > POSITION_STUNNED && GET_POS(ch) < POSITION_FIGHTING) {
            StandUp(ch);
            return (TRUE);
        }
        SET_BIT(ch->player.class, CLASS_MAGIC_USER);

        if (GetMaxLevel(ch) >= 25 && !ch->desc) {
            if (Summoner(ch, cmd, arg, mob, type)) {
                return (TRUE);
            } else if (!IS_SET(real_roomp(ch->in_room)->room_flags, NO_SUM) && 
                NumCharmedFollowersInRoom(ch) < 4 && 
                IS_SET(ch->hatefield, HATE_CHAR)) {
                act("$n utters the words 'Here boy!'", 1, ch, 0, 0, TO_ROOM);
                cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                command_interpreter(ch, "order followers guard on");
                return (TRUE);
            }
        }

#ifdef PREP_SPELLS

        if (!ch->desc) {
            /* 
             * make sure it is a mob not a pc
             *
             * low level prep spells here 
             */
            if (!affected_by_spell(ch, SPELL_SHIELD)) {
                act("$n utters the words 'dragon'", 1, ch, 0, 0, TO_ROOM);
                cast_shield(GetMaxLevel(ch), ch, GET_NAME(ch),
                            SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_STRENGTH)) {
                act("$n utters the words 'giant'", 1, ch, 0, 0, TO_ROOM);
                cast_strength(GetMaxLevel(ch), ch, GET_NAME(ch),
                              SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!IS_EVIL(ch) && 
                !affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL)) {
                act("$n utters the words 'anti-evil'", 1, ch, 0, 0, TO_ROOM);
                cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if ((IS_AFFECTED(ch, AFF_FIRESHIELD) || 
                 IS_AFFECTED(ch, AFF_SANCTUARY)) && 
                !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) && 
                !IS_AFFECTED(ch, AFF_DARKNESS)) {
                act("$n utters the words 'darkness'", 1, ch, 0, 0, TO_ROOM);
                cast_globe_darkness(GetMaxLevel(ch), ch, GET_NAME(ch),
                                    SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_ARMOR)) {
                act("$n utters the words 'dragon'", 1, ch, 0, 0, TO_ROOM);
                cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * high level prep spells here 
             */
            if (GetMaxLevel(ch) >= 25) {
                if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
                    act("$n utters the words 'stone'", 1, ch, 0, 0, TO_ROOM);
                    cast_stone_skin(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_GLOBE_MINOR_INV)) {
                    act("$n utters the words 'haven'", 1, ch, 0, 0, TO_ROOM);
                    cast_globe_minor_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_GLOBE_MAJOR_INV)) {
                    act("$n utters the words 'super haven'", 1, ch, 0, 0,
                        TO_ROOM);
                    cast_globe_major_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) >= 40 &&
                    !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                    !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                    !affected_by_spell(ch, SPELL_CHILLSHIELD) && 
                    !number(0, 2)) {
                    act("$n utters the words 'crackle'", 1, ch, 0, 0, TO_ROOM);
                    cast_fireshield(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }
            return (FALSE);
        }
#endif
    }

    if (!ch->specials.fighting) {
        return (FALSE);
    }
    if (!IS_PC(ch) && GET_POS(ch) > POSITION_STUNNED && 
        GET_POS(ch) < POSITION_FIGHTING) {
        if (GET_HIT(ch) > GET_HIT(ch->specials.fighting) / 2) {
            StandUp(ch);
        } else {
            StandUp(ch);
            do_flee(ch, NULL, 0);
        }
        return (TRUE);
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
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
        lspell += GetMaxLevel(ch) / 5;
    }

    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
        act("$n utters the words 'Let me see the light!'", TRUE, ch, 0, 0,
            TO_ROOM);
        cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 10 && 
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        act("$n utters the words 'Use MagicAway Instant Magic Remover'", 1,
            ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
    }

    if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 10 && 
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        act("$n utters the words 'Use MagicAway Instant Magic Remover'", 1,
            ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 28 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n checks $s watch.", TRUE, ch, 0, 0, TO_ROOM);
        act("$n utters the words 'Oh my, would you just LOOK at the time!'", 
            1, ch, 0, 0, TO_ROOM);
        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
            return (TRUE);
        }
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 15 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n utters the words 'Woah! I'm outta here!'", 1, ch, 0, 0,
            TO_ROOM);
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    if (GET_HIT(ch) > GET_MAX_HIT(ch) / 2 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE) && 
        GetMaxLevel(vict) < GetMaxLevel(ch) && number(0, 1)) {
        if (lspell > 8 && lspell < 50 && !number(0, 6)) {
            act("$n utters the words 'Icky Sticky!'", 1, ch, 0, 0, TO_ROOM);
            cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 6)) {
            act("$n utters the words 'You wimp'", 1, ch, 0, 0, TO_ROOM);
            cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 7)) {
            act("$n utters the words 'Bippety boppity Boom'", 1, ch, 0, 0,
                TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return TRUE;
        }

        if (lspell > 12 && lspell < 20 && !number(0, 7)) {
            act("$n utters the words '&#%^^@%*#'", 1, ch, 0, 0, TO_ROOM);
            cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 10 && lspell < 20 && !number(0, 5)) {
            act("$n utters the words 'yabba dabba do'", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 8 && lspell < 40 && !number(0, 5) && 
            vict->specials.fighting != ch) {
            act("$n utters the words 'You are getting sleepy'", 1, ch, 0,
                0, TO_ROOM);
            cast_charm_monster(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);

            if (IS_AFFECTED(vict, AFF_CHARM)) {
                if (!vict->specials.fighting) {
                    sprintf(buf, "order %s kill %s", GET_NAME(vict),
                            GET_NAME(ch->specials.fighting));
                } else {
                    sprintf(buf, "order %s remove all", GET_NAME(vict));
                }
                command_interpreter(ch, buf);
            }
        }
    } else {
        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'bang! bang! pow!'", 1, ch, 0, 0, TO_ROOM);
            cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
            break;
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'ZZZZzzzzzzTTTT'", 1, ch, 0, 0, TO_ROOM);
            cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            break;
        case 6:
        case 7:
        case 8:
            if (ch->attackers <= 2) {
                act("$n utters the words 'Icky Sticky!'", 1, ch, 0, 0, TO_ROOM);
                cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Fwoosh!'", 1, ch, 0, 0, TO_ROOM);
                cast_burning_hands(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 9:
        case 10:
            act("$n utters the words 'SPOOGE!'", 1, ch, 0, 0, TO_ROOM);
            cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 11:
        case 12:
        case 13:
            if (ch->attackers <= 2) {
                act("$n utters the words 'KAZAP!'", 1, ch, 0, 0, TO_ROOM);
                cast_lightning_bolt(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Ice Ice Baby!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
            }
            break;
        case 14:
        case 15:
            act("$n utters the words 'Ciao!'", 1, ch, 0, 0, TO_ROOM);
            cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 16:
        case 17:
        case 18:
        case 19:
            act("$n utters the words 'maple syrup'", 1, ch, 0, 0, TO_ROOM);
            cast_slow(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 20:
        case 21:
        case 22:
            if (IS_EVIL(ch)) {
                act("$n utters the words 'slllrrrrrrpppp'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_energy_drain(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
            if (ch->attackers <= 2) {
                act("$n utters the words 'Look! A rainbow!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_colour_spray(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Get the sensation!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_cone_of_cold(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
            act("$n utters the words 'Hasta la vista, Baby'", 1, ch, 0, 0,
                TO_ROOM);
            cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 36:
        case 37:
            act("$n utters the words 'KAZAP KAZAP KAZAP!'", 1, ch, 0, 0,
                TO_ROOM);
            cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 38:
            act("$n utters the words 'duhhh'", 1, ch, 0, 0, TO_ROOM);
            cast_feeblemind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 39:
            act("$n utters the words 'STOP'", 1, ch, 0, 0, TO_ROOM);
            cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 40:
        case 41:
            if (ch->attackers <= 2) {
                act("$n utters the words 'frag'", 1, ch, 0, 0, TO_ROOM);
                cast_meteor_swarm(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Whew, whata smell!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_incendiary_cloud(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        default:
            if (ch->attackers <= 2) {
                act("$n utters the words 'ZZAAPP!'", 1, ch, 0, 0, TO_ROOM);
                cast_disintegrate(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Whew, whata smell!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_incendiary_cloud(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        }
    }
    return TRUE;
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
            } else if (obj_index[(ch->equipment[WEAR_EYES])->item_number].
                       virtual == TONGUE_ITEM) {
                act("$n utters the words 'oh golly'", 1, ch, 0, 0, TO_ROOM);
                cast_scourge_warlock(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
            } else {
                obj_to_char(unequip_char(ch, WEAR_EYES), ch);
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

int cleric(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *vict;
    byte            lspell,
                    healperc = 0;

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
    if (!ch->specials.fighting) {
        if (GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
            if ((lspell = GetMaxLevel(ch)) >= 20) {
                act("$n utters the words 'What a Rush!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            } else if (lspell > 12) {
                act("$n utters the words 'Woah! I feel GOOD! Heh.'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            } else if (lspell > 8) {
                act("$n utters the words 'I feel much better now!'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_serious(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
            } else {
                act("$n utters the words 'I feel good!'", 1, ch, 0, 0, TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
            }
        }

#ifdef PREP_SPELLS
        if (!ch->desc) {
            /* 
             * make sure it is a mob 
             *
             * low level prep 
             */
            if (!affected_by_spell(ch, SPELL_ARMOR)) {
                act("$n utters the words 'protect'", FALSE, ch, 0, 0, TO_ROOM);
                cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_BLESS)) {
                act("$n utters the words 'bless'", FALSE, ch, 0, 0, TO_ROOM);
                cast_bless(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_AID)) {
                act("$n utters the words 'aid'", FALSE, ch, 0, 0, TO_ROOM);
                cast_aid(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_DETECT_MAGIC)) {
                act("$n utters the words 'detect magic'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_detect_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL) && 
                !IS_EVIL(ch)) {
                act("$n utters the words 'anti evil'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (GetMaxLevel(ch) > 24) {
                if (!affected_by_spell(ch, SPELL_PROT_FIRE)) {
                    act("$n utters the words 'resist fire'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_prot_fire(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_COLD)) {
                    act("$n utters the words 'resist cold'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_prot_cold(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ENERGY)) {
                    act("$n utters the words 'resist energy'", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_prot_energy(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ELEC)) {
                    act("$n utters the words 'resist electricity'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_prot_elec(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
                
                if (GetMaxLevel(ch) > 44 &&
                    !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                    !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                    !affected_by_spell(ch, SPELL_CHILLSHIELD) && 
                    !number(0, 2)) {
                    act("$n utters the words 'butcher's blade'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_blade_barrier(GetMaxLevel(ch), ch, "",
                                       SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
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
                act("$n utters the words 'cure blind'", FALSE, ch, 0, 0,
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
                    act("$n utters the words 'remove curse'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_remove_curse(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (affected_by_spell(ch, SPELL_PARALYSIS)) {
                    act("$n utters the words 'remove paralysis'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_remove_paralysis(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
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
        act("$n utters the words 'Woah! I'm outta here!'", 1, ch, 0, 0,
            TO_ROOM);
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
        /* 
         * do harm 
         *
         * call lightning 
         */
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 && 
            !number(0, 5)) {
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
            act("$n utters the words 'muhahaha!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            act("$n utters the words 'hocus pocus'", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 7:
            act("$n utters the words 'Va-Voom!'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 8:
            act("$n utters the words 'urgle blurg'", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            act("$n utters the words 'take that!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 11:
            act("$n utters the words 'burn baby burn'", 1, ch, 0, 0, TO_ROOM);
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!IS_SET(vict->M_immune, IMM_FIRE)) {
                act("$n utters the words 'burn baby burn'", 1, ch, 0,
                    0, TO_ROOM);
                cast_flamestrike(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_SANCTUARY)
                       && (GetMaxLevel(ch) > GetMaxLevel(vict))) {
                act("$n utters the words 'Va-Voom!'", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'take that!'", 1, ch, 0, 0, TO_ROOM);
                cast_cause_critic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 17:
        case 18:
        case 19:
        default:
            act("$n utters the words 'Hurts, doesn't it?'", 1, ch, 0, 0,
                TO_ROOM);
            cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        }

        return (TRUE);
    } else {
        /*
         * do heal 
         */

        if (IS_AFFECTED(ch, AFF_BLIND) && lspell >= 4 && !number(0, 3)) {
            act("$n utters the words 'Praise Celestian, I can SEE!'", 1,
                ch, 0, 0, TO_ROOM);
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
            act("$n utters the words 'Praise Dog, I don't feel sick no more!'",
                1, ch, 0, 0, TO_ROOM);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'abrazak'", 1, ch, 0, 0, TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
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
            act("$n utters the words 'Woah! I feel GOOD! Heh.'", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
        case 18:
            act("$n utters the words 'What a Rush!'", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            act("$n utters the words 'Oooh, pretty!'", 1, ch, 0, 0, TO_ROOM);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

        }

        return (TRUE);
    }
}


int RepairGuy(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];
    int             cost,
                    ave;
    struct char_data *vict;
    struct obj_data *obj;
    struct obj_data *new;

    /* 
     * special procedure for this mob/obj 
     */
    int             (*rep_guy) ();

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    rep_guy = RepairGuy;

    if (IS_NPC(ch)) {
        if (cmd == 72) {
            arg = get_argument(arg, &obj_name);
            if (!obj_name || !(obj = get_obj_in_list_vis(ch, obj_name,
                                                         ch->carrying))) {
                return (FALSE);
            }

            arg = get_argument(arg, &vict_name);
            if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) ||
                !IS_NPC(vict)) {
                return (FALSE);
            }

            if (mob_index[vict->nr].func == rep_guy) {
                send_to_char("Nah, you really wouldn't want to do that.", ch);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
    }

    if (cmd == 72) {
        /* 
         * give 
         *
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name || !(obj = get_obj_in_list_vis(ch, obj_name,
                                                     ch->carrying))){
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * the target is the repairman, or an NPC 
         */
        if (!IS_NPC(vict)) {
            return (FALSE);
        }
        if (mob_index[vict->nr].func == rep_guy) {
            /*
             * we have the repair guy, and we can give him the stuff 
             */
            act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
        } else {
            return (FALSE);
        }

        act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

        /*
         * make all the correct tests to make sure that everything is
         * kosher 
         */

        if (ITEM_TYPE(obj) == ITEM_ARMOR && obj->obj_flags.value[1] > 0) {
            if (obj->obj_flags.value[1] > obj->obj_flags.value[0]) {
                cost = obj->obj_flags.cost;
                cost /= obj->obj_flags.value[1];
                cost *= (obj->obj_flags.value[1] - obj->obj_flags.value[0]);

                if (GetMaxLevel(vict) > 25) {
                    /* 
                     * super repair guy 
                     */
                    cost *= 2;
                }

                if (cost > GET_GOLD(ch)) {
                    if (check_soundproof(ch)) {
                        act("$N shakes $S head.\n\r",
                            TRUE, ch, 0, vict, TO_ROOM);
                        act("$N shakes $S head.\n\r",
                            TRUE, ch, 0, vict, TO_CHAR);
                    } else {
                        act("$N says 'I'm sorry, you don't have enough money.'",
                            TRUE, ch, 0, vict, TO_ROOM);
                        act("$N says 'I'm sorry, you don't have enough money.'",
                            TRUE, ch, 0, vict, TO_CHAR);
                    }
                } else {
                    GET_GOLD(ch) -= cost;

                    sprintf(buf, "You give $N %d coins.", cost);
                    act(buf, TRUE, ch, 0, vict, TO_CHAR);
                    act("$n gives some money to $N.", TRUE, ch, obj, vict,
                        TO_ROOM);

                    /*
                     * fix the armor 
                     */
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
                    if (GetMaxLevel(vict) > 25) {
                        obj->obj_flags.value[0] = obj->obj_flags.value[1];
                    } else {
                        ave = MAX(obj->obj_flags.value[0],
                                  (obj->obj_flags.value[0] +
                                   obj->obj_flags.value[1]) / 2);
                        obj->obj_flags.value[0] = ave;
                        obj->obj_flags.value[1] = ave;
                    }

                    if (check_soundproof(ch)) {
                        act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
                        act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
                    } else {
                        act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
                        act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
                    }
                }
            } else if (check_soundproof(ch)) {
                act("$N shrugs.", TRUE, ch, 0, vict, TO_ROOM);
                act("$N shrugs.", TRUE, ch, 0, vict, TO_CHAR);
            } else {
                act("$N says 'Your armor looks fine to me.'",
                    TRUE, ch, 0, vict, TO_ROOM);
                act("$N says 'Your armor looks fine to me.'",
                    TRUE, ch, 0, vict, TO_CHAR);
            }
        } else if (GetMaxLevel(vict) < 25 || ITEM_TYPE(obj) != ITEM_WEAPON) {
            if (check_soundproof(ch)) {
                act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
                act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
            } else if (ITEM_TYPE(obj) != ITEM_ARMOR) {
                act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_ROOM);
                act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_CHAR);
            } else {
                act("$N says 'I can't fix that...'", TRUE, ch, 0,
                    vict, TO_CHAR);
                act("$N says 'I can't fix that...'", TRUE, ch, 0,
                    vict, TO_ROOM);
            }
        } else {
            /*
             * weapon repair.  expensive! 
             */
            cost = obj->obj_flags.cost;
            new = read_object(obj->item_number, REAL);
            if (obj->obj_flags.value[2]) {
                cost /= obj->obj_flags.value[2];
            }

            cost *= (new->obj_flags.value[2] - obj->obj_flags.value[2]);

            if (cost > GET_GOLD(ch)) {
                if (check_soundproof(ch)) {
                    act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
                } else {
                    act("$N says 'I'm sorry, you don't have enough money.'",
                        TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'I'm sorry, you don't have enough money.'",
                        TRUE, ch, 0, vict, TO_CHAR);
                    extract_obj(new);
                }
            } else {
                GET_GOLD(ch) -= cost;

                sprintf(buf, "You give $N %d coins.", cost);
                act(buf, TRUE, ch, 0, vict, TO_CHAR);
                act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

                /*
                 * fix the weapon 
                 */
                act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
                act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);

                obj->obj_flags.value[2] = new->obj_flags.value[2];
                extract_obj(new);

                if (check_soundproof(ch)) {
                    act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
                } else {
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
                }
            }
        }

        act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
        return (TRUE);
    }

    if (cmd) {
        return FALSE;
    }
    return (fighter(ch, cmd, arg, mob, type));
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

int AbyssGateKeeper(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            fighter(ch, cmd, arg, mob, type);
        }
    } else if (cmd == 5) {
        send_to_char("The gatekeeper shakes his head, and blocks your way.\n\r",
                     ch);
        act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0,
            0, TO_ROOM);
        return (TRUE);
    }
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

void Submit(struct char_data *ch, struct char_data *t)
{
    char            buf[200];

    switch (number(1, 5)) {
    case 1:
        sprintf(buf, "bow %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 2:
        sprintf(buf, "smile %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 3:
        sprintf(buf, "wink %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 4:
        sprintf(buf, "wave %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    default:
        act("$n nods $s head at you", 0, ch, 0, t, TO_VICT);
        act("$n nods $s head at $N", 0, ch, 0, t, TO_NOTVICT);
        break;
    }
}

void SayHello(struct char_data *ch, struct char_data *t)
{
    char            buf[200];
    char            buf2[80];

    switch (number(1, 10)) {
    case 1:
        command_interpreter(ch, "say Greetings, adventurer");
        break;
    case 2:
        if (t->player.sex == SEX_FEMALE) {
            command_interpreter(ch, "say Good day, milady");
        } else {
            command_interpreter(ch, "say Good day, lord");
        }
        break;
    case 3:
        if (t->player.sex == SEX_FEMALE) {
            command_interpreter(ch, "say Pleasant Journey, Mistress");
        } else {
            command_interpreter(ch, "say Pleasant Journey, Master");
        }
        break;
    case 4:
        if (t->player.sex == SEX_FEMALE) {
            sprintf(buf, "say Make way!  Make way for the lady %s!",
                    GET_NAME(t));
        } else {
            sprintf(buf, "say Make way!  Make way for the lord %s!",
                    GET_NAME(t));
        }
        command_interpreter(ch, buf);
        break;
    case 5:
        command_interpreter(ch, "say May the prophet smile upon you");
        break;
    case 6:
        command_interpreter(ch, "say It is a pleasure to see you again.");
        break;
    case 7:
        command_interpreter(ch, "say You are always welcome here, great one");
        break;
    case 8:
        command_interpreter(ch, "say My lord bids you greetings");
        break;
    case 9:
        if (time_info.hours > 6 && time_info.hours < 12) {
            sprintf(buf, "say Good morning, %s", GET_NAME(t));
        } else if (time_info.hours >= 12 && time_info.hours < 20) {
            sprintf(buf, "say Good afternoon, %s", GET_NAME(t));
        } else if (time_info.hours >= 20 && time_info.hours <= 24) {
            sprintf(buf, "say Good evening, %s", GET_NAME(t));
        } else {
            sprintf(buf, "say Up for a midnight stroll, %s?\n", GET_NAME(t));
        }
        command_interpreter(ch, buf);
        break;
    case 10:
        if (time_info.hours < 6) {
            strcpy(buf2, "evening");
        } else if (time_info.hours < 12) {
            strcpy(buf2, "morning");
        } else if (time_info.hours < 20) {
            strcpy(buf2, "afternoon");
        } else {
            strcpy(buf2, "evening");
        }

        switch (weather_info.sky) {
        case SKY_CLOUDLESS:
            sprintf(buf, "say Lovely weather we're having this %s, isn't it, "
                         "%s.", buf2, GET_NAME(t));
        case SKY_CLOUDY:
            sprintf(buf, "say Nice %s to go for a walk, %s.", buf2, 
                    GET_NAME(t));
            break;
        case SKY_RAINING:
            sprintf(buf, "say I hope %s's rain clears up.. don't you %s?",
                    buf2, GET_NAME(t));
            break;
        case SKY_LIGHTNING:
            sprintf(buf, "say How can you be out on such a miserable %s, %s!",
                    buf2, GET_NAME(t));
            break;
        default:
            sprintf(buf, "say Such a pleasant %s, don't you think?", buf2);
            break;
        }
        command_interpreter(ch, buf);
        break;
    }
}

void GreetPeople(struct char_data *ch)
{
    struct char_data *tch;

    if (!IS_SET(ch->specials.act, ACT_GREET)) {
        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (!IS_NPC(tch) && !number(0, 8) && 
                GetMaxLevel(tch) > GetMaxLevel(ch) && CAN_SEE(ch, tch)) {
                Submit(ch, tch);
                SayHello(ch, tch);
                SET_BIT(ch->specials.act, ACT_GREET);
                break;
            }
        }
    } else if (!number(0, 100)) {
        REMOVE_BIT(ch->specials.act, ACT_GREET);
    }
}

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
                    sprintf(buf, "Removing MURDER and STOLE bit from %s.",
                            GET_NAME(t));
                    Log(buf);
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
                    sprintf(buf, "Removing STOLE and MURDER bit from %s.",
                            GET_NAME(t));
                    Log(buf);

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

int GenericCityguardHateUndead(struct char_data *ch, int cmd, char *arg,
                               struct char_data *mob, int type)
{
    struct char_data *tch,
                   *evil;
    int             max_evil;
    char            buf[MAX_STRING_LENGTH];

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (!check_soundproof(ch)) {
            if (number(0, 100) == 0) {
                command_interpreter(ch, "shout To me, my fellows! I am in "
                                        "need of thy aid!");
            } else {
                act("$n shouts 'To me, my fellows! I need thy aid!'",
                    TRUE, ch, 0, 0, TO_ROOM);
            }

            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, type);
            }
            return (TRUE);
        }
    }

    max_evil = 0;
    evil = 0;

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
#if 0

    /*
     * disabled, to many bugs in murder/stole jail and settings 
     * might try and find them some other time 
     */

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {

        if (IS_MURDER(tch) && CAN_SEE(ch, tch)) {
            if (GetMaxLevel(tch) >= 20) {
                if (!check_soundproof(ch))
                    act("$n screams 'MURDERER!!!  EVIL!!!  KILLER!!!  "
                        "BANZAI!!'", FALSE, ch, 0, 0, TO_ROOM);
                hit(ch, tch, TYPE_UNDEFINED);
                return (TRUE);
            } else {
                act("$n thawacks $N muttering 'Murderer' and has $M dragged "
                    "off to prison!", TRUE, ch, 0, tch, TO_NOTVICT);
                act("$n thawacks you muttering 'Murderer' and has you dragged "
                    "off to prison!", TRUE, ch, 0, tch, TO_VICT);
                char_from_room(tch);
                char_to_room(tch, PRISON_ROOM);
                do_look(tch, NULL, 0);
                act("The prison door slams shut behind you!",
                    TRUE, ch, 0, tch, TO_VICT);
                return (TRUE);
            }
        } else if (IS_STEALER(tch) && CAN_SEE(ch, tch)) {
            if (!number(0, 30)) {
                if (!check_soundproof(ch))
                    act("$n screams 'ROBBER!!!  EVIL!!!  THIEF!!!  BANZAI!!'",
                        FALSE, ch, 0, 0, TO_ROOM);
                hit(ch, tch, TYPE_UNDEFINED);
                return (TRUE);
            } else {
                act("$n thawacks $N muttering 'Thief' and has $M dragged off "
                    "to prison!", TRUE, ch, 0, tch, TO_NOTVICT);
                act("$n thawacks you muttering 'Thief' and has you dragged off "
                    "to prison!", TRUE, ch, 0, tch, TO_VICT);
                char_from_room(tch);
                char_to_room(tch, PRISON_ROOM);
                do_look(tch, NULL, 0);
                act("The prison door slams shut behind you!",
                    TRUE, ch, 0, tch, TO_VICT);
                return (TRUE);
            }
        }
    }

#endif

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if ((IS_NPC(tch) && (IsUndead(tch)) && CAN_SEE(ch, tch)) || 
            (IsGoodSide(ch) && IsBadSide(tch) && CAN_SEE(ch, tch)) || 
            (IsGoodSide(tch) && IsBadSide(ch) && CAN_SEE(ch, tch))) {

            max_evil = -1000;
            evil = tch;
            if (!check_soundproof(ch)) {
                act("$n screams 'EVIL!!! BANZAI!!'", FALSE, ch, 0, 0, TO_ROOM);
            }
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        }

        if (!IS_PC(tch) && tch->specials.fighting &&
            GET_ALIGNMENT(tch) < max_evil && 
            (!IS_PC(tch) || !IS_PC(tch->specials.fighting))) {
            max_evil = GET_ALIGNMENT(tch);
            evil = tch;
        }
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
        if (IS_MURDER(evil) || 
            GET_HIT(evil->specials.fighting) > GET_HIT(evil) ||
            evil->specials.fighting->attackers > 3) {
            if (!check_soundproof(ch)) {
                act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!!'",
                    FALSE, ch, 0, 0, TO_ROOM);
            }
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        } else if (!IS_MURDER(evil->specials.fighting)) {
            if (!check_soundproof(ch)) {
                act("$n yells 'There's no need to fear! $n is here!'",
                    FALSE, ch, 0, 0, TO_ROOM);
            }

            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (!ch->skills[SKILL_RESCUE].learned) {
                ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch) * 3 + 30;
            }
            sprintf( buf, "rescue %s", GET_NAME(evil->specials.fighting) );
            command_interpreter(ch, buf );
        }
    }

    GreetPeople(ch);

    return (FALSE);
}

int GenericCityguard(struct char_data *ch, int cmd, char *arg,
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

        if (!check_soundproof(ch)) {
            if (number(0, 120) == 0) {
                command_interpreter(ch, "shout To me, my fellows! I am in "
                                        "need of thy aid!");
            } else {
                act("$n shouts 'To me, my fellows! I need thy aid!'",
                    TRUE, ch, 0, 0, TO_ROOM);
            }

            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, type);
            }
            return (TRUE);
        }
    }

    max_evil = 1000;
    evil = 0;

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (tch->specials.fighting && GET_ALIGNMENT(tch) < max_evil &&
            (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
            max_evil = GET_ALIGNMENT(tch);
            evil = tch;
        }
    }

    if (evil && GET_ALIGNMENT(evil->specials.fighting) >= 0) {
        if (!check_soundproof(ch)) {
            act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }

    GreetPeople(ch);

    return (FALSE);
}

/*
 * Realms of Delbrandor (Coal) Special Procedures 
 */

/*
 * Avatar of Blibdoolpoolp - will shout every hour (approx) 
 */
int ABShout(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    static time_t   time_diff = 0;
    static time_t   last_time = 0;

    if (cmd) {
        return (FALSE);
    }
    /*
     * If it has been an hour, there is a 33% chance he will shout 
     */
    if (time_diff > 3600 && !number(0, 2)) {
        time_diff = 0;          
        /* 
         * reset 
         */
        last_time = time(NULL);
        command_interpreter(ch, "shout You puny mortals, come release me!  I "
                                "have a pest dragon whom imprisoned me to "
                                "slay, and some sea elves to boot!");
        return (TRUE);
    } else {
        /* 
         * not been an hour, update how long it has been 
         */
        time_diff = time(NULL) - last_time;
    }

    return (FALSE);
}

/*
 * Avatar of Posereisn 
 */
int AvatarPosereisn(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                   *temp,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0;
    static time_t   time_diff = 0;
    static time_t   last_time = 0;

    if (!cmd) {
        /*
         * If it has been an hour, there is a 33% chance he will shout 
         */
        if ((time_diff > 3600) && (!number(0, 2))) {
            time_diff = 0;      
            /* 
             * reset 
             */
            last_time = time(NULL);
            command_interpreter(ch, "shout Might a noble mortal bring me what "
                                    "is rightfully mine?  You shall be "
                                    "generously rewarded!");
            return (TRUE);
        } else {
            time_diff = time(NULL) - last_time;
        }
    }

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (cmd == 72) {
        /* 
         * give 
         *
         *
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name) {
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (!(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (IS_PC(vict)) {
            return (FALSE);
        }
        /*
         * the target is not the Avatar of Posereisn or is a PC 
         */
        if (mob_index[vict->nr].virtual != 28042) {
            return (FALSE);
        }
        /*
         * The object is not the Ankh of Posereisn 
         */
        if (!IS_IMMORTAL(ch)) {
            if ((obj_index[obj->item_number].virtual != 28180)) {
                sprintf(buf, "tell %s That is not the item I seek.",
                        GET_NAME(ch));
                command_interpreter(vict, buf);
                return (TRUE);
            }
        } else {
            sprintf(buf, "give %s %s", obj_name, vict_name);
            command_interpreter(ch, buf);
            if (obj_index[obj->item_number].virtual == 28180) {
                test = 1;
            }
        }

        if (!IS_IMMORTAL(ch)) {
            test = 1;
            temp = strdup("Ankh-Posereisn Avatar-Posereisn");
            do_give(ch, temp, 0);
            if( temp ) {
                free( temp );
            }
        }
    } else
        return (FALSE);

    if (test == 1) {
        /* 
         * It is the Avatar, and the Ankh 
         */
        if (vict->equipment[WIELD]) {
            /* 
             * This is only done if he is wielding hellreaper 
             */
            sprintf(buf, "tell %s Thank you mighty hero.  Take this as a "
                         "token of my appreciation.", GET_NAME(ch));
            command_interpreter(vict, buf);
            sprintf(buf, "remove %s", vict->equipment[WIELD]->name);
            command_interpreter(vict, buf);
            sprintf(buf, "give Hellreaper %s", GET_NAME(ch));
            command_interpreter(vict, buf);
            return (TRUE);
        } else {
            /* 
             * This is done if he is not wielding hellreaper 
             */
            sprintf(buf, "tell %s You are indeed a mighty hero, but I cannot "
                         "take this, for I have nothing to offer you in "
                         "return.", GET_NAME(ch));
            command_interpreter(vict, buf);
            sprintf(buf, "give Ankh-Posereisn %s", GET_NAME(ch));
            command_interpreter(vict, buf);
            return (TRUE);
        }
    }
    return( FALSE );
}

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

int PrydainGuard(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    return (GenericCityguardHateUndead(ch, cmd, arg, mob, PRYDAIN));
}

struct breath_victim {
    struct char_data *ch;
    int             yesno;      /* 1 0 */
    struct breath_victim *next;
};

struct breath_victim *choose_victims(struct char_data *ch,
                                     struct char_data *first_victim)
{
    /*
     * this is goofy, dopey extraordinaire 
     */
    struct char_data *cons;
    struct breath_victim *head = NULL,
                   *temp = NULL;

    for (cons = real_roomp(ch->in_room)->people; cons;
         cons = cons->next_in_room) {
        temp = (struct breath_victim *) malloc(sizeof(struct breath_victim));
        temp->ch = cons;
        temp->next = head;
        head = temp;
        if (first_victim == cons) {
            temp->yesno = 1;
        } else if (ch == cons) {
            temp->yesno = 0;
        } else if (in_group(first_victim, cons) ||
                   cons == first_victim->master ||
                   cons->master == first_victim) {
            /*
             * group members will get hit 2/5 times 
             */
           temp->yesno = (dice(1, 5) < 3);
        } else if (cons->specials.fighting == ch) {
            /*
             * people fighting the dragon get hit 4/5 times 
             */
            temp->yesno = (!dice(1, 5));
        } else {
            /* 
             * bystanders get his 3/5 times 
             */
            temp->yesno = (dice(1, 5) < 4);
        }
    }
    return head;
}

void free_victims(struct breath_victim *head)
{
    struct breath_victim *temp;

    while (head) {
        temp = head->next;
        if (head) {
            free(head);
        }
        head = temp;
    }
}

void breath_weapon(struct char_data *ch, struct char_data *target,
                   int mana_cost, void (*func) ())
{
    struct breath_victim *hitlist,
                   *scan;
    struct char_data *tmp;
    int             victim;

    act("$n rears back and inhales", 1, ch, 0, ch->specials.fighting, TO_ROOM);
    victim = 0;

    for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
        if (tmp != ch && !IS_IMMORTAL(tmp)) {
            victim = 1;
            cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tmp, 0);
        }
    }

    hitlist = choose_victims(ch, target);

    if (func != NULL && victim && hitlist) {
        act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

        for (scan = hitlist; scan; scan = scan->next) {
            if (scan->yesno && !IS_IMMORTAL(scan->ch) && 
                scan->ch->in_room == ch->in_room) {
                func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
            }
        }
        GET_MANA(ch) -= mana_cost;
    } else {
        act("$n Breathes...coughs and sputters...", 1, ch, 0, 
            ch->specials.fighting, TO_ROOM);
        do_flee(ch, NULL, 0);
    }

    free_victims(hitlist);
}

void use_breath_weapon(struct char_data *ch, struct char_data *target,
                       int cost, void (*func) ())
{
    if (GET_MANA(ch) >= 0) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && GET_MANA(ch) >= -cost) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && GET_MANA(ch) >= -2 * cost) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_MANA(ch) <= -3 * cost) {
        /* 
         * sputter 
         */
        breath_weapon(ch, target, 0, NULL);
    }
}

static funcp    breaths[] = {
    (funcp)cast_acid_breath, NULL, (funcp)cast_frost_breath, NULL, 
    (funcp)cast_lightning_breath, NULL, (funcp)cast_fire_breath, NULL,
    (funcp)cast_acid_breath, (funcp)cast_fire_breath, 
    (funcp)cast_lightning_breath, NULL
};

struct breather breath_monsters[] = {
    {230, 55, breaths + 0},
    {233, 55, breaths + 4},
    {243, 55, breaths + 2},
    {3670, 30, breaths + 2},
    {3674, 45, breaths + 6},
    {3675, 45, breaths + 8},
    {3676, 30, breaths + 6},
    {3952, 20, breaths + 8},
    {5005, 55, breaths + 4},
    {6112, 55, breaths + 4},
    {6635, 55, breaths + 0},
    {6609, 30, breaths + 0},
    {6642, 45, breaths + 2},
    {6801, 55, breaths + 2},
    {6802, 55, breaths + 2},
    {6824, 55, breaths + 0},
    {7040, 55, breaths + 6},
    {9217, 45, breaths + 4},
    {9418, 45, breaths + 2},
    {9419, 45, breaths + 2},
    {9727, 30, breaths + 0},
    {12010, 45, breaths + 6},
    {12011, 45, breaths + 6},
    {15858, 45, breaths + 0},
    {15879, 30, breaths + 0},
    {16620, 45, breaths + 0},
    {16700, 45, breaths + 4},
    {16738, 75, breaths + 6},
    {18003, 20, breaths + 8},
    {20002, 55, breaths + 6},
    {20017, 55, breaths + 6},
    {20016, 55, breaths + 6},
    {20016, 55, breaths + 6},
    {25009, 30, breaths + 6},
    {25504, 30, breaths + 4},
    {27016, 30, breaths + 6},
    {28022, 55, breaths + 6},
    {-1},
};

int BreathWeapon(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    char            buf[MAX_STRING_LENGTH];
    struct breather *scan;
    int             count;

    if (cmd) {
        return FALSE;
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {

        for (scan = breath_monsters;
             scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual;
             scan++) {
            /*
             * Empty loop
             */
        }

        if (scan->vnum < 0) {
            sprintf(buf, "monster %s tries to breath, but isn't listed.",
                    ch->player.short_descr);
            Log(buf);
            return FALSE;
        }

        for (count = 0; scan->breaths[count]; count++) {
            /* 
             * Empty loop 
             */
        }

        if (count < 1) {
            sprintf(buf, "monster %s has no breath weapons",
                    ch->player.short_descr);
            Log(buf);
            return FALSE;
        }

        use_breath_weapon(ch, ch->specials.fighting, scan->cost,
                          scan->breaths[dice(1, count) - 1]);
    }

    return (FALSE);
}


int Devil(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    return (magic_user(ch, cmd, arg, mob, type));
}

int Demon(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    return (magic_user(ch, cmd, arg, mob, type));

}

void DruidHeal(struct char_data *ch, int level)
{
    if (level > 13) {
        act("$n utters the words 'Woah! I feel GOOD! Heh.'.",
            1, ch, 0, 0, TO_ROOM);
        cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    } else if (level > 8) {
        act("$n utters the words 'I feel much better now!'.",
            1, ch, 0, 0, TO_ROOM);
        cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    } else {
        act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
        cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    }
}

void DruidTree(struct char_data *ch)
{
    act("$n utters the words 'harumph!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n takes on the form and shape of a huge tree!", FALSE, ch, 0, 0,
        TO_ROOM);
    GET_RACE(ch) = RACE_TREE;
    ch->points.max_hit = GetMaxLevel(ch) * 10;
    ch->points.hit += GetMaxLevel(ch) * 5;

    if (ch->player.long_descr) {
        free(ch->player.long_descr);
    }
    if (ch->player.short_descr) {
        free(ch->player.short_descr);
    }
    ch->player.short_descr = (char *) strdup("The druid-tree");
    ch->player.long_descr = (char *) strdup("A huge tree stands here");
    GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
    ch->mult_att = 6;
    ch->specials.damsizedice = 6;
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
}

void DruidMob(struct char_data *ch)
{
    act("$n utters the words 'lagomorph'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n takes on the form and shape of a huge lion", FALSE, ch, 0, 0,
        TO_ROOM);
    GET_RACE(ch) = RACE_PREDATOR;
    ch->points.max_hit *= 2;
    ch->points.hit += GET_HIT(ch) / 2;

    if (ch->player.long_descr) {
        free(ch->player.long_descr);
    }
    if (ch->player.short_descr) {
        free(ch->player.short_descr);
    }
    ch->player.short_descr = strdup("The druid-lion");
    ch->player.long_descr = strdup("A huge lion stands here, his tail twitches"
                                   " menacingly");
    GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
    ch->mult_att = 3;
    ch->specials.damnodice = 3;
    ch->specials.damsizedice = 4;
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
}

int DruidChallenger(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    struct room_data *rp;
    int             level;
    struct char_data *vict;
    int             i;
    char           *name;

    if ((cmd) || (!AWAKE(ch))) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);
    if ((!rp) || (check_soundproof(ch))) {
        return (FALSE);
    }
    GreetPeople(ch);

    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_SLEEPING) {
        StandUp(ch);
        return (TRUE);
    }

    if (number(0, 101) > GetMaxLevel(ch) + 40) {
        return (TRUE);
    }
    if (!ch->specials.fighting) {
        level = number(1, GetMaxLevel(ch));
        if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
            DruidHeal(ch, level);
            return (TRUE);
        }

        if (!ch->equipment[WIELD]) {
            if (GetMaxLevel(ch) > 4) {
                act("$n utters the words 'gimme a light'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_flame_blade(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            }
            return (TRUE);
        }

        if (!affected_by_spell(ch, SPELL_BARKSKIN) && number(0, 1)) {
            act("$n utters the words 'woof woof'", 1, ch, 0, 0, TO_ROOM);
            cast_barkskin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (ch->in_room == 501 && (vict = FindAHatee(ch)) == NULL) {
            vict = FindVictim(ch);
            if (vict) {
                name = strdup(GET_NAME(vict));
                do_hit(ch, name, 0);
                if( name ) {
                    free( name );
                }
            }
            return (FALSE);
        }
    } else {
        if ((vict = FindAHatee(ch)) == NULL) {
            vict = FindVictim(ch);
        }
        if (!vict) {
            vict = ch->specials.fighting;
        }
        if (!vict) {
            return (FALSE);
        }
        level = number(1, GetMaxLevel(ch));

        if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 2) && number(0, 1)) {
            DruidHeal(ch, level);
            return (TRUE);
        }

        if (GetMaxLevel(ch) >= 20 && !IS_AFFECTED(vict, AFF_SILENCE) &&
            !number(0, 4)) {
            act("$n utters the words 'ssshhhh'", 1, ch, 0, 0, TO_ROOM);
            cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (FALSE);
        }

        if ((IS_AFFECTED(vict, AFF_FIRESHIELD) || 
            IS_AFFECTED(vict, AFF_SANCTUARY)) && 
            GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'use instaway instant magic remover'", 1,
                ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, vict, 0);
            return (TRUE);
        }

        if (IsUndead(vict) && !number(0, 2)) {
            act("$n utters the words 'see the light!'", 1, ch, 0, 0, TO_ROOM);
            cast_sunray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (FALSE);
        }

        if (vict->equipment[WIELD] && level > 19) {
            act("$n utters the words 'frogemoth'", 1, ch, 0, 0, TO_ROOM);
            cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            return (FALSE);
        }

        if (level > 23) {
            for (i = 0; i < MAX_WEAR; i++) {
                if (vict->equipment[i]) {
                    act("$n utters the words 'barbecue?'", 1, ch, 0, 0,
                        TO_ROOM);
                    cast_warp_weapon(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }
            }
        }

        if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, TUNNEL) ||
            IS_SET(rp->room_flags, PRIVATE)) {
            DruidAttackSpells(ch, vict, level);
        } else {
            if (rp->sector_type == SECT_FOREST) {
                if (level > 16 && !number(0, 5)) {
                    act("$n utters the words 'briar'", 1, ch, 0, 0, TO_ROOM);
                    cast_entangle(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }

                if (level >= 8 && !number(0, 3)) {
                    act("$n utters the words 'snap!'", 1, ch, 0, 0, TO_ROOM);
                    cast_snare(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
                               0);
                    return (FALSE);
                }

                if (level > 30 && ch->mult_att < 6 && !number(0, 8)) {
                    DruidTree(ch);
                    return (FALSE);
                }

                if (ch->mult_att < 2 && level > 10 && !number(0, 8)) {
                    DruidMob(ch);
                    return (FALSE);
                }

                if (level > 30 && !number(0, 4)) {
                    act("$n utters the words 'Where is my SERVANT!'",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_fire_servant(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, 0, 0);
                    command_interpreter(ch, "order followers guard on");
                    return (FALSE);
                } 
                
                if (level > 10 && !number(0, 5)) {
                    act("$n whistles", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_1(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 16 && !number(0, 5)) {
                    act("$n whistles loudly", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_2(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 24 && !number(0, 5)) {
                    act("$n whistles extremely loudly", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_animal_summon_3(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }
            } else if (!IS_SET(rp->room_flags, INDOORS)) {
                if (level > 8 && !number(0, 3)) {
                    act("$n utters the words 'let it rain'", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_control_weather(GetMaxLevel(ch), ch, "worse",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 15 && !number(0, 2)) {
                    act("$n utters the words 'here lightning'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_call_lightning(GetMaxLevel(ch), ch, "",
                                        SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }
            }
            DruidAttackSpells(ch, vict, level);
        }
    }
    return(FALSE);
}

int MonkChallenger(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict;
    char                *name;

    if ((cmd) || (!AWAKE(ch))) {
        return (FALSE);
    }
    GreetPeople(ch);

    if (!ch->specials.fighting && ch->in_room == 551 &&
        (vict = FindAHatee(ch)) == NULL) {
        vict = FindVictim(ch);
        if (vict) {
            name = strdup(GET_NAME(vict));
            do_hit(ch, name, 0);
            if( name ) {
                free( name );
            }
        }
        return (FALSE);
    }
    MonkMove(ch);
    return (1);
}



int druid_challenge_prep_room(struct char_data *ch, int cmd, char *arg,
                              struct room_data *rp, int type)
{
    struct room_data *me,
                   *chal;
    int             i,
                    newr;
    struct obj_data *o,
                   *tmp_obj,
                   *next_obj;
    struct char_data *mob;

    me = real_roomp(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    chal = real_roomp(DRUID_CHALLENGE_ROOM);
    if (!chal) {
        send_to_char("The challenge room is gone.. please contact a god\n\r",
                     ch);
        return (TRUE);
    }

    if (cmd == NOD) {
        if (!HasClass(ch, CLASS_DRUID)) {
            send_to_char("You're no druid.\n\r", ch);
            return (FALSE);
        }

        if (GET_LEVEL(ch, DRUID_LEVEL_IND) < 10) {
            send_to_char("You have no business here, kid.\n\r", ch);
            return (FALSE);
        }

        if (GET_EXP(ch) <= 
            classes[DRUID_LEVEL_IND].
                levels[GET_LEVEL(ch, DRUID_LEVEL_IND) + 1].exp - 100) {
            send_to_char("You cannot advance now.\n\r", ch);
            return (TRUE);
        } else if (GET_LEVEL(ch, DRUID_LEVEL_IND) == 50) {
            send_to_char("You are far too powerful to be trained here... Seek "
                         "an implementor to help you.", ch);
            return (FALSE);
        }

        if (chal->river_speed != 0) {
            send_to_char("The challenge room is busy.. please wait.\n\r", ch);
            return (TRUE);
        }

        for (i = 0; i < MAX_WEAR; i++) {
            if (ch->equipment[i]) {
                o = unequip_char(ch, i);
                obj_to_char(o, ch);
            }
        }

        for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = next_obj) {
            next_obj = tmp_obj->next_content;
            obj_from_char(tmp_obj);
            obj_to_room(tmp_obj, ch->in_room);
        }

        druidpreproom = ch->in_room;

        send_to_char("You are taken into the combat room.\n\r", ch);
        act("$n is ushered into the combat room.", FALSE, ch, 0, 0, TO_ROOM);
        newr = DRUID_CHALLENGE_ROOM;
        char_from_room(ch);
        char_to_room(ch, newr);

        /*
         * load the mob at the same lev as char 
         */
        mob = read_mobile(DRUID_MOB + GET_LEVEL(ch, DRUID_LEVEL_IND) - 10,
                          VIRTUAL);

        if (!mob) {
            send_to_char("The fight is called off. Go home.\n\r", ch);
            return (TRUE);
        }
        char_to_room(mob, ch->in_room);
        chal->river_speed = 1;
        do_look(ch, NULL, 0);
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        return (TRUE);
    }

    return (FALSE);
}

int druid_challenge_room(struct char_data *ch, int cmd, char *arg,
                         struct room_data *rp, int type)
{
    struct char_data *i;
    struct room_data *me;
    int             rm;

    me = real_roomp(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    rm = ch->in_room;

    if (!me->river_speed) {
        return (FALSE);
    }
    if (IS_PC(ch)) {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    }

    if (cmd == FLEE) {
        /*
         * this person just lost 
         */
        send_to_char("You lose\n\r", ch);
        if (IS_PC(ch)) {
            if (IS_NPC(ch)) {
                command_interpreter(ch, "return");
            }
            GET_EXP(ch) = 
                MIN(classes[DRUID_LEVEL_IND].
                        levels[(int)GET_LEVEL(ch, DRUID_LEVEL_IND)].exp,
                    GET_EXP(ch));
            send_to_char("Go home\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, rm - 1);
            me->river_speed = 0;
            while (me->people) {
                extract_char(me->people);
            }
            return (TRUE);
        } else {
            if (mob_index[ch->nr].virtual >= DRUID_MOB &&
                mob_index[ch->nr].virtual <= DRUID_MOB + 40) {
                extract_char(ch);
                /*
                 * find pc in room; 
                 */
                for (i = me->people; i; i = i->next_in_room)
                    if (IS_PC(i)) {
                        if (IS_NPC(i)) {
                            command_interpreter(i, "return");
                        }
                        GET_EXP(i) = 
                            MAX(classes[DRUID_LEVEL_IND].
                                  levels[GET_LEVEL(i, DRUID_LEVEL_IND) + 
                                         1].exp + 1, GET_EXP(i));
                        GainLevel(i, DRUID_LEVEL_IND);
                        char_from_room(i);
                        char_to_room(i, druidpreproom);

                        if (affected_by_spell(i, SPELL_POISON)) {
                            affect_from_char(ch, SPELL_POISON);
                        }

                        if (affected_by_spell(i, SPELL_HEAT_STUFF)) {
                            affect_from_char(ch, SPELL_HEAT_STUFF);
                        }

                        while (me->people) {
                            extract_char(me->people);
                        }

                        while (me->contents) {
                            extract_obj(me->contents);
                        }

                        me->river_speed = 0;

                        return (TRUE);
                    }
                return (TRUE);
            } else {
                return (FALSE);
            }
        }
    }
    return (FALSE);

}

int monk_challenge_room(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    struct char_data *i;
    struct room_data *me;
    int             rm;

    rm = ch->in_room;

    me = real_roomp(ch->in_room);
    if ((!me) || (!me->river_speed)) {
        return (FALSE);
    }
    if (IS_PC(ch)) {
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
    }

    if (cmd == FLEE) {
        /*
         * this person just lost 
         */
        send_to_char("You lose.\n\r", ch);
        if (IS_PC(ch)) {
            if (IS_NPC(ch)) {
                command_interpreter(ch, "return");
            }
            GET_EXP(ch) = 
                MIN(classes[MONK_LEVEL_IND].
                       levels[(int)GET_LEVEL(ch, MONK_LEVEL_IND)].exp,
                    GET_EXP(ch));
            send_to_char("Go home.\n\r", ch);
            char_from_room(ch);
            char_to_room(ch, rm - 1);
            me->river_speed = 0;
            return (TRUE);
        } else if (mob_index[ch->nr].virtual >= MONK_MOB &&
                   mob_index[ch->nr].virtual <= MONK_MOB + 40) {
            extract_char(ch);
            /*
             * find pc in room; 
             */
            for (i = me->people; i; i = i->next_in_room) {
                if (IS_PC(i)) {
                    if (IS_NPC(i)) {
                        command_interpreter(i, "return");
                    }
                    GET_EXP(i) = 
                        MAX(classes[MONK_LEVEL_IND].
                               levels[GET_LEVEL(i, MONK_LEVEL_IND) + 
                                      1].exp + 1, GET_EXP(i));
                    GainLevel(i, MONK_LEVEL_IND);
                    char_from_room(i);
                    char_to_room(i, monkpreproom);

                    while (me->people) {
                        extract_char(me->people);
                    }
                    while (me->contents) {
                        extract_obj(me->contents);
                    }
                    me->river_speed = 0;
                    return (TRUE);
                }
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

int monk_challenge_prep_room(struct char_data *ch, int cmd, char *arg,
                             struct room_data *rp, int type)
{
    struct room_data *me,
                   *chal;
    int             i,
                    newr;
    struct obj_data *o,
                   *tmp_obj,
                   *next_obj;
    struct char_data *mob;

    me = real_roomp(ch->in_room);
    if (!me) {
        return (FALSE);
    }
    chal = real_roomp(MONK_CHALLENGE_ROOM);
    if (!chal) {
        send_to_char("The challenge room is gone.. please contact a god\n\r",
                     ch);
        return (TRUE);
    }

    if (cmd == NOD) {
        if (!HasClass(ch, CLASS_MONK)) {
            send_to_char("You're no monk\n\r", ch);
            return (FALSE);
        }

        if (GET_LEVEL(ch, MONK_LEVEL_IND) < 10) {
            send_to_char("You have no business here, kid.\n\r", ch);
            return (FALSE);
        }

        if (GET_EXP(ch) <= 
            classes[MONK_LEVEL_IND].
                levels[GET_LEVEL(ch, MONK_LEVEL_IND) + 1].exp - 100) {
            send_to_char("You cannot advance now\n\r", ch);
            return (TRUE);
        }

        if (chal->river_speed != 0) {
            send_to_char("The challenge room is busy.. please wait\n\r", ch);
            return (TRUE);
        }

        for (i = 0; i < MAX_WEAR; i++) {
            if (ch->equipment[i]) {
                o = unequip_char(ch, i);
                obj_to_char(o, ch);
            }
        }

        for (tmp_obj = ch->carrying; tmp_obj; tmp_obj = next_obj) {
            next_obj = tmp_obj->next_content;
            obj_from_char(tmp_obj);
            obj_to_room(tmp_obj, ch->in_room);
        }

        monkpreproom = ch->in_room;

        send_to_char("You are taken into the combat room.\n\r", ch);
        act("$n is ushered into the combat room", FALSE, ch, 0, 0, TO_ROOM);
        newr = MONK_CHALLENGE_ROOM;
        char_from_room(ch);
        char_to_room(ch, newr);

        /*
         * load the mob at the same lev as char 
         */
        mob = read_mobile(MONK_MOB + GET_LEVEL(ch, MONK_LEVEL_IND) - 10,
                          VIRTUAL);

        if (!mob) {
            send_to_char("The fight is called off.. Go home.\n\r", ch);
            return (TRUE);
        }
        char_to_room(mob, ch->in_room);
        chal->river_speed = 1;
        do_look(ch, NULL, 0);
        REMOVE_BIT(ch->specials.act, PLR_WIMPY);
        return (TRUE);
    }

    return (FALSE);
}


/************************************************************************/
#if 0
/*
 * glass teleport ring 
 */
int glass_teleport_ring(struct char_data *ch, int cmd, char *arg,
                        struct obj_data *obj, int type)
{

}

#endif


int portal(struct char_data *ch, int cmd, char *arg, struct obj_data *obj,
           int type)
{
    struct obj_data *port;
    char           *obj_name;

    if (type == PULSE_COMMAND) {
        if (cmd != ENTER) {
            return (FALSE);
        }
        arg = get_argument(arg, &obj_name);
        if (!obj_name ||
            !(port = get_obj_in_list_vis(ch, obj_name,
                                         real_roomp(ch->in_room)->contents))) {
            return (FALSE);
        }

        if (port != obj) {
            return (FALSE);
        }
        if (port->obj_flags.value[1] <= 0 || port->obj_flags.value[1] > 80000) {
            /* 
             * see hash.h Mythos 
             */
            send_to_char("The portal leads nowhere\n\r", ch);
            return( FALSE );
        }

        act("$n enters $p, and vanishes!", FALSE, ch, port, 0, TO_ROOM);
        act("You enter $p, and you are transported elsewhere", FALSE, ch,
            port, 0, TO_CHAR);
        char_from_room(ch);
        char_to_room(ch, port->obj_flags.value[1]);
        do_look(ch, NULL, 0);
        act("$n appears from thin air!", FALSE, ch, 0, 0, TO_ROOM);
    } else {
        obj->obj_flags.value[0]--;
        if (obj->obj_flags.value[0] == 0) {
            if (obj->in_room != NOWHERE && real_roomp(obj->in_room)->people) {
                act("$p vanishes in a cloud of smoke!", FALSE,
                    real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
                act("$p vanishes in a cloud of smoke!", FALSE,
                    real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
            }
            extract_obj(obj);
        }
    }
    return( TRUE );
}

int scraps(struct char_data *ch, int cmd, char *arg, struct obj_data *obj,
           int type)
{
    if (type == PULSE_COMMAND) {
        return (FALSE);
    } else {
        if (obj->obj_flags.value[0]) {
            obj->obj_flags.value[0]--;
        }
        if (obj->obj_flags.value[0] == 0 && obj->in_room) {
            if (obj->in_room != NOWHERE && real_roomp(obj->in_room)->people) {
                act("$p disintegrates into atomic particles!", FALSE,
                    real_roomp(obj->in_room)->people, obj, 0, TO_ROOM);
                act("$p disintegrates into atomic particles!", FALSE,
                    real_roomp(obj->in_room)->people, obj, 0, TO_CHAR);
            }
            extract_obj(obj);
        }
    }
    return( TRUE );
}

int attack_rats(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             dir;

    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == EVENT_WINTER) {
        ch->generic = ATTACK_ROOM + number(0, 26);
        /* 
         * so they'll attack beggars, etc 
         */
        AddHatred(ch, OP_RACE, RACE_HUMAN);
    }

    if (type == EVENT_SPRING) {
        ch->generic = 0;
    }
    if (ch->generic == 0) {
        return (FALSE);
    }
    dir = choose_exit_global(ch->in_room, ch->generic, MAX_ROOMS);
    if (dir == -1) {
        /* 
         * assume we found it.. start wandering 
         */
        ch->generic = 0;
        /* 
         * We Can't Go Anywhere. 
         */
        return (FALSE);
    }

    go_direction(ch, dir);
    return( TRUE );
}


int DragonHunterLeader(struct char_data *ch, int cmd, char *arg,
                       struct char_data *mob, int type)
{
    register struct char_data *i,
                   *j;
    int             found = FALSE,
                    dir,
                    count;
    char            buf[255];

#if 0
    char           *name;

    if(type == PULSE_COMMAND) { 
        if(cmd == 19) {
            if(!strncasecmp(arg,"biff", 4)) { 
                do_follow(ch, arg, cmd); 
                name = strdup(GET_NAME(ch));
                do_group(mob, name, 0); 
                if( name ) {
                    free( name );
                }
                return(TRUE); 
            }
        }
        return(FALSE); 
    }
#endif

    if (type == PULSE_TICK) {
        if (ch->specials.position == POSITION_SITTING) {
            ch->generic = 0;
            switch (number(1, 10)) {
            case 1:
                command_interpreter(ch, "emote mumbles something about in his "
                                        "day the tavern being a popular "
                                        "hangout.");
                break;
            case 2:
                command_interpreter(ch, "say I really miss the good old days "
                                        "of fighting dragons all day.");
                command_interpreter(ch, "say I really should do it more often "
                                        "to keep in shape.");
                break;
            default:
                break;
            }
            return (TRUE);
        }

        if (ch->specials.position == POSITION_STANDING) {
            if (ch->generic <= 20) {
                ch->generic++;
                return (FALSE);
            } else if (ch->generic == 21) {
                for (i = character_list; i; i = i->next) {
                    if (IS_MOB(i) && mob_index[i->nr].virtual == WHO_TO_HUNT) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    ch->generic = 25;
                    command_interpreter(ch, "say Ack! The dragon is dead! I'm "
                                            "going back to the bar!");
                } else {
                    command_interpreter(ch, "say Ok, Follow me and let's go "
                                            "kill ourselves a dragon!");
                    ch->generic = 23;
                    count = 1;
                    for (i = real_roomp(ch->in_room)->people; i;
                         i = i->next_in_room) {
                        if (IS_MOB(i) && 
                            mob_index[i->nr].virtual == WHO_TO_CALL) {
                            (*mob_index[i->nr].func) (i, 0, "", ch,
                                                      EVENT_FOLLOW);
                            sprintf(buf, "group %d.%s", count, GET_NAME(i));
                            command_interpreter(ch, buf);
                            count++;
                        } else if (i->master && i->master == ch && 
                                   GetMaxLevel(i) > 10) {
                            sprintf(buf, "group %s", GET_NAME(i));
                            command_interpreter(ch, buf);
                        } else if (i->master && i->master == ch) {
                            sprintf(buf, "tell %s You're too little! Get Lost!",
                                    GET_NAME(i));
                            command_interpreter(ch, buf);
                        }
                    }
                }

                if (!IS_AFFECTED(ch, AFF_GROUP)) {
                    SET_BIT(ch->specials.affected_by, AFF_GROUP);
                }
                spell_fly_group(40, ch, 0, 0);
                return (FALSE);
            } else if (ch->generic == 23) {
                for (i = character_list; i; i = i->next) {
                    if (IS_MOB(i) && mob_index[i->nr].virtual == WHO_TO_HUNT) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    ch->generic = 25;
                    command_interpreter(ch, "say Ack! The dragon is dead! I'm "
                                            "going back to the bar!");
                } else {
                    dir = choose_exit_global(ch->in_room, i->in_room,
                                             MAX_ROOMS);
                    if (dir == -1) {
                        /* 
                         * can't go anywhere, wait... 
                         */
                        return (FALSE);
                    }
                    go_direction(ch, dir);

                    if (ch->in_room == i->in_room) {
                        /* 
                         * we're here! 
                         */
                        command_interpreter(ch, "shout The dragon must die!");

                        for (j = real_roomp(ch->in_room)->people; j;
                             j = j->next_in_room) {
                            if (IS_MOB(j) && 
                                mob_index[j->nr].virtual == WHO_TO_CALL) {
                                (*mob_index[j->nr].func) (j, 0, "", i,
                                                          EVENT_ATTACK);
                            }
                        }

                        ch->generic = 24;
                        hit(ch, i, TYPE_UNDEFINED);
                    }
                    return (FALSE);
                }
            } else if (ch->generic == 24) {
                command_interpreter(ch, "say Guess it's back to the bar for me!"
                                        " I need a drink!");
                ch->generic = 25;
            } else if (ch->generic == 25) {
                dir = choose_exit_global(ch->in_room, WHERE_TO_SIT, MAX_ROOMS);
                if (dir == -1) {
                    /* 
                     * no place to go, wait 
                     */
                    return (FALSE);
                }
                go_direction(ch, dir);
                if (ch->in_room == WHERE_TO_SIT) {
                    command_interpreter(ch, "say Ahhh, time for a drink!");
                    for (i = real_roomp(ch->in_room)->people; i;
                         i = i->next_in_room) {
                        if (IS_MOB(i) && 
                            mob_index[i->nr].virtual == WHO_TO_CALL) {
                            (*mob_index[i->nr].func) (i, 0, "", i,
                                                      EVENT_FOLLOW);
                        }
                    }

                    do_sit(ch, NULL, 0);
                    command_interpreter(ch, "say Bartender, how about a "
                                            "drink?");
                    ch->generic = 0;
                }
            }
        }
    }

    if (type == EVENT_WEEK) {
        /* 
         * months are TOO long 
         */
        if (ch->specials.position != POSITION_SITTING) {
            /* 
             * We're doing something else, ignore 
             */
            return (FALSE);
        }

        for (i = character_list; i; i = i->next) {
            if (IS_MOB(i) && mob_index[i->nr].virtual == WHO_TO_HUNT) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            /* 
             * No Dragon in the game, ignore 
             */
            return (FALSE);
        }

        for (i = character_list; i; i = i->next) {
            if (IS_MOB(i) && (mob_index[i->nr].virtual == WHO_TO_CALL)) {
                (*mob_index[i->nr].func) (i, 0, "", ch, EVENT_GATHER);
            }
        }

        command_interpreter(ch, "shout All who want to hunt a dragon, come to "
                                "me!");
        do_stand(ch, NULL, 0);
    }

    return (FALSE);
}

int HuntingMercenary(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    int             dir;

    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == PULSE_TICK) {
        if (ch->generic == 1) {
            /* 
             * Going to room 
             */
            if (!IS_SET(ch->specials.act, ACT_SENTINEL)) {
                SET_BIT(ch->specials.act, ACT_SENTINEL);
            }
            dir = choose_exit_global(ch->in_room, WHERE_TO_SIT, MAX_ROOMS);
            if (dir == -1) {
                return (FALSE);
            }
            go_direction(ch, dir);

            if (ch->in_room == WHERE_TO_SIT) {
                ch->generic = 0;
            }
        }
        return (FALSE);
    }

    if (type == EVENT_GATHER) {
        ch->generic = 1;
        return (FALSE);
    }

    if (type == EVENT_ATTACK) {
        hit(ch, mob, TYPE_UNDEFINED);
        return (FALSE);
    }

    if (type == EVENT_FOLLOW) {
        if (ch == mob) {
            if (IS_SET(ch->specials.act, ACT_SENTINEL)) {
                REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
            }
            ch->generic = 0;
            stop_follower(ch);
        } else {
            add_follower(ch, mob);
        }
    }

    return (FALSE);
}

long            jackpot = 25;

int SlotMachine(struct char_data *ch, int cmd, char *arg,
                struct obj_data *obj, int type)
{
    int             c,
                    i[3],
                    ind = 0;
    char            buf[255];

    if (cmd != 224) {
        return (FALSE);
    }
    if (GET_GOLD(ch) < 25) {
        send_to_char("You don't have enough gold!\n\r", ch);
        return (TRUE);
    }

    if (jackpot == 0) {
        /* 
         * always at LEAST have 25 in there 
         */
        jackpot = 25; 
    }

    GET_GOLD(ch) -= 25;
    jackpot += 25;

    for (c = 0; c <= 2; c++) {
        i[c] = number(1, 28);
        switch (i[c]) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            i[c] = 0;
            sprintf(buf, "Slot %d: Lemon\n\r", c);
            send_to_char(buf, ch);
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
            i[c] = 1;
            sprintf(buf, "Slot %d: Orange\n\r", c);
            send_to_char(buf, ch);
            break;
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
            i[c] = 2;
            sprintf(buf, "Slot %d: Banana\n\r", c);
            send_to_char(buf, ch);
            break;
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
            i[c] = 3;
            sprintf(buf, "Slot %d: Peach\n\r", c);
            send_to_char(buf, ch);
            break;
        case 26:
        case 27:
            i[c] = 4;
            sprintf(buf, "Slot %d: Bar\n\r", c);
            send_to_char(buf, ch);
            break;
        case 28:
            i[c] = 5;
            sprintf(buf, "Slot %d: Gold\n\r", c);
            send_to_char(buf, ch);
            break;
        }

    }

    if (i[0] == i[1] && i[1] == i[2]) {
        /* 
         * Ok, they've won, now how much? 
         */
        send_to_char("You've won!\n\r", ch);

        switch (i[0]) {
        case 0:
            /* 
             * Give them back what they put in 
             */
            ind = 25;
            break;
        case 1:
            ind = 100;
            break;
        case 2:
            ind = 200;
            break;
        case 3:
            ind = 500;
            break;
        case 4:
            ind = 1000;
            break;
        case 5:
            /* 
             * Wow! We've won big! 
             */
            ind = jackpot; 
            act("Sirens start sounding and lights start flashing everywhere!", 
                FALSE, ch, 0, 0, TO_ROOM);
            break;
        }

        if (ind > jackpot) {
            /* 
             * Can only win as much as there is 
             */
            ind = jackpot;
        }

        sprintf(buf, "You have won %d coins!\n\r", ind);
        send_to_char(buf, ch);

        GET_GOLD(ch) += ind;
        jackpot -= ind;
        return (TRUE);
    }

    send_to_char("Sorry, you didn't win.\n\r", ch);
    return (TRUE);
}


int astral_destination[] = {
    41925,     /* mob 2715 */
    21108,     /* mob 2716 */
    1474,      /* ... */
    1633,
    4109,
    5000,
    5126,
    5221,
    6513,
    7069,
    6601,
    9359,
    13809,
    16925,
    20031,
    27431,
    21210,
    25041,    /* mob 2732 */
    26109,    /* mob 2733 */
    18233,
    15841,
    13423,
    44980,    /* mob 2737 */
    31908,    /* Ash's winterfell */
    49302    /* Tarantis Bazaar */
};
int astral_dest_count = NELEMS(astral_destination);

int astral_portal(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    char           *arg1;
    char            buf[255];
    int             i;
    int             j;
    struct char_data *portal;

    /*
     * To add another color pool, create another mobile (2733, etc) and
     * add another destination.  
     */

    if (cmd != 7) {
        /* 
         * enter 
         */
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1 && (is_abbrev(arg1, "pool") || is_abbrev(arg1, "color pool")) && 
        (portal = get_char_room("color pool", ch->in_room))) {
        i = mob_index[portal->nr].virtual - AST_MOB_NUM;
        if( i < 0 || i >= astral_dest_count ) {
            sprintf( buf, "Astral destination bugger-up: index %d", i );
            Log(buf);
            return(FALSE);
        }

        j = astral_destination[i];
        if (j > 0 && j < 50000) {
            send_to_char("\n\r", ch);
            send_to_char("You attempt to enter the pool, and it gives.\n\r",
                         ch);
            send_to_char("You press on further and the pool surrounds you, "
                         "like some soft membrane.\n\r", ch);
            send_to_char("There is a slight wrenching sensation, and then the "
                         "color disappears.\n\r", ch);
            send_to_char("\n\r", ch);
            act("$n enters a color and disappears!", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, j);

            act("$n appears in a dazzling explosion of light!",
                FALSE, ch, 0, 0, TO_ROOM);
            command_interpreter(ch, "look");
            return (TRUE);
        }
    }
    return (FALSE);
}

const int       post_list[] = {
    32001,
    32004,
    32009,
    32011,
    32030,
    32024,
    32032,
    33180,                      /* split */
    31804,
    32600,
    32300,
    32801,
    32803,
    32802
};
int post_count = NELEMS(post_list);

const int       post_destination[] = {
    31804,
    33180,
    32300,
    32600,
    32838,
    3014,
    25002,
    32004,
    32000,
    32011,
    32009,
    32030,
    32024,
    32032
};

int Etheral_post(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    int             check = -1,
                    x = 0;
    char           *arg1;
    int             j;
    struct char_data *post;

    if (cmd != 463) {
        /* 
         * touch 
         */
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1) {
        if (!strcasecmp("post", arg1) || !strcasecmp("ethereal", arg1) ||
            !strcasecmp("ethereal post", arg1)) {
            if ((post = get_char_room("ethereal post", ch->in_room))) {
                /*
                 * Check to see where the post is going 
                 */
                check = -1;

                for (x = 0; x < post_count; x++) {
                    if (mob_index[post->nr].virtual == post_list[x]) {
                        check = x;
                    }
                }

                if (check == -1) {
                    return (FALSE);
                }
                if (GetMaxLevel(ch) < 41) {
                    /* 
                     * level fix 
                     */
                    send_to_char("You touch the post, and a brief sensation "
                                 "of unworthiness runs through you.\n\r", ch);
                    return (TRUE);
                }

                j = post_destination[check];

                send_to_char("You touch the strange post and suddenly feel "
                             "your mind", ch);
                send_to_char(" and body being torn appart.\n\r", ch);

                act("$n touches the strange post and suddenly disappears!",
                    FALSE, ch, 0, 0, TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, j);

                act("A strange rift like portal appears and $n steps out!",
                    FALSE, ch, 0, 0, TO_ROOM);
                do_look(ch, NULL, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

int board_ship(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    int             j;
    char           *arg1;
    struct char_data *ship;

    if (cmd != 620) {
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1) {
        if ((!strcasecmp("ship", arg1) || !strcasecmp("corsair", arg1) || 
             !strcasecmp("corsair ship", arg1)) &&  
            (ship = get_char_room("corsair ship", ch->in_room))) {
            j = 32033;

            send_to_char("You enter the ship.\n\r", ch);

            act("$n enters the ship.", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, j);

            act("Walks onto the ship.", FALSE, ch, 0, 0, TO_ROOM);
            do_look(ch, NULL, 0);
            return (TRUE);
        }
    }
    return (FALSE);
}


extern int      gevent;
extern float    shop_multiplier;

int DwarvenMiners(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == EVENT_END_STRIKE && ch->specials.position == POSITION_SITTING) {
        command_interpreter(ch, "emote is off strike.");
        do_stand(ch, NULL, 0);
        ch->specials.default_pos = POSITION_STANDING;
        ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                 sizeof(char) * 54);
        strcpy(ch->player.long_descr,
               "A dwarven mine-worker is here, working the mines.\n\r");
        if (gevent != 0) {
            gevent = 0;
        }
        shop_multiplier = 0;
    }

    if (type == EVENT_DWARVES_STRIKE) {
        if (ch->specials.position == POSITION_STANDING) {
            command_interpreter(ch, "emote is on strike.");
            do_sit(ch, NULL, 0);
            ch->specials.default_pos = POSITION_SITTING;
            ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                     sizeof(char) * 55);
            strcpy(ch->player.long_descr,
                   "A dwarven mine-worker is sitting here on-strike\n\r");
        }
        ch->generic = 30;
        return (FALSE);
    }

    if (type == PULSE_TICK) {
        if (gevent != DWARVES_STRIKE) {
            ch->generic = 0;
            return (FALSE);
        }

        ch->generic++;
        if (ch->generic == 30) {
            /* 
             * strike over, back to work 
             */
            PulseMobiles(EVENT_END_STRIKE);
            if (ch->specials.position == POSITION_SITTING) {
                command_interpreter(ch, "emote is off strike.");
                do_stand(ch, NULL, 0);
                ch->specials.default_pos = POSITION_STANDING;
                ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                         sizeof(char) * 65);
                strcpy(ch->player.long_descr,
                       "A dwarven mine-worker is here, working the mines.\n\r");
            }
            ch->generic = 0;
            gevent = 0;
            shop_multiplier = 0;
        }

        return (FALSE);
    }

    if ((type == EVENT_BIRTH) && (gevent != DWARVES_STRIKE)) {
        return (FALSE);
    }
    if (type == EVENT_BIRTH) {
        if (ch->specials.position == POSITION_STANDING) {
            command_interpreter(ch, "emote is on strike.");
            do_sit(ch, NULL, 0);
            ch->specials.default_pos = POSITION_SITTING;
            ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                     sizeof(char) * 55);
            strcpy(ch->player.long_descr,
                   "A dwarven mine-worker is sitting here on-strike\n\r");
        }
        ch->generic = 30;
        return (FALSE);
    }

    if (type == EVENT_WEEK) {
        if (gevent != 0) {
            /* 
             * something else happening? FORGET IT! 
             */
            return (FALSE);
        }

        if (!number(1, 6)) {
            /* 
             * 1 in 6 chance of striking this week 
             */
            return (FALSE);
        }

        PulseMobiles(EVENT_DWARVES_STRIKE);
        gevent = DWARVES_STRIKE;
        switch (number(1, 5)) {
        case 1:
            shop_multiplier = 0.25;
            break;
        case 2:
            shop_multiplier = 0.5;
            break;
        case 3:
            shop_multiplier = 0.75;
            break;
        case 4:
            shop_multiplier = 1.0;
            break;
        case 5:
            shop_multiplier = 1.5;
            break;
        }
    }
    return (FALSE);
}

/*
 * From the appendages of Gecko... (now you know who to blame =) 
 */

int real_rabbit(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *i;

    if (cmd || !AWAKE(ch) || ch->specials.fighting) {
        return FALSE;
    }
    for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (IS_NPC(i) && mob_index[i->nr].virtual == 6005 && !number(0, 3)) {
            command_interpreter(ch, "emote sees the damn fox and runs like "
                                    "hell.");
            do_flee(ch, NULL, 0);
            return TRUE;
        }
    }

    if (!number(0, 5)) {
        switch (number(1, 2)) {
        case 1:
            command_interpreter(ch, "emote nibbles on some grass.");
            break;
        case 2:
            command_interpreter(ch, "emote bounces lightly to another patch of"
                                    " grass.");
            break;
        }
        return TRUE;
    }
    return FALSE;
}

int real_fox(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *i;
    struct obj_data *j,
                   *k,
                   *next;

    if (cmd || !AWAKE(ch) || ch->specials.fighting) {
        return FALSE;
    }
    if (ch->generic) {
        ch->generic--;
        return TRUE;
    }

    for (j = real_roomp(ch->in_room)->contents; j; j = j->next_content) {
        if (GET_ITEM_TYPE(j) == ITEM_CONTAINER && j->obj_flags.value[3] &&
            !strcmp(j->name, "corpse rabbit")) {
            command_interpreter(ch, "emote gorges on the corpse of a rabbit.");
            for (k = j->contains; k; k = next) {
                next = k->next_content;
                obj_from_obj(k);
                obj_to_room(k, ch->in_room);
            }
            extract_obj(j);
            ch->generic = 10;
            return (TRUE);
        }
    }

    for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (IS_NPC(i) && mob_index[i->nr].virtual == 6001 && !number(0, 3)) {
            command_interpreter(ch, "emote yips and starts to make dinner.");
            hit(ch, i, TYPE_UNDEFINED);
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * holy hand-grenade of antioch code 
 */

int antioch_grenade(struct char_data *ch, int cmd, char *arg,
                    struct obj_data *obj, int type)
{

    if (type == PULSE_TICK && obj->obj_flags.value[0]) {
        obj->obj_flags.value[0] -= 1;
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
            obj->obj_flags.value[0] = 4;
        } else if (!strcmp(arg, "two") && obj->obj_flags.value[0] >= 3 && 
                   obj->obj_flags.value[0] <= 4) {
            obj->obj_flags.value[0] = 15;
        } else if (!strcmp(arg, "three") && obj->obj_flags.value[0] >= 14) {
            obj->obj_flags.value[0] += 10;
        } else if (!strcmp(arg, "five")) {
            if (obj->obj_flags.value[0] >= 14 && 
                obj->obj_flags.value[0] <= 15) {
                obj->obj_flags.value[0] = 35;
            } else {
                obj->obj_flags.value[0] = 0;
            }
        } else {
            return (0);
        }
    }
    return (0);
}


int arena_prep_room(struct char_data *ch, int cmd, char *arg,
                    struct room_data *rp, int type)
{

    char            buf[MAX_STRING_LENGTH + 30];
    extern int      preproomexitsquad1[],
                    preproomexitsquad2[],
                    preproomexitsquad3[],
                    preproomexitsquad4[],
                    Quadrant;

    if (cmd == 1) {
        send_to_char("You enter the arena!\n\r", ch);
        sprintf(buf, "%s leaves the preparation room\n\r", GET_NAME(ch));
        send_to_room_except(buf, ch->in_room, ch);
        char_from_room(ch);

        switch (Quadrant) {
        case 1:
            char_to_room(ch, preproomexitsquad1[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 2:
            char_to_room(ch, preproomexitsquad2[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 3:
            char_to_room(ch, preproomexitsquad3[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        case 4:
            char_to_room(ch, preproomexitsquad4[(dice(1, 4) - 1)]);
            do_look(ch, NULL, 0);
            return (TRUE);
        default:
            Log("Major Screw Up In Arena Prep. Room!!");
            return (TRUE);
        }
        return (TRUE);
    }
    return (FALSE);
}

int arena_arrow_dispel_trap(struct char_data *ch, int cmd, char *arg,
                            struct room_data *rp, int type)
{

    char            buf[MAX_STRING_LENGTH + 30];

    if (cmd >= 1 &&  cmd <= 6) {
        if (dice(1, 100) < 65) {
            send_to_char("A magic arrow emerges from the wall and hits "
                         "you!\n\r", ch);
            sprintf(buf, "A magic arrow emerges from the wall hits %s\n\r",
                    GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            spell_dispel_magic(60, ch, ch, 0);
            do_move(ch, arg, cmd);
            return (TRUE);
        } else {
            send_to_char("A magic arrow emerges from the wall and almost hits"
                         " you!\n\r", ch);
            sprintf(buf, "A magic arrow emerges from the wall nearly hits "
                         "%s\n\r", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
#if 0             
            do_move(ch, arg, (cmd+999));
#endif            
            return (FALSE);
        }
    }
    return (FALSE);
}

int arena_fireball_trap(struct char_data *ch, int cmd, char *arg,
                        struct room_data *rp, int type)
{
    char            buf[MAX_STRING_LENGTH + 30];
    int             dam;

    if (cmd >= 1 && cmd <= 6 && dice(1, 100) < 70) {
        dam = dice(30, 6);
        if (saves_spell(ch, SAVING_SPELL)) {
            send_to_char("You barely avoid a fireball!", ch);
            sprintf(buf, "%s barely avoids a huge fireball!", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
#if 0            
            do_move(ch, arg, (cmd+999));
#endif            
            return (FALSE);
        } else {
            send_to_char("You are envelopped by burning flames!", ch);
            sprintf(buf, "A fireball strikes %s!", GET_NAME(ch));
            send_to_room_except(buf, ch->in_room, ch);
            MissileDamage(ch, ch, dam, SPELL_FIREBALL);
#if 0            
            do_move(ch, arg, (cmd+999));
#endif     
            return (FALSE);
        }
        return (FALSE);
    }
    return (FALSE);
}

int arena_dispel_trap(struct char_data *ch, int cmd, char *arg,
                      struct room_data *rp, int type)
{
    if (cmd >= 1 && cmd <= 6) {
        spell_dispel_magic(50, ch, ch, 0);
#if 0        
        do_move(ch, arg, (cmd+999));
#endif    
    }
    return (FALSE);
}

/*
 * Cthol (Xenon) 
 */
int dispel_room(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    if (cmd == 5) {
        /* 
         * Up 
         */
        spell_dispel_magic(50, ch, ch, 0);
    }
    return (FALSE);
}

int fiery_alley(struct char_data *ch, int cmd, char *arg,
                struct room_data *rp, int type)
{
    if (ch->in_room == 40287 && cmd == 4) {
        /* 
         * West 
         */
        spell_fireball(40, ch, ch, 0);
        return (FALSE);
    }

    if (ch->in_room == 40285 && cmd == 2) {
        /* 
         * East 
         */
        spell_fireball(40, ch, ch, 0);
        return (FALSE);
    }
    return (FALSE);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
