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

/*
 * external vars 
 */



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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
