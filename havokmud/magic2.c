#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

/*
 * Extern structures
 */

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
                act("$N says 'Thank you'", FALSE, ch, 0, victim, TO_CHAR);
                act("$N says 'Thank you'", FALSE, ch, 0, victim, TO_ROOM);
            } else if (too_many_followers(ch)) {
                act("$N takes one look at the size of your posse and just says"
                    " no!", TRUE, ch, 0, victim, TO_CHAR);
                act("$N takes one look at the size of $n's posse and just says"
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







/*
 * either
 */














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

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
