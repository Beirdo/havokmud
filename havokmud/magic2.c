#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

/*
 * Extern structures
 */
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern long     SystemFlags;
extern int      ArenaNoGroup,
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

void            do_snoop(struct char_data *ch, char *argument, int cmd);

/*
 * cleric spells
 */

/*
 **   requires the sacrifice of 150k coins, victim loses a con point, and
 **   caster is knocked down to 1 hp, 1 mp, 1 mana, and sits for a LONG
 **   time (if a pc)
 */
void spell_resurrection(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_file_u st;
    struct affected_type af;
    struct obj_data *obj_object,
                   *next_obj;
    FILE           *fl;

    if (!obj) {
        return;
    }
    if (IS_CORPSE(obj)) {
        if (obj->char_vnum) {
            /*
             * corpse is a npc
             */

            if (GET_GOLD(ch) < 25000) {
                send_to_char("The gods are not happy with your sacrifice.\n\r",
                             ch);
                return;
            } else {
                GET_GOLD(ch) -= 25000;
            }

            victim = read_mobile(obj->char_vnum, VIRTUAL);
            char_to_room(victim, ch->in_room);
            GET_GOLD(victim) = 0;
            GET_EXP(victim) = 0;
            GET_HIT(victim) = 1;
            GET_POS(victim) = POSITION_STUNNED;

            act("With mystic power, $n resurrects a corpse.", TRUE, ch,
                0, 0, TO_ROOM);
            act("$N slowly rises from the ground.", FALSE, ch, 0, victim,
                TO_ROOM);

            /*
             * should be charmed and follower ch
             */

            if (IsImmune(victim, IMM_CHARM) || IsResist(victim, IMM_CHARM) ||
                IS_IMMORTAL(victim)) {
                act("$n says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
            } else if (too_many_followers(ch)) {
                act("$N takes one look at the size of your posse and just says"
                    " no!", TRUE, ch, 0, victim, TO_CHAR);
                act("You take one look at the size of $n's posse and just say"
                    " no!", TRUE, ch, 0, victim, TO_ROOM);
            } else {
                af.type = SPELL_CHARM_PERSON;
                af.duration = follow_time(ch);
                af.modifier = 0;
                af.location = 0;
                af.bitvector = AFF_CHARM;

                affect_to_char(victim, &af);
                add_follower(victim, ch);
            }

            IS_CARRYING_W(victim) = 0;
            IS_CARRYING_N(victim) = 0;

            /*
             * take all from corpse, and give to person
             */

            for (obj_object = obj->contains; obj_object;
                 obj_object = next_obj) {
                next_obj = obj_object->next_content;
                obj_from_obj(obj_object);
                obj_to_char(obj_object, victim);
            }

            /*
             * get rid of corpse
             */
            extract_obj(obj);
        } else {
            /*
             * corpse is a pc
             */
            if (GET_GOLD(ch) < 75000) {
                send_to_char("The gods are not happy with your sacrifice.\n\r",
                             ch);
                return;
            } else {
                GET_GOLD(ch) -= 75000;
            }

            fl = fopen(PLAYER_FILE, "r+");
            if (!fl) {
                perror("player file");
                assert(0);
            }
            rewind(fl);
            fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
            fread(&st, sizeof(struct char_file_u), 1, fl);
            /*
             * this is a serious kludge, and must be changed before multiple
             * languages can be implemented
             */
#if 0
            if (st.talks[2] && st.abilities.con > 3) {
#else
            if (!get_char(st.name) && st.abilities.con > 3) {
#endif
                st.points.exp *= 2;
                st.points.leadership_exp *= 2;
                st.m_deaths--;
                st.talks[2] = TRUE;
                st.abilities.con -= 1;
                act("A clear bell rings throughout the heavens",
                    TRUE, ch, 0, 0, TO_CHAR);
                act("A ghostly spirit smiles, and says 'Thank you'",
                    TRUE, ch, 0, 0, TO_CHAR);
                act("A clear bell rings throughout the heavens",
                    TRUE, ch, 0, 0, TO_ROOM);
                act("A ghostly spirit smiles, and says 'Thank you'",
                    TRUE, ch, 0, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                GET_MANA(ch) = 1;
                GET_MOVE(ch) = 1;
                GET_HIT(ch) = 1;
                GET_POS(ch) = POSITION_STUNNED;
                act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
                send_to_char("You collapse from the effort\n\r", ch);
                rewind(fl);
                fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
                fwrite(&st, sizeof(struct char_file_u), 1, fl);
                ObjFromCorpse(obj);
            } else {
                send_to_char("The body does not have the strength to be "
                             "recreated.\n\r", ch);
            }
            fclose(fl);
        }
    }
}

void spell_cause_light(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(1, 8);

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_LIGHT);
}

void spell_cause_critical(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(3, 8) + 3;

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_CRITICAL);
}

void spell_cause_serious(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + 2;

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_CAUSE_SERIOUS);
}

void spell_cure_serious(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + 2;

    if ((dam + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += dam;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);
#if 0
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=2;
    }
#endif
}

void spell_mana(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = level * 4;

    if (GET_MANA(ch) + dam > GET_MAX_MANA(ch)) {
        GET_MANA(ch) = GET_MAX_MANA(ch);
    } else {
        GET_MANA(ch) += dam;
    }
}

void spell_second_wind(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct affected_type *k;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 8) + level;

    if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
        GET_MOVE(victim) = move_limit(victim);
    } else {
        GET_MOVE(victim) += dam;
    }
    if (affected_by_spell(victim, COND_WINGS_TIRED)) {
        affect_from_char(victim, COND_WINGS_TIRED);
    }
    if (affected_by_spell(victim, COND_WINGS_FLY)) {
        for (k = victim->affected; k; k = k->next) {
            if (k->type == COND_WINGS_FLY) {
                k->duration = GET_CON(victim);
            }
        }
    }
    send_to_char("You feel less tired\n\r", victim);

}

void spell_flamestrike(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(6, 8);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    } else if (!saves_spell(victim, SAVING_SPELL - 4)) {
        BurnWings(victim);
        /*
         * Fail two saves, burn the wings
         */
    }
    heat_blind(victim);
    MissileDamage(ch, victim, dam, SPELL_FLAMESTRIKE);
}

void spell_dispel_good(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int     dam;
    
    dam = dice(5,10);
    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (IsExtraPlanar(victim)) {
        if (IS_GOOD(ch)) {
            victim = ch;
        } else if (IS_EVIL(victim)) {
            act("Evil protects $N.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        }

        if (!saves_spell(victim, SAVING_SPELL)) {
            act("$n forces $N from this plane.", TRUE, ch, 0, victim,
                TO_NOTVICT);
            act("You force $N from this plane.", TRUE, ch, 0, victim, TO_CHAR);
            act("$n forces you from this plane.", TRUE, ch, 0, victim, TO_VICT);
            gain_exp(ch, MIN(GET_EXP(victim) / 2, 50000));
            extract_char(victim);
        } else {
            act("$N resists the attack.", TRUE, ch, 0, victim, TO_CHAR);
            act("You resist $n's attack.", TRUE, ch, 0, victim, TO_VICT);
            damage(ch, victim, dam, SPELL_EARTHQUAKE);
        }
    } else {
        act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
        act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
    }
}

void spell_turn(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             diff,
                    i;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    if (IsUndead(victim)) {
        diff = level - GetTotLevel(victim);
        if (diff <= 0) {
            act("You are powerless to affect $N", TRUE, ch, 0, victim, TO_CHAR);
            return;
        } else {
            for (i = 1; i <= diff; i++) {
                if (!saves_spell(victim, SAVING_SPELL)) {
                    act("$n forces $N from this room.", TRUE, ch, 0,
                        victim, TO_NOTVICT);
                    act("You force $N from this room.", TRUE, ch, 0,
                        victim, TO_CHAR);
                    act("$n forces you from this room.", TRUE, ch, 0,
                        victim, TO_VICT);
                    do_flee(victim, "", 0);
                    break;
                }
            }
            if (i < diff) {
                act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
                act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
                act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
            }
        }
    } else {
        act("$n just tried to turn you, what a moron!", TRUE, ch, 0,
            victim, TO_VICT);
        act("$N thinks $n is really strange.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("Um... $N isn't undead...", TRUE, ch, 0, victim, TO_CHAR);
    }
}

void spell_remove_paralysis(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if (IS_AFFECTED(victim, AFF_PARALYSIS)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_PARALYSIS);
    }
    if (affected_by_spell(victim, SPELL_PARALYSIS)) {
        affect_from_char(victim, SPELL_PARALYSIS);
        act("A warm feeling runs through your body.", FALSE, victim, 0, 0,
            TO_CHAR);
        act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
    }
}

void spell_holy_word(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             max = 80;

    max += level;
    max += level / 2;

    if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > 53) {
        damage(ch, victim, GET_MAX_HIT(victim) * 12, SPELL_HOLY_WORD);
    } else {
        send_to_char("They are too powerful to destroy this way\n\r", ch);
    }
}

void spell_unholy_word(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             max = 80;

    max += level;
    max += level / 2;

    if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > 53) {
        damage(ch, victim, GET_MAX_HIT(victim) * 12, SPELL_UNHOLY_WORD);
    } else {
        send_to_char("They are too powerful to destroy this way\n\r", ch);
    }
}

void spell_succor(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *o;

    o = read_object(3052, VIRTUAL);
    obj_to_char(o, ch);

    o->obj_flags.cost = 0;
    o->obj_flags.cost_per_day = -1;

    act("$n waves $s hand, and creates $p.", TRUE, ch, o, 0, TO_ROOM);
    act("You wave your hand and create $p.", TRUE, ch, o, 0, TO_CHAR);
}

void spell_true_seeing(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!IS_AFFECTED(victim, AFF_TRUE_SIGHT)) {
        if (ch != victim) {
            send_to_char("Your eyes glow silver for a moment.\n\r", victim);
            act("$n's eyes take on a silvery hue.", FALSE, victim, 0, 0,
                TO_ROOM);
        } else {
            send_to_char("Your eyes glow silver.\n\r", ch);
            act("$n's eyes glow silver.", FALSE, ch, 0, 0, TO_ROOM);
        }

        af.type = SPELL_TRUE_SIGHT;
        af.duration = 2 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_TRUE_SIGHT;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing seems to happen.\n\r", ch);
    }
}

/*
 * magic user spells
 */
void spell_major_track(int level, struct char_data *ch,
                       struct char_data *targ, struct obj_data *obj)
{
    struct affected_type af;

    if (ch != targ) {
        send_to_char("You feel your awareness grow!\n\r", targ);
    } else {
        send_to_char("You feel your awareness grow!\n\r", ch);
    }

    act("$N's eyes take on an emerald hue for just a moment.", 0, ch, 0,
        targ, TO_ROOM);

    af.type = SPELL_MAJOR_TRACK;
    af.duration = level * 2;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(targ, &af);
}

void spell_minor_track(int level, struct char_data *ch,
                       struct char_data *targ, struct obj_data *obj)
{
    struct affected_type af;


    if (ch != targ) {
        send_to_char("You feel your awareness grow!\n\r", targ);
    } else {
        send_to_char("You feel your awareness grow!\n\r", ch);
    }

    act("$N's eyes take on an emerald hue for just a moment.", 0, ch, 0,
        targ, TO_ROOM);

    af.type = SPELL_MINOR_TRACK;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(targ, &af);
}

void spell_poly_self(int level, struct char_data *ch,
                     struct char_data *mob, struct obj_data *obj)
{
    char           *buf;

    /*
     * Checking for No_Poly system flag) - Manwe Windmaster 97/03/07
     */

    if (IS_SET(SystemFlags, SYS_NO_POLY)) {
        send_to_char("The gods prevent you from changing your shape", ch);
        extract_char(mob);
        return;
    }
    /*
     *  Check to make sure that there is no snooping going on.
     */
    if (!ch->desc || ch->desc->snoop.snooping) {
        send_to_char("Godly interference prevents the spell from working.", ch);
        extract_char(mob);
        return;
    }
    if (ch->desc->snoop.snoop_by) {
        /*
         * force the snooper to stop
         */
        do_snoop(ch->desc->snoop.snoop_by, GET_NAME(ch->desc->snoop.snoop_by),
                 0);
    }
    /*
     * Put mobile in correct room
     */
    char_to_room(mob, ch->in_room);
    SwitchStuff(ch, mob);
    /*
     *  move char to storage
     */
    act("$n's flesh melts and flows into the shape of $N.", TRUE, ch, 0, mob,
        TO_ROOM);
    act("Your flesh melts and flows into the shape of $N.", TRUE, ch, 0, mob,
        TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, 3);
    /*
     * stop following whoever you are following..
     */
    if (ch->master) {
        stop_follower(ch);
    }
    /*
     *  switch caster into mobile
     */
    ch->desc->character = mob;
    ch->desc->original = ch;
    mob->desc = ch->desc;
    ch->desc = 0;
    SET_BIT(mob->specials.act, ACT_POLYSELF);
    SET_BIT(mob->specials.act, ACT_NICE_THIEF);
    SET_BIT(mob->specials.act, ACT_SENTINEL);
    REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    REMOVE_BIT(mob->specials.act, ACT_META_AGG);
    REMOVE_BIT(mob->specials.act, ACT_SCAVENGER);
    /*
     * Why set current mana at max 85?
     * Look in to this. -Lennya
     */
    GET_MANA(mob) = MIN((GET_MANA(mob) - 15), 85);
    WAIT_STATE(mob, PULSE_VIOLENCE * 2);
    /*
     * do some fiddling with the strings
     */
    buf = (char *) malloc(strlen(GET_NAME(mob)) + strlen(GET_NAME(ch)) + 2);
    sprintf(buf, "%s %s", GET_NAME(ch), GET_NAME(mob));

#ifndef TITAN
    /*
     * this code crashes ardent titans
     */
    if (GET_NAME(mob)) {
        free(GET_NAME(mob));
    }
#endif

    GET_NAME(mob) = buf;
    buf = (char *) malloc(strlen(mob->player.short_descr) +
                          strlen(GET_NAME(ch)) + 2);
    sprintf(buf, "%s %s", GET_NAME(ch), mob->player.short_descr);

#ifdef TITAN
    if (mob->player.short_descr) {
        free(mob->player.short_descr);
    }
#endif
    mob->player.short_descr = buf;
#if 0
    buf = (char *)malloc(strlen(mob->player.short_descr)+12);
    sprintf(buf, "%s is here\n\r", mob->player.short_descr);
#endif
#ifndef TITAN
    if (mob->player.long_descr) {
        free(mob->player.long_descr);
    }
#endif
#if 0
    mob->player.long_descr = buf;
#endif
    /*
     * prettied up the way polies look in the room -Lennya
     */
    mob->player.long_descr = NULL;
}

void spell_minor_create(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    assert(ch && obj);

    act("$n claps $s hands together.", TRUE, ch, 0, 0, TO_ROOM);
    act("You clap your hands together.", TRUE, ch, 0, 0, TO_CHAR);
    act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_ROOM);
    act("In a flash of light, $p appears.", TRUE, ch, obj, 0, TO_CHAR);
    obj_to_room(obj, ch->in_room);
}

void spell_stone_skin(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
        act("$n's skin turns grey and granite-like.", TRUE, ch, 0, 0, TO_ROOM);
        act("Your skin turns to a stone-like substance.", TRUE, ch, 0, 0,
            TO_CHAR);

        af.type = SPELL_STONE_SKIN;
        af.duration = level;
        af.modifier = -40;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(ch, &af);

        /*
         * resistance to piercing weapons
         */

        af.type = SPELL_STONE_SKIN;
        af.duration = level;
        af.modifier = IMM_PIERCE;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }
}

void spell_infravision(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!IS_AFFECTED(victim, AFF_INFRAVISION)) {
        if (ch != victim) {
            send_to_char("Your eyes glow red.\n\r", victim);
            act("$n's eyes glow red.", FALSE, victim, 0, 0, TO_ROOM);
        } else {
            send_to_char("Your eyes glow red.\n\r", ch);
            act("$n's eyes glow red.", FALSE, ch, 0, 0, TO_ROOM);
        }

        af.type = SPELL_INFRAVISION;
        af.duration = 4 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_INFRAVISION;
        affect_to_char(victim, &af);

    }
}

void spell_shield(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (!affected_by_spell(victim, SPELL_SHIELD)) {
        act("$N is surrounded by a strong force shield.",
            TRUE, ch, 0, victim, TO_NOTVICT);
        if (ch != victim) {
            act("$N is surrounded by a strong force shield.",
                TRUE, ch, 0, victim, TO_CHAR);
            act("You are surrounded by a strong force shield.",
                TRUE, ch, 0, victim, TO_VICT);
        } else {
            act("You are surrounded by a strong force shield.",
                TRUE, ch, 0, 0, TO_CHAR);
        }

        af.type = SPELL_SHIELD;
        af.duration = 8 + level;
        af.modifier = -10;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}

void spell_weakness(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    float           modifier;

    assert(ch && victim);

    if (!affected_by_spell(victim, SPELL_WEAKNESS) &&
        !saves_spell(victim, SAVING_SPELL)) {
        modifier = level / 200.0;
        act("You feel weaker.", FALSE, victim, 0, 0, TO_VICT);
        act("$n seems weaker.", FALSE, victim, 0, 0, TO_ROOM);

        af.type = SPELL_WEAKNESS;
        af.duration = (int) level / 2;
        af.modifier = (int) 0 - (victim->abilities.str * modifier);
        if (victim->abilities.str_add) {
            af.modifier -= 2;
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}

void spell_invis_group(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim;
    struct affected_type af;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = tmp_victim->next_in_room) {
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim) &&
            in_group(ch, tmp_victim) &&
            !affected_by_spell(tmp_victim, SPELL_INVISIBLE)) {
            act("$n slowly fades out of existence.", TRUE,
                tmp_victim, 0, 0, TO_ROOM);
            send_to_char("You vanish.\n\r", tmp_victim);

            af.type = SPELL_INVISIBLE;
            af.duration = 24;
            af.modifier = -40;
            af.location = APPLY_AC;
            af.bitvector = AFF_INVISIBLE;
            affect_to_char(tmp_victim, &af);
        }
    }
}

void spell_acid_blast(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 6);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_ACID_BLAST);
}

void spell_cone_of_cold(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 5);

    send_to_char("A cone of freezing air fans out before you\n\r", ch);
    act("$n sends a cone of ice shooting from the fingertips!",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are chilled to the bone!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_CONE_OF_COLD);
            } else {
                act("You are able to avoid the cone!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
    }
}

void spell_ice_storm(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4);

    send_to_char("You conjure a storm of ice\n\r", ch);
    act("$n conjures an ice storm!", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are blasted by the storm",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_ICE_STORM);
            } else {
                act("You are able to dodge the storm!",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
    }
}


void spell_meteor_swarm(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 12);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_METEOR_SWARM);
}

void spell_Create_Monster(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *mob;
    int             rnum;
    struct room_data *rp;

    /*
     * load in a monster of the correct type, determined by level of the spell
     * really simple to start out with
     */

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There isn't enough room in here to summon that.\n\r", ch);
        return;
    }
    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }
    if (level <= 5) {
        mob = read_mobile(16034, VIRTUAL);
#if 0
        rnum = number(1, 10) + 200;
        /*
         * removed this plot of mobs
         */
        mob = read_mobile(rnum, VIRTUAL);
#endif
    } else if (level <= 7) {
        mob = read_mobile(9191, VIRTUAL);
#if 0
        rnum = number(1, 10) + 210;
        /*
         * removed this plot of mobs
         */
        mob = read_mobile(rnum, VIRTUAL);
#endif
    } else if (level <= 9) {
        rnum = number(1, 10) + 220;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 11) {
        rnum = number(1, 10) + 230;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 13) {
        rnum = number(1, 10) + 240;
        mob = read_mobile(rnum, VIRTUAL);
    } else if (level <= 15) {
        rnum = 250 + number(1, 10);
        mob = read_mobile(rnum, VIRTUAL);
    } else {
        rnum = 260 + number(1, 10);
        mob = read_mobile(rnum, VIRTUAL);
    }

    if (!mob) {
        send_to_char("The summons failed\n\r", ch);
        return;
    }

    /*
     * lets make monsum stronger.. they are pussies... (GH)
     */
    GET_DAMROLL(mob) += (int) level / 2;
    GET_HITROLL(mob) += (int) level / 2;
    mob->points.max_hit += level * 5;
#if 0
    GET_MAX_HIT(mob) += level*2;
#endif
    char_to_room(mob, ch->in_room);
    act("$n waves $s hand, and $N appears!", TRUE, ch, 0, mob, TO_ROOM);
    act("You wave your hand, and $N appears!", TRUE, ch, 0, mob, TO_CHAR);

    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd!!'",
            TRUE, ch, 0, mob, TO_ROOM);
        act("$N refuses to hang out with crowd of your size!!", TRUE, ch,
            0, mob, TO_CHAR);
    } else {
        /*
         * charm them for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);

        af.type = SPELL_CHARM_PERSON;

        if (GET_CHR(ch)) {
            if (IS_PC(ch) || ch->master) {
                af.duration = follow_time(ch);
                af.modifier = 0;
                af.location = 0;
                af.bitvector = AFF_CHARM;
                affect_to_char(mob, &af);
            } else {
                SET_BIT(mob->specials.affected_by, AFF_CHARM);
            }
        } else {
            af.duration = 1;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        }
    }

    /*
     * adjust the bits...
     * get rid of aggressive, add sentinel
     */

    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
    }
}

/*
 * either
 */

void spell_light(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{

    /*
     * creates a ball of light in the hands.
     */
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = read_object(20, VIRTUAL); /* this is all you have to do */
    if (tmp_obj) {
        tmp_obj->obj_flags.value[2] = 24 + level;
        obj_to_char(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the ball of light\n\r", ch);
        return;
    }

    act("$n twiddles $s thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_ROOM);
    act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_CHAR);
}

void spell_fly(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim && IS_SET(victim->specials.act, PLR_NOFLY)) {
        REMOVE_BIT(victim->specials.act, PLR_NOFLY);
    }

    if (victim && affected_by_spell(victim, SPELL_FLY)) {
        send_to_char("The spell seems to be wasted.\n\r", ch);
        return;
    }

    act("You feel lighter than air!", TRUE, ch, 0, victim, TO_VICT);
    if (victim != ch) {
        act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_CHAR);
    } else {
        send_to_char("Your feet rise up off the ground.\n\r", ch);
    }
    act("$N's feet rise off the ground.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type = SPELL_FLY;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_FLYING;
    affect_to_char(victim, &af);
}

void spell_fly_group(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *tch;

    assert(ch);

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    if (ch && IS_SET(ch->specials.act, PLR_NOFLY)) {
        REMOVE_BIT(ch->specials.act, PLR_NOFLY);
    }

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
        if (in_group(ch, tch)) {
            act("You feel lighter than air!", TRUE, ch, 0, tch, TO_VICT);
            if (tch != ch) {
                act("$N's feet rise off the ground.", TRUE, ch, 0, tch,
                    TO_CHAR);
            } else {
                send_to_char("Your feet rise up off the ground.", ch);
            }
            act("$N's feet rise off the ground.", TRUE, ch, 0, tch, TO_NOTVICT);

            af.type = SPELL_FLY;
            af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_FLYING;
            affect_to_char(tch, &af);
        }
    }
}

void spell_refresh(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct affected_type *k;

    assert(ch && victim);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4) + level;
    dam = MAX(dam, 20);

    if ((dam + GET_MOVE(victim)) > move_limit(victim)) {
        GET_MOVE(victim) = move_limit(victim);
    } else {
        GET_MOVE(victim) += dam;
    }
    if (affected_by_spell(victim, COND_WINGS_TIRED) ||
        affected_by_spell(victim, COND_WINGS_FLY)) {
        for (k = victim->affected; k; k = k->next) {
            if (k->type == COND_WINGS_TIRED) {
                k->duration = k->duration - dice(1, 10);
                if (k->duration < 0) {
                    affect_from_char(victim, COND_WINGS_TIRED);
                }
            } else if (k->type == COND_WINGS_FLY) {
                k->duration = k->duration + dice(1, 10);
                k->duration = MIN(k->duration, GET_CON(ch));
            }
        }
    }
    send_to_char("You feel less tired\n\r", victim);
}

void spell_water_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    act("You feel fishy!", TRUE, ch, 0, victim, TO_VICT);
    if (victim != ch) {
        act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_CHAR);
    }
    act("$N makes a face like a fish.", TRUE, ch, 0, victim, TO_NOTVICT);

    af.type = SPELL_WATER_BREATH;
    af.duration = GET_LEVEL(ch, BestMagicClass(ch)) + 3;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_WATERBREATH;
    affect_to_char(victim, &af);
}

void spell_cont_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = read_object(20, VIRTUAL);
    if (tmp_obj) {
        obj_to_char(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the ball of light\n\r", ch);
        return;
    }

    act("$n twiddles $s thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_ROOM);
    act("You twiddle your thumbs and $p suddenly appears.", TRUE, ch,
        tmp_obj, 0, TO_CHAR);
}

void spell_animate_dead(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *corpse)
{
    struct char_data *mob;
    struct obj_data *obj_object,
                   *next_obj;
    char            buf[MAX_STRING_LENGTH];
    /*
     * virtual # for zombie
     */
    int             r_num = 100;

    /*
     * some sort of check for corpse hood
     */
    if (GET_ITEM_TYPE(corpse) != ITEM_CONTAINER ||
        !corpse->obj_flags.value[3]) {
        send_to_char("The magic fails abruptly!\n\r", ch);
        return;
    }

    mob = read_mobile(r_num, VIRTUAL);
    char_to_room(mob, ch->in_room);

    act("With mystic power, $n animates a corpse.", TRUE, ch, 0, 0, TO_ROOM);
    act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_ROOM);
    act("$N slowly rises from the ground.", FALSE, ch, 0, mob, TO_CHAR);
    /*
     * zombie should be charmed and follower ch
     */

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, mob, TO_CHAR);
        act("You take one look at the size of $n's posse and just say no!",
            TRUE, ch, 0, mob, TO_ROOM);
    } else {
        SET_BIT(mob->specials.affected_by, AFF_CHARM);
        add_follower(mob, ch);
    }

    GET_EXP(mob) = 0;
    IS_CARRYING_W(mob) = 0;
    IS_CARRYING_N(mob) = 0;

    /*
     * take all from corpse, and give to zombie
     */

    for (obj_object = corpse->contains; obj_object; obj_object = next_obj) {
        next_obj = obj_object->next_content;
        obj_from_obj(obj_object);
        obj_to_char(obj_object, mob);
    }
    /*
     * set up descriptions and such
     */
    sprintf(buf, "%s is here, slowly animating\n\r", corpse->short_description);
    mob->player.long_descr = (char *) strdup(buf);
    /*
     * set up hitpoints
     */
    mob->points.max_hit = dice(MAX(level / 2, 5), 8);
    mob->points.hit = mob->points.max_hit / 2;
    mob->player.sex = 0;
    GET_RACE(mob) = RACE_UNDEAD_ZOMBIE;
    mob->player.class = ch->player.class;
    /*
     * get rid of corpse
     */
    extract_obj(corpse);
}

void spell_know_alignment(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             ap;
    char            buf[200],
                    name[100];

    assert(victim && ch);

    if (IS_NPC(victim)) {
        strcpy(name, victim->player.short_descr);
    } else {
        strcpy(name, GET_NAME(victim));
    }
    ap = GET_ALIGNMENT(victim);

    if (ap > 700) {
        sprintf(buf, "%s has an aura as white as the driven snow.\n\r", name);
    } else if (ap > 350) {
        sprintf(buf, "%s is of excellent moral character.\n\r", name);
    } else if (ap > 100) {
        sprintf(buf, "%s is often kind and thoughtful.\n\r", name);
    } else if (ap > 25) {
        sprintf(buf, "%s isn't a bad sort...\n\r", name);
    } else if (ap > -25) {
        sprintf(buf, "%s doesn't seem to have a firm moral commitment\n\r",
                name);
    } else if (ap > -100) {
        sprintf(buf, "%s isn't the worst you've come across\n\r", name);
    } else if (ap > -350) {
        sprintf(buf, "%s could be a little nicer, but who couldn't?\n\r", name);
    } else if (ap > -700) {
        sprintf(buf, "%s probably just had a bad childhood\n\r", name);
    } else {
        sprintf(buf, "I'd rather just not say anything at all about %s\n\r",
                name);
    }
    send_to_char(buf, ch);
}

void spell_dispel_magic(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             yes = 0;
    int             i;
    int             check_falling(struct char_data *ch);

    assert(ch && (victim || obj));
    if (A_NODISPEL(ch)) {
        send_to_char("The arena rules do not allow the use of dispel "
                     "magic!\n\r", ch);
        return;
    }

    if (obj) {
        if (IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE))
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);

        if (level >= 45) {
            /*
             * if level 45> then they can do this
             */
            if (IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {
                REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
            }
            /*
             * strip off everything
             */
            for (i = 0; i < MAX_OBJ_AFFECT; i++) {
                obj->affected[i].location = 0;
            }
        }

        if (level >= IMMORTAL) {
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_GLOW);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_HUM);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_EVIL);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_NEUTRAL);
        }
        return;
    }

    /*
     * gets rid of infravision, invisibility, detect, etc
     */
    if (GetMaxLevel(victim) <= level) {
        /* Changed so it's actually using scroll level (GH) */
        yes = TRUE;
    } else {
        yes = FALSE;
    }

    if (affected_by_spell(victim, SPELL_INVISIBLE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INVISIBLE);
        send_to_char("You feel exposed.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_INVISIBLE);
        send_to_char("You feel less perceptive.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_EVIL) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_EVIL);
        send_to_char("You feel less morally alert.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_GOOD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_GOOD);
        send_to_char("You can't sense the goodness around you anymore.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_MAGIC) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_MAGIC);
        send_to_char("You stop noticing the magic in your life.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SENSE_LIFE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SENSE_LIFE);
        send_to_char("You feel less in touch with living things.\n\r", victim);
    }

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_SANCTUARY);
            if (affected_by_spell(victim, SPELL_SANCTUARY)) {
                affect_from_char(victim, SPELL_SANCTUARY);
            }
            send_to_char("You don't feel so invulnerable anymore.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The white glow around $n's body fades.", FALSE,
                    victim, 0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROTECT_FROM_EVIL);
        send_to_char("You feel less morally protected.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROTECT_FROM_GOOD);
        send_to_char("Some of your evilness dissipates into the ether.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_INFRAVISION) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INFRAVISION);
        send_to_char("Your sight grows dimmer.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SLEEP) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SLEEP);
        send_to_char("You don't feel so tired.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_CHARM_PERSON) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CHARM_PERSON);
        send_to_char("You feel less enthused about your master.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WEAKNESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WEAKNESS);
        send_to_char("You don't feel so weak.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_STRENGTH);
        send_to_char("You don't feel so strong.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_ARMOR) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ARMOR);
        send_to_char("You don't feel so well protected.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_DETECT_POISON) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_DETECT_POISON);
        send_to_char("You don't feel so sensitive to fumes.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BLESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_BLESS);
        send_to_char("You don't feel so blessed.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FLY) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FLY);
        send_to_char("You don't feel lighter than air anymore.\n\r", victim);
        check_falling(victim);
    }

    if (affected_by_spell(victim, SPELL_WATER_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WATER_BREATH);
        send_to_char("You don't feel so fishy anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FIRE_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FIRE_BREATH);
        send_to_char("You don't feel so fiery anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_LIGHTNING_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_LIGHTNING_BREATH);
        send_to_char("You don't feel so electric anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GAS_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GAS_BREATH);
        send_to_char("You don't have gas anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FROST_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FROST_BREATH);
        send_to_char("You don't feel so frosty anymore.\n\r", victim);
    }

    if (IS_AFFECTED(victim, AFF_FIRESHIELD)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_FIRESHIELD);
            if (affected_by_spell(victim, SPELL_FIRESHIELD)) {
                affect_from_char(victim, SPELL_FIRESHIELD);
            }
            send_to_char("Your aura of burning flames suddenly winks out of"
                         " existence.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The fiery aura around $n's body fades.", FALSE,
                    victim, 0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (IS_AFFECTED(victim, AFF_CHILLSHIELD)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_CHILLSHIELD);
            if (affected_by_spell(victim, SPELL_CHILLSHIELD)) {
                affect_from_char(victim, SPELL_CHILLSHIELD);
            }
            send_to_char("Your aura of chill flames suddenly winks out of "
                         "existence.\n\r", victim);
            if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                act("The cold aura around $n's body fades.", FALSE, victim,
                    0, 0, TO_ROOM);
            }
        }

        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (IS_AFFECTED(victim, AFF_BLADE_BARRIER)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_BLADE_BARRIER);
            if (affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
                affect_from_char(victim, SPELL_BLADE_BARRIER);
            }
            send_to_char("Your blade barrier suddenly winks out of "
                         "existence.\n\r", victim);
            act("$n's blade barrier disappears.", FALSE, victim, 0, 0, TO_ROOM);
        }
        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_FAERIE_FIRE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FAERIE_FIRE);
        send_to_char("You don't feel so pink anymore.\n\r", victim);
        if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
            act("The pink glow around $n's body fades.", TRUE, ch, 0,
                0, TO_ROOM);
        }
    }

    if (affected_by_spell(victim, SPELL_MINOR_TRACK) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_MINOR_TRACK);
        send_to_char("You lose the trail.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_MAJOR_TRACK) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_MAJOR_TRACK);
        send_to_char("You lose the trail.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WEB)) {
        affect_from_char(victim, SPELL_WEB);
        send_to_char("You don't feel so sticky anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SILENCE)) {
        affect_from_char(victim, SPELL_SILENCE);
        send_to_char("You don't feel so quiet anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_TREE_TRAVEL)) {
        affect_from_char(victim, SPELL_TREE_TRAVEL);
        send_to_char("You don't feel so in touch with trees anymore.\n\r",
                     victim);
    }

    if (IS_SET(victim->specials.affected_by2, AFF2_HEAT_STUFF)) {
#if 0
        if (affected_by_spell(victim, SPELL_HEAT_STUFF)) {
#endif
        affect_from_char(victim, SPELL_HEAT_STUFF);
        send_to_char("You don't feel so hot anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HASTE)) {
        affect_from_char(victim, SPELL_HASTE);
        send_to_char("You don't feel so fast anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SLOW)) {
        affect_from_char(victim, SPELL_SLOW);
        send_to_char("You don't feel so slow anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BARKSKIN)) {
        affect_from_char(victim, SPELL_BARKSKIN);
        send_to_char("You don't feel so barky anymore\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_AID) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_AID);
        send_to_char("You feel less aided.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SHIELD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SHIELD);
        send_to_char("You feel less shielded.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_TRUE_SIGHT) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        REMOVE_BIT(victim->specials.affected_by, AFF_TRUE_SIGHT);
        affect_from_char(victim, SPELL_TRUE_SIGHT);
        send_to_char("You feel less keen.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS)) {
        affect_from_char(victim, SPELL_INVIS_TO_ANIMALS);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_UNDEAD)) {
        affect_from_char(victim, SPELL_INVIS_TO_UNDEAD);
    }

    if (affected_by_spell(victim, SPELL_DRAGON_RIDE)) {
        affect_from_char(victim, SPELL_DRAGON_RIDE);
    }

    if (IS_AFFECTED(victim, AFF_DARKNESS)) {
        if (yes || !saves_spell(victim, SAVING_SPELL)) {
            REMOVE_BIT(victim->specials.affected_by, AFF_DARKNESS);
            if (affected_by_spell(victim, SPELL_GLOBE_DARKNESS)) {
                affect_from_char(victim, SPELL_GLOBE_DARKNESS);
            }
            send_to_char("The dark globe surrounding you vanishes.\n\r",
                         victim);
            act("The globe of darkness surrounding $n vanishes.", FALSE,
                victim, 0, 0, TO_ROOM);
        }
        if (victim->attackers < 6 && !victim->specials.fighting &&
            IS_NPC(victim)) {
            set_fighting(victim, ch);
        }
    }

    if (affected_by_spell(victim, SPELL_GLOBE_MINOR_INV) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GLOBE_MINOR_INV);
        send_to_char("You see your globe of protection vanish.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GLOBE_MAJOR_INV) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GLOBE_MAJOR_INV);
        send_to_char("You see your globe of protection vanish.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_ENERGY_DRAIN) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_ENERGY_DRAIN);
        send_to_char("You feel fearful of vampiric creatures.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_DRAGON_BREATH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_DRAGON_BREATH);
        send_to_char("You feel the urge to avoid dragons now.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_WIZARDEYE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_WIZARDEYE);
        send_to_char("Your wizardeye disappears with a *pop*\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_FIRE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_FIRE);
        send_to_char("You feel the urge to avoid fire dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_FROST) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_FROST);
        send_to_char("You feel the urge to avoid frost dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_ELEC) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_ELEC);
        send_to_char("You feel the urge to avoid electric dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_ACID) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_ACID);
        send_to_char("You feel the urge to avoid acid dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_PROT_BREATH_GAS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_PROT_BREATH_GAS);
        send_to_char("You feel the urge to avoid gas dragons breath\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_GIANT_GROWTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GIANT_GROWTH);
        send_to_char("You feel less powerful.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_FIND_TRAPS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_FIND_TRAPS);
        send_to_char("Your sense of traps just left you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_STONE_SKIN) &&
        !saves_spell(victim, SAVING_SPELL)) {
        affect_from_char(victim, SPELL_STONE_SKIN);
        send_to_char("You skin softens considerably.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HOLY_ARMOR) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_HOLY_ARMOR);
        send_to_char("Your Deity's protection has left you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_HOLY_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_HOLY_STRENGTH);
        send_to_char("Your divine strength drains away.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_ENLIGHTENMENT) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ENLIGHTENMENT);
        send_to_char("You don't feel so terribly smart anymore.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_CIRCLE_PROTECTION) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CIRCLE_PROTECTION);
        send_to_char("You don't have a clue what you just lost, since it "
                     "doesn't do anything yet.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_AURA_POWER) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_AURA_POWER);
        send_to_char("You lose the strength granted by your Deity.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_INVIS_TO_UNDEAD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_INVIS_TO_UNDEAD);
        send_to_char("The eyes of the dead will see you.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SUIT_OF_BONE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SUIT_OF_BONE);
        send_to_char("Your suit of bones crumbles to dust.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_SPECTRAL_SHIELD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SPECTRAL_SHIELD);
        send_to_char("The battlescarred armor surrounding you disappears.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_CLINGING_DARKNESS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_CLINGING_DARKNESS);
        send_to_char("You feel the cloak of darkness lift from your eyes.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_SIPHON_STRENGTH) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_SIPHON_STRENGTH);
        send_to_char("Your strength turn back to normal.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_GATHER_SHADOWS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_GATHER_SHADOWS);
        send_to_char("The shadows around you dissipate into thin air.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_ENDURE_COLD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_ENDURE_COLD);
        send_to_char("Your skin loses it's white hue.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_EYE_OF_THE_DEAD) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_EYE_OF_THE_DEAD);
        send_to_char("Your eye of the dead goes blind and strays beyond your"
                     " grasp.\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_VAMPIRIC_EMBRACE) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_VAMPIRIC_EMBRACE);
        send_to_char("The aura of negative energy leaves your hands.\n\r",
                     victim);
    }

    if (affected_by_spell(victim, SPELL_IRON_SKINS) &&
        (yes || !saves_spell(victim, SAVING_SPELL))) {
        affect_from_char(victim, SPELL_IRON_SKINS);
        send_to_char("Your iron skins shatter leaving you unprotected.\n\r",
                     victim);
    }

    if (level >= IMPLEMENTOR) {
        if (affected_by_spell(victim, SPELL_STONE_SKIN)) {
            affect_from_char(victim, SPELL_STONE_SKIN);
            send_to_char("You skin softens considerably.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
            affect_from_char(victim, SPELL_ANTI_MAGIC_SHELL);
            send_to_char("Your anti-magic shell fizzles out.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_BLINDNESS)) {
            affect_from_char(victim, SPELL_BLINDNESS);
            send_to_char("Your vision returns.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_PARALYSIS)) {
            affect_from_char(victim, SPELL_PARALYSIS);
            send_to_char("You feel freedom of movement.\n\r", victim);
        }

        if (affected_by_spell(victim, SPELL_DARK_PACT)) {
            affect_from_char(victim, SPELL_DARK_PACT);
            send_to_char("Your pact with your Lord is cruelly ripped "
                         "apart.\n\r", victim);
        }
        if (affected_by_spell(victim, COND_WINGS_FLY)) {
            affect_from_char(victim, COND_WINGS_FLY);
        }
        if (IS_AFFECTED2(victim, AFF2_WINGSBURNED)) {
            REMOVE_BIT(victim->specials.affected_by2, AFF2_WINGSBURNED);
        }
        if (IS_AFFECTED2(victim, AFF2_WINGSTIRED)) {
            REMOVE_BIT(victim->specials.affected_by2, AFF2_WINGSTIRED);
        }
        if (IS_AFFECTED(victim, COND_WINGS_BURNED)) {
            affect_from_char(victim, COND_WINGS_BURNED);
        }
        if (IS_AFFECTED(victim, COND_WINGS_TIRED)) {
            affect_from_char(victim, COND_WINGS_TIRED);
        }
        if (affected_by_spell(victim, SPELL_POISON)) {
            affect_from_char(victim, SPELL_POISON);
        }
        if (affected_by_spell(victim, SPELL_DECAY)) {
            affect_from_char(victim, SPELL_DECAY);
        }
        if (affected_by_spell(victim, SPELL_DISEASE)) {
            affect_from_char(victim, SPELL_DISEASE);
        }
        if (affected_by_spell(victim, SPELL_FLESH_GOLEM)) {
            affect_from_char(victim, SPELL_FLESH_GOLEM);
        }
        if (affected_by_spell(victim, SPELL_MANA_SHIELD)) {
            affect_from_char(victim, SPELL_MANA_SHIELD);
        }
        if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
            affect_from_char(victim, SPELL_FEEBLEMIND);
        }
    }
}

void spell_paralyze(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
        if (IsImmune(victim, IMM_HOLD)) {
            FailPara(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_HOLD)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailPara(victim, ch);
                return;
            }
            if (saves_spell(victim, SAVING_PARA)) {
                FailPara(victim, ch);
                return;
            }
        }

        if (!IsSusc(victim, IMM_HOLD) && saves_spell(victim, SAVING_PARA)) {
            FailPara(victim, ch);
            return;
        } else if (IsSusc(victim, IMM_HOLD) &&
                   saves_spell(victim, SAVING_PARA) &&
                   saves_spell(victim, SAVING_PARA)) {
            FailPara(victim, ch);
            return;
        }

        af.type = SPELL_PARALYSIS;
        af.duration = 4 + level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_PARALYSIS;
        affect_join(victim, &af, FALSE, FALSE);

        act("Your limbs freeze in place", FALSE, victim, 0, 0, TO_CHAR);
        act("$n is paralyzed!", TRUE, victim, 0, 0, TO_ROOM);
        GET_POS(victim) = POSITION_STUNNED;
    } else {
        send_to_char("Someone tries to paralyze you AGAIN!\n\r", victim);
    }
}

void spell_fear(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(victim && ch);

    if (GetMaxLevel(ch) >= GetMaxLevel(victim) - 2) {
        if (!saves_spell(victim, SAVING_SPELL)) {
            do_flee(victim, "", 0);
        } else {
            send_to_char("You feel afraid, but the effect fades.\n\r", victim);
        }
    }
}

void spell_calm(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    /*
     * removes aggressive bit from monsters
     */
    if (IS_NPC(victim)) {
        if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
            if (HitOrMiss(ch, victim, CalcThaco(ch))) {
                REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
                send_to_char("You sense peace.\n\r", ch);
            }
        } else {
            send_to_char("You feel calm\n\r", victim);
        }
    } else {
        send_to_char("You feel calm.\n\r", victim);
    }
}

void spell_web(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    bool            fail,
                    pissed,
                    big,
                    no_effect;

    assert(ch && victim);

    big = pissed = fail = no_effect = FALSE;

    switch (GET_RACE(victim)) {
    case RACE_ARACHNID:
    case RACE_SLIME:
    case RACE_GHOST:
        act("$N laughs at the webs!", FALSE, ch, 0, victim, TO_CHAR);
        act("Hah, $n casted web on you, what a flake.", FALSE, ch, 0,
            victim, TO_VICT);
        act("$N laughs at $n's webs!", FALSE, ch, 0, victim, TO_NOTVICT);
        return;
        break;
    }

    if (!saves_spell(victim, SAVING_PARA)) {
        fail = TRUE;
    }
    if (IS_NPC(victim) && IS_SET(victim->specials.act, ACT_HUGE)) {
        big = TRUE;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        if (IsSmall(victim) && !fail && !number(0, 3)) {
            pissed = TRUE;
            /*
             * 25%
             */
        } else if (big) {
            if (fail) {
                if (number(0, 4) < 2) {
                    pissed = TRUE;
                    /*
                     * 40%
                     */
                }
            } else {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            }
        } else {
            if (fail) {
                if (!number(0, 4)) {
                    pissed = TRUE;
                    /*
                     * 20%
                     */
                }
            } else {
                if (!number(0, 2)) {
                    pissed = TRUE;
                    /*
                     * 33%
                     */
                }
            }
        }
    } else {
        /*
         * assume if not indoors, outdoors and web is less affective at
         * blocking the victim from the caster.
         */
        if (IsSmall(victim) && !fail && !number(0, 2)) {
            pissed = TRUE;
            /*
             * 33%
             */
        } else if (big) {
            if (fail) {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            } else {
                pissed = TRUE;
                /*
                 * 100%
                 */
            }
        } else {
            if (fail) {
                if (number(0, 4) < 2) {
                    pissed = TRUE;
                    /*
                     * 40%
                     */
                }
            } else {
                if (number(0, 4) < 3) {
                    pissed = TRUE;
                    /*
                     * 60%
                     */
                }
            }
        }
    }

    if (fail) {
        af.type = SPELL_WEB;
        af.duration = level;
        af.modifier = -50;
        af.location = APPLY_MOVE;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        if (!pissed) {
            act("You are stuck in a sticky webbing!", FALSE, ch, 0, victim,
                TO_VICT);
            act("$N is stuck in a sticky webbing!", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You wrap $N in a sticky webbing!", FALSE, ch, 0, victim,
                TO_CHAR);
        } else {
            act("You are wrapped in webs, but they don't stop you!", FALSE,
                ch, 0, victim, TO_VICT);
            act("$N attacks, paying little heed to the webs that slow it.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You only manage to piss off $N with your webs, ack!",
                FALSE, ch, 0, victim, TO_CHAR);
        }
    } else {
        if (pissed) {
            act("You are almost caught in a sticky webbing, GRRRR!",
                FALSE, ch, 0, victim, TO_VICT);
            act("$N growls and dodges $n's sticky webbing!",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You miss $N with your sticky webbing!  Uh oh, it's mad.",
                FALSE, ch, 0, victim, TO_CHAR);
        } else {
            act("You watch with amusement as $n casts web about the room.",
                FALSE, ch, 0, victim, TO_VICT);
            act("$n misses $N with the webs!", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You miss with your webs, but $N doesn't seem to notice.",
                FALSE, ch, 0, victim, TO_CHAR);
        }
    }
    if (pissed && IS_NPC(victim) && !victim->specials.fighting) {
        set_fighting(victim, ch);
    }
}

void spell_heroes_feast(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tch;

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
        if ((in_group(tch, ch)) && (GET_POS(ch) > POSITION_SLEEPING)) {
            send_to_char("You partake of a magnificent feast!\n\r", tch);
            gain_condition(tch, FULL, 24);
            gain_condition(tch, THIRST, 24);
            if (GET_HIT(tch) < GET_MAX_HIT(tch)) {
                GET_HIT(tch) += 1;
            }
            GET_MOVE(tch) = GET_MAX_MOVE(tch);
        }
    }
}

void spell_conjure_elemental(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    /*
     *   victim, in this case, is the elemental
     *   object could be the sacrificial object
     */

    assert(ch && victim && obj);

    /*
     * objects:
     *     fire  : red stone
     *     water : pale blue stone
     *     earth : grey stone
     *     air   : clear stone
     */

    act("$n gestures, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_ROOM);
    act("You gesture, and a cloud of smoke appears", TRUE, ch, 0, 0, TO_CHAR);
    act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_ROOM);
    act("$p explodes with a loud BANG!", TRUE, ch, obj, 0, TO_CHAR);
    obj_from_char(obj);
    extract_obj(obj);
    char_to_room(victim, ch->in_room);
    act("Out of the smoke, $N emerges", TRUE, ch, 0, victim, TO_NOTVICT);

    /*
     * charm them for a while
     */

    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd, but thanks for the"
            " lift'", TRUE, ch, 0, victim, TO_ROOM);
        act("$N refuses to hang out with crowd of your size, but thanks you "
            "for the free ride.", TRUE, ch, 0, victim, TO_CHAR);
    } else {
        if (victim->master) {
            stop_follower(victim);
        }

        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;
        af.duration = 20;
        af.modifier = follow_time(ch);
        af.location = 0;
        af.bitvector = AFF_CHARM;

        affect_to_char(victim, &af);
    }
}

void spell_faerie_fire(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (affected_by_spell(victim, SPELL_FAERIE_FIRE)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    act("$n points at $N.", TRUE, ch, 0, victim, TO_ROOM);
    act("You point at $N.", TRUE, ch, 0, victim, TO_CHAR);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_ROOM);
    act("$N is surrounded by a pink outline", TRUE, ch, 0, victim, TO_CHAR);

    af.type = SPELL_FAERIE_FIRE;
    af.duration = level;
    af.modifier = 20;
    af.location = APPLY_ARMOR;
    af.bitvector = 0;

    affect_to_char(victim, &af);
}

void spell_faerie_fog(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim;
    int             revealed = 0;

    assert(ch);

    act("$n snaps $s fingers, and a cloud of purple smoke billows forth",
        TRUE, ch, 0, 0, TO_ROOM);
    act("You snap your fingers, and a cloud of purple smoke billows forth",
        TRUE, ch, 0, 0, TO_CHAR);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = tmp_victim->next_in_room) {
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (IS_IMMORTAL(tmp_victim)) {
                break;
            }
            if (!in_group(ch, tmp_victim)) {
                if (IS_AFFECTED(tmp_victim, AFF_HIDE)) {
                    if (ImpSaveSpell(tmp_victim, SAVING_SPELL, -10)) {
                        REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        act("$n is briefly revealed, but disappears again.",
                            TRUE, tmp_victim, 0, 0, TO_ROOM);
                        act("You are briefly revealed, but disappear again.",
                            TRUE, tmp_victim, 0, 0, TO_CHAR);
                        SET_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        revealed = 1;
                    } else {
                        REMOVE_BIT(tmp_victim->specials.affected_by, AFF_HIDE);
                        act("$n is revealed!", TRUE, tmp_victim, 0, 0,
                            TO_ROOM);
                        act("You are revealed!", TRUE, tmp_victim, 0, 0,
                            TO_CHAR);
                        revealed = 1;
                    }
                }
                if (IS_AFFECTED(tmp_victim, AFF_INVISIBLE)) {
                    REMOVE_BIT(tmp_victim->specials.affected_by, AFF_INVISIBLE);
                    if (!revealed) {
                        act("$n is revealed!", TRUE, tmp_victim, 0, 0, TO_ROOM);
                        act("You are revealed!", TRUE, tmp_victim, 0, 0,
                            TO_CHAR);
                    }
                }
            }
        }
    }
}

void spell_cacaodemon(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim && obj);

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }

    act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0,
        TO_ROOM);
    act("$n gestures, and a black cloud of smoke appears", TRUE, ch, 0, 0,
        TO_CHAR);
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) > 40 && IS_EVIL(ch)) {
        act("$p smokes briefly", TRUE, ch, obj, 0, TO_ROOM);
        act("$p smokes briefly", TRUE, ch, obj, 0, TO_CHAR);
        obj->obj_flags.cost /= 2;
        if (obj->obj_flags.cost < 100) {
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_ROOM);
            act("$p bursts into flame and disintegrates!",
                TRUE, ch, obj, 0, TO_CHAR);
            obj_from_char(obj);
            extract_obj(obj);
        }
    } else {
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_ROOM);
        act("$p bursts into flame and disintegrates!", TRUE, ch, obj, 0,
            TO_CHAR);
        obj_from_char(obj);
        extract_obj(obj);
        GET_ALIGNMENT(ch) -= 5;
    }
    char_to_room(victim, ch->in_room);

    act("With an evil laugh, $N emerges from the smoke", TRUE, ch, 0,
        victim, TO_NOTVICT);

    if (too_many_followers(ch)) {
        act("$N says 'No way I'm hanging with that crowd!!'",
            TRUE, ch, 0, victim, TO_ROOM);
        act("$N refuses to hang out with crowd of your size!!", TRUE, ch,
            0, victim, TO_CHAR);
    } else {
        /*
         * charm them for a while
         */
        if (victim->master) {
            stop_follower(victim);
        }

        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;
        af.duration = follow_time(ch);
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;

        affect_to_char(victim, &af);
    }

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
    }
    if (IS_SET(victim->specials.act, ACT_META_AGG)) {
        REMOVE_BIT(victim->specials.act, ACT_META_AGG);
    }
    if (!IS_SET(victim->specials.act, ACT_SENTINEL)) {
        SET_BIT(victim->specials.act, ACT_SENTINEL);
    }
}

void spell_geyser(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    struct char_data *tmp_victim,
                   *temp;

    if (ch->in_room < 0) {
        return;
    }
    dam = dice(level, 3);

    act("The Geyser erupts in a huge column of steam!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people;
         tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        if ((ch != tmp_victim) && (ch->in_room == tmp_victim->in_room)) {
            if (!IS_IMMORTAL(tmp_victim)) {
                MissileDamage(ch, tmp_victim, dam, SPELL_GEYSER);
                act("You are seared by the boiling water!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            } else {
                act("You are almost seared by the boiling water!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        }
    }
}

void spell_green_slime(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    hpch = GET_MAX_HIT(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = (int) (hpch / 10);

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    send_to_char("You are attacked by green slime!\n\r", victim);
    damage(ch, victim, dam, SPELL_GREEN_SLIME);
}

void spell_prot_dragon_breath(int level, struct char_data *ch,
                              struct char_data *victim,
                              struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_DRAGON_BREATH)) {
        if (ch != victim) {
            act("$n summons a protective globe around $N", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You summon a protective globe about $N", FALSE, ch, 0,
                victim, TO_CHAR);
            act("$n summons a protective globe around you", FALSE, ch, 0,
                victim, TO_VICT);
        } else {
            act("$n summons a protective globe about $mself", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You summon a protective globe about yourself", FALSE, ch,
                0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_DRAGON_BREATH;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = (int) level / 10;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N is already surrounded by a protective globe");
        } else {
            sprintf(buf, "You are already surrounded by a protective globe");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_energy_drain(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROT_ENERGY_DRAIN)) {
        send_to_char("You call forth a protective shield against energy "
                     "drain.\n\r", ch);
        af.type = SPELL_PROT_ENERGY_DRAIN;
        af.duration = !IS_IMMORTAL(ch) ? 3 : level;
        af.modifier = IMM_DRAIN;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("You are already protected from energy drain.\n\r", ch);
    }
}

void spell_globe_darkness(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    assert(victim);

    if (!affected_by_spell(victim, SPELL_GLOBE_DARKNESS) &&
        !IS_AFFECTED(victim, AFF_DARKNESS)) {
        if (ch != victim) {
            act("$n summons a black globe around $N.", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You summon a black globe about $N.", FALSE, ch, 0, victim,
                TO_CHAR);
            act("$n summons a black globe around you.", FALSE, ch, 0,
                victim, TO_VICT);
        } else {
            act("$n summons a black globe about $mself.", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You summon a black globe about yourself.", FALSE, ch, 0,
                victim, TO_CHAR);
        }

        af.type = SPELL_GLOBE_DARKNESS;
        af.duration = level;
        af.modifier = 5;
        af.location = APPLY_NONE;
        af.bitvector = AFF_DARKNESS;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "%s is already surrounded by a globe of darkness.\n\r",
                GET_NAME(victim));
        send_to_char(buf, ch);
    } else {
        sprintf(buf, "You are already surrounded by a globe of darkness.\n\r");
        send_to_char(buf, ch);
    }
}

void spell_prot_fire(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_FIRE)) {
        if (ch != victim) {
            act("$n summons a fire protective globe around $N.", FALSE, ch,
                0, victim, TO_NOTVICT);
            act("You summon a fire protective globe about $N.", FALSE, ch,
                0, victim, TO_CHAR);
            act("$n summons a fire protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a fire protective globe about $mself.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a fire protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_FIRE;
        af.modifier = IMM_FIRE;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N is already surrounded by a fire protective "
                         "globe.");
        } else {
            sprintf(buf, "You are already surrounded by a fire protective "
                         "globe.");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_cold(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_COLD)) {
        if (ch != victim) {
            act("$n summons a cold protective globe around $N.", FALSE, ch,
                0, victim, TO_NOTVICT);
            act("You summon a cold protective globe about $N.", FALSE, ch,
                0, victim, TO_CHAR);
            act("$n summons a cold protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a cold protective globe about $mself.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a cold protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_COLD;
        af.modifier = IMM_COLD;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a cold protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a cold protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_energy(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_ENERGY)) {
        if (ch != victim) {
            act("$n summons a energy protective globe around $N.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a energy protective globe about $N.", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n summons a energy protective globe around you.", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a energy protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a energy protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_ENERGY;
        af.modifier = IMM_ENERGY;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a energy protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a energy protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_elec(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_ELEC)) {
        if (ch != victim) {
            act("$n summons a electric protective globe around $N.", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a electric protective globe about $N.", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n summons a electric protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a electric protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_ELEC;
        af.modifier = IMM_ELEC;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_dragon_breath_fire(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_FIRE)) {
        if (ch != victim) {
            act("$n summons a fire breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a fire breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a fire breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a fire breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a fire breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_FIRE;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a fire protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a fire protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_dragon_breath_frost(int level, struct char_data *ch,
                                    struct char_data *victim,
                                    struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_FROST)) {
        if (ch != victim) {
            act("$n summons a frost breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a frost breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a frost breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a frost breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a frost breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_FROST;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a frost protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a frost protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_dragon_breath_elec(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a electric breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a electric breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a electric breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a electric breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_ELEC;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim){
        sprintf(buf, "$N is already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a electric protective "
                     "globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_dragon_breath_acid(int level, struct char_data *ch,
                                   struct char_data *victim,
                                   struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a acid breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a acid breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a acid breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a acid breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a acid breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_ACID;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a acid protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a acid protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_prot_dragon_breath_gas(int level, struct char_data *ch,
                                  struct char_data *victim,
                                  struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_PROT_BREATH_ELEC)) {
        if (ch != victim) {
            act("$n summons a gas breath protective globe around $N.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a gas breath protective globe about $N.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n summons a gas breath protective globe around you.",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a gas breath protective globe about $mself.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a gas breath protective globe about yourself.",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_PROT_BREATH_GAS;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        af.duration = level / 10;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        sprintf(buf, "$N is already surrounded by a gas protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    } else {
        sprintf(buf, "You are already surrounded by a gas protective globe.");
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_wall_of_thought(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (!affected_by_spell(ch, SPELL_WALL_OF_THOUGHT)) {
        act("$n's wall of thought.", TRUE, ch, 0, 0, TO_ROOM);
        act("A wall of thought comes up.", TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_WALL_OF_THOUGHT;
        af.duration = level;
        af.modifier = -40;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(ch, &af);

        /*
         * resistance to piercing weapons
         */

        af.type = SPELL_WALL_OF_THOUGHT;
        af.duration = level;
        af.modifier = IMM_BLUNT;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
