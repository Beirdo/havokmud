#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

/*
 * Extern structures
 */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
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
extern const struct skillset weaponskills[];
struct time_info_data age(struct char_data *ch);
extern char    *spells[];
extern char    *RaceName[];
extern char    *AttackType[];
extern struct index_data *obj_index;
extern char    *item_types[];
extern char    *extra_bits[];
extern char    *apply_types[];
extern char    *affected_bits[];
extern char    *affected_bits2[];
extern char    *immunity_names[];
extern char    *wear_bits[];
extern struct char_data *auctioneer;

#define ASTRAL_ENTRANCE   2701

void            add_follower(struct char_data *ch, struct char_data *leader);
bool            circle_follow(struct char_data *ch, struct char_data *victim);
void            stop_follower(struct char_data *ch);
void            sprintbit(unsigned long, char *[], char *);

/*
 * For future use in blinding those with infravision who are fireballed or
 * otherwise subjected to lotsa heat quickly in dark rooms.
 */

void heat_blind(struct char_data *ch)
{
    struct affected_type af;
    byte            tmp;

    tmp = number(1, 4);

    if (!ch) {
        return;
    }
    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return;
    } else if (IS_DARK(ch->in_room) && !IS_IMMORTAL(ch) &&
               !IS_AFFECTED(ch, AFF_TRUE_SIGHT) &&
               IS_AFFECTED(ch, AFF_INFRAVISION)) {
        send_to_char("Aaarrrggghhh!!  The heat blinds you!!\n\r", ch);
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -4;
        af.duration = tmp;
        af.bitvector = AFF_BLIND;
        affect_to_char(ch, &af);
    }
}

/*
 * Offensive Spells
 */

void spell_magic_missile(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice((int) (level / 2) + 1, 4) + (level / 2) + 1;

    if (affected_by_spell(victim, SPELL_SHIELD)) {
        dam = 0;
    }
    MissileDamage(ch, victim, dam, SPELL_MAGIC_MISSILE);
}

void spell_chill_touch(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = number(level, 3 * level);

    if (!saves_spell(victim, SAVING_SPELL)) {
        af.type = SPELL_CHILL_TOUCH;
        af.duration = 6;
        af.modifier = -1;
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_join(victim, &af, TRUE, FALSE);
    } else {
        dam >>= 1;
    }
    damage(ch, victim, dam, SPELL_CHILL_TOUCH);
}

void spell_burning_hands(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(1, 4) + level / 2 + 1;

    send_to_char("Searing flame fans out in front of you!\n\r", ch);
    act("$n sends a fan of flame shooting from the fingertips!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
		rdam = dam;
        temp = tmp_victim->next_in_room;
        if (ch->in_room == tmp_victim->in_room && ch != tmp_victim) {
            if (GetMaxLevel(tmp_victim) > LOW_IMMORTAL &&
                !IS_NPC(tmp_victim)) {
                return;
            }
            if (!in_group(ch, tmp_victim)) {
                act("You are seared by the burning flame!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam = 0;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    BurnWings(tmp_victim);
                }
                MissileDamage(ch, tmp_victim, rdam, SPELL_BURNING_HANDS);
            } else {
                act("You are able to avoid the flames!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        }
    }
}

void spell_shocking_grasp(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = number(1, 8) + level;

    if ((GET_HIT(victim) < -4) && IsHumanoid(victim) && !IsUndead(victim)) {
        act("$n utters the words 'clear', and touches $N's chest",
            FALSE, ch, 0, victim, TO_ROOM);
        GET_HIT(victim) += dam;
        update_pos(victim);
        return;
    }

    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_SHOCKING_GRASP);
}

void spell_lightning_bolt(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 6);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BOLT);
}

void spell_colour_spray(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = 4 * level;

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_COLOUR_SPRAY);
}

void spell_energy_restore(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    char            buf[100];

    if (IS_IMMORTAL(victim)) {
        send_to_char("You tried to cast energy restore on an immortal... "
                     "oops...", ch);
        send_to_char("Some puny mortal tried to cast energy restore on you... "
                     "funny...", victim);
        return;
    }

    if (IS_NPC(victim)) {
        send_to_char("You casting energy restore on a mob, silly :P", ch);
        return;
    }

    if (victim->old_exp) {
        act("$n does a complex religious ritual!", FALSE, ch, 0, 0, TO_ROOM);
        sprintf(buf, "You call the gods to restore %s's lost energy!\n",
                GET_NAME(victim));
        send_to_char(buf, ch);
        send_to_char("You feel your life energy flowing back in your veins!\n",
                     victim);
        GET_EXP(victim) = victim->old_exp;
        victim->old_exp = 0;
        act("You are overcome by a wave of exhaustion.", FALSE, ch, 0, 0,
            TO_CHAR);
        act("$n slumps to the ground, exhausted.", FALSE, ch, 0, 0, TO_ROOM);
        if (!IS_IMMORTAL(ch)) {
            WAIT_STATE(ch, PULSE_VIOLENCE * 12);
            GET_POS(ch) = POSITION_STUNNED;
        }
    } else {
        send_to_char("This person can't be restored!", ch);
    }
}

/*
 * Drain XP, MANA, HP - caster gains HP and MANA
 */
void spell_energy_drain(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             tmp;

    void            set_title(struct char_data *ch);
    void            gain_exp(struct char_data *ch, int gain);

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!saves_spell(victim, SAVING_SPELL)) {
        GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 200);

        if (GetMaxLevel(victim) <= 1) {
            /*
             * Kill the sucker
             */
            damage(ch, victim, 100, SPELL_ENERGY_DRAIN);
        } else if (!IS_NPC(victim) && GetMaxLevel(victim) >= LOW_IMMORTAL) {
            send_to_char("Some puny mortal just tried to drain you...\n\r",
                         victim);
        } else if (!IS_SET(victim->M_immune, IMM_DRAIN) &&
                 !IS_SET(victim->immune, IMM_DRAIN) &&
                 !saves_spell(victim, SAVING_SPELL)) {
            send_to_char("Your life energy is drained!\n\r", victim);
            dam = 1;
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
            if (!IS_NPC(victim)) {
                victim->old_exp = GET_EXP(victim);
                drop_level(victim, BestClassBIT(victim), FALSE);
                set_title(victim);
            } else {
                tmp = GET_MAX_HIT(victim) / GetMaxLevel(victim);
                victim->points.max_hit -= tmp;
                GET_HIT(victim) -= tmp;
                tmp = GET_EXP(victim) / GetMaxLevel(victim);
                GET_EXP(ch) += tmp;
                GET_EXP(victim) -= tmp;
                victim->points.hitroll += 1;
            }
        } else if (!IS_SET(ch->M_immune, IMM_DRAIN) &&
                   !IS_SET(ch->immune, IMM_DRAIN) &&
                   !saves_spell(ch, SAVING_SPELL)) {
            send_to_char("Your spell backfires!\n\r", ch);
            dam = 1;
            damage(ch, victim, dam, SPELL_ENERGY_DRAIN);
            if (!IS_NPC(ch)) {
                ch->old_exp = GET_EXP(ch);
                drop_level(ch, BestClassBIT(ch), FALSE);
                set_title(ch);
            } else {
                tmp = GET_MAX_HIT(victim) / GetMaxLevel(victim);
                victim->points.max_hit -= tmp;
                GET_HIT(victim) -= tmp;
                victim->points.hitroll += 1;
                tmp = GET_EXP(victim) / GetMaxLevel(victim);
                GET_EXP(ch) += tmp;
                GET_EXP(victim) -= tmp;
            }
        } else {
            send_to_char("Your spell fails utterly.\n\r", ch);
        }
    } else {
        /*
         * Miss
         */
        damage(ch, victim, 0, SPELL_ENERGY_DRAIN);
    }
}

void spell_fireball(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 8);
    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    /*
                     * Fail the second save, then their wings are toast
                     * for two weeks
                     */
                    BurnWings(tmp_victim);
                }
                heat_blind(tmp_victim);
                MissileDamage(ch, tmp_victim, rdam, SPELL_FIREBALL);
            } else {
                act("You dodge the mass of flame!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        } else if (tmp_victim->in_room != NOWHERE &&
                   real_roomp(ch->in_room)->zone ==
                   real_roomp(tmp_victim->in_room)->zone) {
            send_to_char("You feel a blast of hot air.\n\r", tmp_victim);
        }
    }
}

void spell_earthquake(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(1, 4) + level + 1;

    send_to_char("The earth trembles beneath your feet!\n\r", ch);
    act("$n makes the earth tremble and shiver", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim) &&
                !IS_AFFECTED(tmp_victim, AFF_FLYING)) {

                if (GetMaxLevel(tmp_victim) > 4) {
                    act("You fall and hurt yourself!!\n\r",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    MissileDamage(ch, tmp_victim, dam, SPELL_EARTHQUAKE);
                } else {
                    act("You are sucked into a huge hole in the ground!",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    act("$N is sucked into a huge hole in the ground!",
                        FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                    MissileDamage(ch, tmp_victim, GET_MAX_HIT(tmp_victim) * 12,
                                  SPELL_EARTHQUAKE);
                }
            } else {
                act("You almost fall and hurt yourself!!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
            }
        } else {
            if (real_roomp(ch->in_room)->zone ==
                real_roomp(tmp_victim->in_room)->zone)
                send_to_char("The earth trembles...\n\r", tmp_victim);
        }
    }
}

void spell_dispel_evil(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam = 1;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (IsExtraPlanar(victim)) {
        if (IS_EVIL(ch)) {
            victim = ch;
        } else if (IS_GOOD(victim)) {
            act("Good protects $N.", FALSE, ch, 0, victim, TO_CHAR);
            return;
        }

        if (!saves_spell(victim, SAVING_SPELL)) {
            act("$n forces $N from this plane.", TRUE, ch, 0, victim,
                TO_ROOM);
            act("You force $N from this plane.", TRUE, ch, 0, victim,
                TO_CHAR);
            act("$n forces you from this plane.", TRUE, ch, 0, victim,
                TO_VICT);
            gain_exp(ch, GET_EXP(victim) / 2);
            extract_char(victim);
        } else {
            act("$N resists the attack", TRUE, ch, 0, victim, TO_CHAR);
            act("you resist $n's attack.", TRUE, ch, 0, victim, TO_VICT);
            damage(ch, victim, dam, SPELL_EARTHQUAKE);
        }
    } else {
        act("$N laughs at you.", TRUE, ch, 0, victim, TO_CHAR);
        act("$N laughs at $n.", TRUE, ch, 0, victim, TO_NOTVICT);
        act("You laugh at $n.", TRUE, ch, 0, victim, TO_VICT);
    }
}

void spell_call_lightning(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    extern struct weather_data weather_info;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level + 2, 8);

    if (OUTSIDE(ch) && (weather_info.sky >= SKY_RAINING)) {
        if (saves_spell(victim, SAVING_SPELL)) {
            dam >>= 1;
        }
        MissileDamage(ch, victim, dam, SPELL_CALL_LIGHTNING);
    } else {
        send_to_char("The proper atmospheric conditions are not at hand.\n\r",
                     ch);
        return;
    }
}

void spell_harm(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = GET_HIT(victim) - dice(1, 4);

    if (dam < 0) {
        /*
         * Kill the suffering bastard
         */
        dam = 100;
    } else {
        if (GET_RACE(ch) == RACE_GOD) {
            dam = 0;
        }
        if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
            dam = 0;
        }
    }
    dam = MIN(dam, 100);

    damage(ch, victim, dam, SPELL_HARM);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) -= 4;
    }
}


void spell_armor(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_ARMOR)) {
        af.type = SPELL_ARMOR;
        af.duration = 24;
        af.modifier = -20;
        af.location = APPLY_AC;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        send_to_char("You feel someone protecting you.\n\r", victim);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}


void spell_astral_walk(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp,
                   *tmp2;
    struct room_data *rp;

    if (IS_SET(SystemFlags, SYS_NOASTRAL)) {
        send_to_char("The astral planes are shifting, you cannot!\n", ch);
        return;
    }

    rp = real_roomp(ch->in_room);

    for (tmp = rp->people; tmp; tmp = tmp2) {
        tmp2 = tmp->next_in_room;
        if (in_group(ch, tmp) && !tmp->specials.fighting &&
            (IS_PC(tmp) || IS_SET(tmp->specials.act, ACT_POLYSELF)) &&
            IS_AFFECTED(tmp, AFF_GROUP)) {
            act("$n wavers, fades and dissappears", FALSE, tmp, 0, 0, TO_ROOM);
            char_from_room(tmp);
            char_to_room(tmp, ASTRAL_ENTRANCE);
            do_look(tmp, "\0", 0);
        }
    }
}

void spell_teleport(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             to_room,
                    try = 0;
    extern int      top_of_world;
    struct room_data *room;

    assert(ch && victim);

    if (victim != ch) {
        if (saves_spell(victim, SAVING_SPELL)) {
            send_to_char("Your spell has no effect.\n\r", ch);
            if (IS_NPC(victim)) {
                if (!victim->specials.fighting) {
                    set_fighting(victim, ch);
                }
            } else {
                send_to_char("You feel strange, but the effect fades.\n\r",
                             victim);
            }
            return;
        } else {
            /*
             * the character (target) is now the victim
             */
            ch = victim;
        }
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    do {
        to_room = number(0, top_of_world);
        room = real_roomp(to_room);
        if (room && ((IS_SET(room->room_flags, PRIVATE)) ||
                     (IS_SET(room->room_flags, TUNNEL)) ||
                     (IS_SET(room->room_flags, NO_SUM)) ||
                     (IS_SET(room->room_flags, NO_MAGIC)) ||
                     !IsOnPmp(to_room))) {
            room = 0;
            try++;
        }
    } while (!room && try < 10);

    if (try >= 10) {
        send_to_char("The magic fails.\n\r", ch);
        return;
    }

    act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, to_room);
    act("$n slowly fades in to existence.", FALSE, ch, 0, 0, TO_ROOM);

    do_look(ch, "", 0);

    if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
        GetMaxLevel(ch) < LOW_IMMORTAL) {
        NailThisSucker(ch);
        return;
    }
    check_falling(ch);
}

void spell_bless(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && (victim || obj));

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (GET_POS(victim) != POSITION_FIGHTING &&
        !affected_by_spell(victim, SPELL_BLESS)) {

        send_to_char("You feel righteous.\n\r", victim);
        af.type = SPELL_BLESS;
        af.duration = 6;
        af.modifier = 1;
        af.location = APPLY_HITROLL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.location = APPLY_SAVING_SPELL;
        af.modifier = -1;
        affect_to_char(victim, &af);
    }
}

void spell_blindness(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (saves_spell(victim, SAVING_SPELL) ||
        affected_by_spell(victim, SPELL_BLINDNESS)) {
        return;
    }
    act("$n seems to be blinded!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You have been blinded!\n\r", victim);

    af.type = SPELL_BLINDNESS;
    af.location = APPLY_HITROLL;
    af.modifier = -4;
    af.duration = level / 2;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +20;
    affect_to_char(victim, &af);

    if ((!victim->specials.fighting) && (victim != ch)) {
        set_fighting(victim, ch);
    }
}


void spell_create_food(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);

    tmp_obj->name = (char *) strdup("mushroom");
    tmp_obj->short_description = (char *) strdup("A Magic Mushroom");
    tmp_obj->description = (char *)strdup("A really delicious looking magic"
                                          " mushroom lies here.");

    tmp_obj->obj_flags.type_flag = ITEM_FOOD;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->obj_flags.value[0] = 5 + level;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;
    tmp_obj->obj_flags.cost_per_day = 1;

    tmp_obj->next = object_list;
    object_list = tmp_obj;

    obj_to_room(tmp_obj, ch->in_room);

    tmp_obj->item_number = -1;

    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void spell_create_water(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             water;

    extern struct weather_data weather_info;
    void            name_to_drinkcon(struct obj_data *obj, int type);
    void            name_from_drinkcon(struct obj_data *obj);

    assert(ch && obj);

    if (GET_ITEM_TYPE(obj) == ITEM_DRINKCON) {
        if (obj->obj_flags.value[2] != LIQ_WATER &&
            obj->obj_flags.value[1] != 0) {

            name_from_drinkcon(obj);
            obj->obj_flags.value[2] = LIQ_SLIME;
            name_to_drinkcon(obj, LIQ_SLIME);
        } else {
            water = 2 * level * ((weather_info.sky >= SKY_RAINING) ? 2 : 1);

            /*
             * Calculate water it can contain, or water created
             */
            water = MIN(obj->obj_flags.value[0] - obj->obj_flags.value[1],
                        water);

            if (water > 0) {
                obj->obj_flags.value[2] = LIQ_WATER;
                obj->obj_flags.value[1] += water;

                weight_change_object(obj, water);

                name_from_drinkcon(obj);
                name_to_drinkcon(obj, LIQ_WATER);
                act("$p is partially filled.", FALSE, ch, obj, 0, TO_CHAR);
            }
        }
    }
}

void spell_cure_blind(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    if (!victim || level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (IS_AFFECTED(victim, AFF_BLIND)) {
        REMOVE_BIT(victim->specials.affected_by, AFF_BLIND);
        send_to_char("Your vision returns!\n\r", victim);
    }

    if (affected_by_spell(victim, SPELL_BLINDNESS)) {
        affect_from_char(victim, SPELL_BLINDNESS);
        send_to_char("Your vision returns!\n\r", victim);
    }

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) += 1;
    }
}

void spell_cure_critic(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    healpoints = dice(3, 8) + 3;

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);
#if 0
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=3;
    }
#endif
}

void spell_cure_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    healpoints = dice(1, 8);

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You feel better!\n\r", victim);

    update_pos(victim);
#if 0
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch)+=1;
    }
#endif
}

void spell_curse(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (obj) {
        SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD);
        SET_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);

        /*
         * LOWER ATTACK DICE BY -1
         */
        if (obj->obj_flags.type_flag == ITEM_WEAPON) {
            obj->obj_flags.value[2]--;
        }
        act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
    } else {
        if (saves_spell(victim, SAVING_SPELL) ||
            affected_by_spell(victim, SPELL_CURSE)) {
            return;
        }
        af.type = SPELL_CURSE;
        af.duration = 24 * 7;
        af.modifier = -1;
        af.location = APPLY_HITROLL;
        af.bitvector = AFF_CURSE;
        affect_to_char(victim, &af);

        af.location = APPLY_SAVING_PARA;
        af.modifier = 1;
        affect_to_char(victim, &af);

        act("$n briefly reveals a red aura!", FALSE, victim, 0, 0, TO_ROOM);
        act("You feel very uncomfortable.", FALSE, victim, 0, 0, TO_CHAR);
        if (IS_NPC(victim) && !victim->specials.fighting) {
            set_fighting(victim, ch);
        }
        if (IS_PC(ch) && IS_PC(victim)) {
            GET_ALIGNMENT(ch) -= 2;
        }
    }
}

void spell_detect_evil(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_EVIL)) {
        return;
    }
    af.type = SPELL_DETECT_EVIL;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_EVIL;

    affect_to_char(victim, &af);

    act("$n's eyes briefly glow red.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_good(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_GOOD)) {
        return;
    }
    af.type = SPELL_DETECT_GOOD;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_DETECT_GOOD;

    affect_to_char(victim, &af);

    act("$n's eyes briefly glow white.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_invisibility(int level, struct char_data *ch,
                               struct char_data *victim,
                               struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_INVISIBLE)) {
        return;
    }
    af.type = SPELL_DETECT_INVISIBLE;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_INVISIBLE;

    affect_to_char(victim, &af);
    act("$n's eyes briefly glow yellow", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("Your eyes tingle.\n\r", victim);
}

void spell_detect_magic(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (affected_by_spell(victim, SPELL_DETECT_MAGIC)) {
        return;
    }
    af.type = SPELL_DETECT_MAGIC;
    af.duration = level * 5;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_DETECT_MAGIC;

    affect_to_char(victim, &af);
    send_to_char("Your eyes tingle.\n\r", victim);
}

#if 1
void spell_detect_poison(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if (victim == ch) {
            if (IS_AFFECTED(victim, AFF_POISON)) {
                send_to_char("You can sense poison in your blood.\n\r", ch);
            } else {
                send_to_char("You feel healthy.\n\r", ch);
            }
        } else if (IS_AFFECTED(victim, AFF_POISON)) {
            act("You sense that $E is poisoned.", FALSE, ch, 0, victim,
                TO_CHAR);
        } else {
            act("You don't find any poisons in $S blood.", FALSE, ch, 0,
                victim, TO_CHAR);
        }
    } else {
        /*
         * It's an object
         */
        if (obj->obj_flags.type_flag == ITEM_DRINKCON ||
            obj->obj_flags.type_flag == ITEM_FOOD) {
            if (obj->obj_flags.value[3]) {
                act("Poisonous fumes are revealed.", FALSE, ch, 0, 0, TO_CHAR);
            } else {
                send_to_char("It looks very delicious.\n\r", ch);
            }
        }
    }
}
#else
void spell_detect_poison(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if (victim == ch) {
            if (IS_AFFECTED(victim, AFF_POISON)) {
                send_to_char("You can sense poison in your blood.\n\r", ch);
            } else {
                send_to_char("You cannot sense any poisons in your blood.\n\r",
                             ch);
            }
        } else if (IS_AFFECTED(victim, AFF_POISON)) {
            act("You sense that $E is poisoned.", FALSE, ch, 0, victim,
                TO_CHAR);
        } else {
            act("You don't find any poisons in $E's blood.", FALSE, ch, 0,
                victim, TO_CHAR);
        }
    } else {
        /* It's an object */
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            if (obj->obj_flags.value[3]) {
                act("Poisonous fumes are revealed.", FALSE, ch, 0, 0, TO_CHAR);
            } else {
                send_to_char("You cannot find any poisons in it.\n\r", ch);
            }
        }
    }
}
#endif

void spell_enchant_weapon(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             i;
    int             count = 0;

    if(!ch || !obj || MAX_OBJ_AFFECT < 2) {
        return;
    }

    if (GET_ITEM_TYPE(obj) == ITEM_WEAPON &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location == APPLY_NONE) {
                count++;
            }
            if (obj->affected[i].location == APPLY_HITNDAM ||
                obj->affected[i].location == APPLY_HITROLL ||
                obj->affected[i].location == APPLY_DAMROLL) {
                return;
            }
        }

        if (count < 2) {
            return;
        }
        /*
         * find the slots
         */
        i = getFreeAffSlot(obj);

        SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

        obj->affected[i].location = APPLY_HITROLL;
        obj->affected[i].modifier = 1;
        if (level > 20) {
            obj->affected[i].modifier += 1;
        }
        if (level > 40) {
            obj->affected[i].modifier += 1;
        }
        if (level > MAX_MORT) {
            obj->affected[i].modifier += 1;
        }
        if (level == BIG_GUY) {
            obj->affected[i].modifier += 1;
        }
        i = getFreeAffSlot(obj);

        obj->affected[i].location = APPLY_DAMROLL;
        obj->affected[i].modifier = 1;
        if (level > 15) {
            obj->affected[i].modifier += 1;
        }
        if (level > 30) {
            obj->affected[i].modifier += 1;
        }
        if (level > MAX_MORT) {
            obj->affected[i].modifier += 1;
        }
        if (level == BIG_GUY) {
            obj->affected[i].modifier += 1;
        }
        if (IS_GOOD(ch)) {
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_EVIL | ITEM_ANTI_NEUTRAL);
            act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
            act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
        } else {
            act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
        }
    }
}

void spell_heal(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    assert(victim);

    spell_cure_blind(level, ch, victim, obj);

    GET_HIT(victim) += 100;

    if (GET_HIT(victim) >= hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim) - dice(1, 4);
    }
    send_to_char("A warm feeling fills your body.\n\r", victim);

    if (IS_AFFECTED2(victim, AFF2_WINGSBURNED)) {
        affect_from_char(victim, COND_WINGS_BURNED);
        send_to_char("Your wings knit together and begin to sprout new "
                     "feathers!\n\r", victim);
    }

    update_pos(victim);

    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) += 5;
    }
}

void spell_invisibility(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert((ch && obj) || victim);

    if (obj) {
        if (!IS_SET(obj->obj_flags.extra_flags, ITEM_INVISIBLE)) {
            act("$p turns invisible.", FALSE, ch, obj, 0, TO_CHAR);
            act("$p turns invisible.", TRUE, ch, obj, 0, TO_ROOM);
            SET_BIT(obj->obj_flags.extra_flags, ITEM_INVISIBLE);
        }
    } else {
        /*
         * Then it is a PC | NPC
         */
        if (!affected_by_spell(victim, SPELL_INVISIBLE)) {
            act("$n slowly fades out of existence.", TRUE, victim, 0, 0,
                TO_ROOM);
            send_to_char("You vanish.\n\r", victim);

            af.type = SPELL_INVISIBLE;
            af.duration = 24;
            af.modifier = -40;
            af.location = APPLY_AC;
            af.bitvector = AFF_INVISIBLE;
            affect_to_char(victim, &af);
        }
    }
}

#if 0
void spell_locate_object(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *i;
    char            name[256],
                    buf2[256];
    char            buf[MAX_STRING_LENGTH];
    int             j;

    assert(ch);

    if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
        send_to_char
            ("Some powerful magic interference provide you from finding this object\n",
             ch);
        return;
    }

    if (!obj) {
        send_to_char("Everywhere, you sense them everywhere!??\n\r", ch);
        return;
    }

    if (!obj->name || !(*obj->name)) {
        send_to_char("Which object?\n\r", ch);
        return;
    }

    strcpy(name, obj->name);

    j = level >> 1;
    sprintf(buf, "");
    for (i = object_list; i && (j > 0); i = i->next)
        if (isname(name, i->name)) {
            if (i->carried_by) {
                if (strlen(PERS_LOC(i->carried_by, ch)) > 0
#if ZONE_LOCATE
                    && (real_roomp(ch->in_room)->zone ==
                        real_roomp(i->in_room)->zone)
#endif
                    ) {
                    sprintf(buf2, "%s carried by %s.\n\r",
                            i->short_description, PERS_LOC(i->carried_by,
                                                           ch));
                    strcat(buf, buf2);
                }
            } else if (i->equipped_by) {
                if (strlen(PERS_LOC(i->equipped_by, ch)) > 0
#if ZONE_LOCATE
                    && (real_roomp(ch->in_room)->zone ==
                        real_roomp(i->in_room)->zone)
#endif
                    ) {
                    sprintf(buf2, "%s equipped by %s.\n\r",
                            i->short_description, PERS_LOC(i->equipped_by,
                                                           ch));
                    strcat(buf, buf2);
                }
            } else if (i->in_obj
#if ZONE_LOCATE
                       && (real_roomp(ch->in_room)->zone ==
                           real_roomp(i->in_room)->zone)
#endif
                ) {
                sprintf(buf2, "%s in %s.\n\r", i->short_description,
                        i->in_obj->short_description);
                strcat(buf, buf2);
            } else
#if ZONE_LOCATE
            if (IS_IMMORTAL(ch) ||
                    real_roomp(ch->in_room)->zone ==
                    real_roomp(i->in_room)->zone) {
#endif
                sprintf(buf2, "%s in %s.\n\r", i->short_description,
                        (i->in_room ==
                         NOWHERE ? "use but uncertain." : real_roomp(i->
                                                                     in_room)->
                         name));
                strcat(buf, buf2);
                j--;
#if ZONE_LOCATE
            }
#endif

        }

    page_string(ch->desc, buf, 0);

    if (j == 0)
        send_to_char("You are very confused.\n\r", ch);
    if (j == level >> 1)
        send_to_char("No such object.\n\r", ch);
}
#else
void spell_locate_object(int level, struct char_data *ch,
                         struct char_data *victim, char *arg)
{
    struct obj_data *i;
    char            name[256];
    char            buf[MAX_STRING_LENGTH],
                    buf2[256];
    int             j,
                    found = 0;
    struct char_data *target;

    assert(ch);
    sprintf(name, "%s", arg);

    /*
     * when starting out, no object has been found yet
     */
    found = 0;

    if (IS_SET(SystemFlags, SYS_LOCOBJ)) {
        send_to_char("Some powerful magic interference provide you from "
                     "finding this object\n", ch);
        return;
    }

    j = level >> 2;
    if (j < 2) {
        j = 2;
    }
    buf[0] = '\0';

    for (i = object_list; i && (j > 0); i = i->next) {
        if (isname(name, i->name) &&
            !IS_SET(i->obj_flags.extra_flags, ITEM_QUEST)) {
            /*
             * we found at least one item
             */
            if (i->carried_by) {
                target = i->carried_by;
                if (((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                      real_roomp(ch->in_room)->zone ==
                       real_roomp(target->in_room)->zone) ||
                      (!IS_SET(SystemFlags, SYS_ZONELOCATE))) &&
                    !IS_IMMORTAL(target) &&
                    strlen(PERS_LOC(i->carried_by, ch)) > 0) {

                    found = 1;
                    sprintf(buf2, "%s carried by %s.\n\r",
                            i->short_description, PERS(i->carried_by, ch));
                    strcat(buf, buf2);
                    j--;
                }
            } else if (i->equipped_by) {
                target = i->equipped_by;
                if (((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                      real_roomp(ch->in_room)->zone ==
                      real_roomp(target->in_room)->zone) ||
                     (!IS_SET(SystemFlags, SYS_ZONELOCATE))) &&
                    !IS_IMMORTAL(target) &&
                    strlen(PERS_LOC(i->equipped_by, ch)) > 0) {

                    found = 1;
                    sprintf(buf2, "%s equipped by %s.\n\r",
                            i->short_description, PERS(i->equipped_by, ch));
                    strcat(buf, buf2);
                    j--;
                }
            } else if (i->in_obj) {
                if ((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                     real_roomp(ch->in_room)->zone ==
                     real_roomp(i->in_obj->in_room)->zone) ||
                    (!IS_SET(SystemFlags, SYS_ZONELOCATE))) {

                    found = 1;
                    sprintf(buf2, "%s in %s.\n\r",
                            i->short_description, i->in_obj->short_description);
                    strcat(buf, buf2);
                    j--;
                }
            } else if ((IS_SET(SystemFlags, SYS_ZONELOCATE) &&
                        real_roomp(ch->in_room)->zone ==
                        real_roomp(i->in_room)->zone) ||
                       (!IS_SET(SystemFlags, SYS_ZONELOCATE))) {
                found = 1;
                sprintf(buf2, "%s in %s.\n\r",
                        i->short_description,
                        (i->in_room == NOWHERE ? "use but uncertain." :
                         real_roomp(i->in_room)->name));
                strcat(buf, buf2);
                j--;
            }
        }
    }
    page_string(ch->desc, buf, 0);

    if (j == 0) {
        send_to_char("You are very confused.\n\r", ch);
    }
    if (found == 0) {
        send_to_char("Nothing at all by that name.\n\r", ch);
    }
}

#endif

void spell_poison(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim || obj);

    if (victim) {
        if (IS_SET(victim->M_immune, IMM_POISON)) {
            send_to_char("Forces unknown help you resist the poison!\n\r",
                         victim);
            return;
        }

        if (IS_NPC(ch)) {
            if (!IS_SET(ch->specials.act, ACT_DEADLY)) {
                if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
                    af.type = SPELL_POISON;
                    af.duration = level * 2;
                    af.modifier = -2;
                    af.location = APPLY_STR;
                    af.bitvector = AFF_POISON;
                    affect_join(victim, &af, FALSE, FALSE);
                    send_to_char("You feel very sick.\n\r", victim);
                    if (!victim->specials.fighting) {
                        AddHated(victim, ch);
                        set_fighting(victim, ch);
                    }
                } else {
                    FailPoison(victim, ch);
                    return;
                }
            } else {
                if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
                    act("Deadly poison fills your veins.", TRUE, ch, 0, 0,
                        TO_CHAR);
                    if (!damage(victim, victim, MAX(10, GET_HIT(victim) * 2),
                                SPELL_POISON)) {
                        AddHated(victim, ch);
                    }
                } else {
                    FailPoison(victim, ch);
                    return;
                }
            }
        } else if (!ImpSaveSpell(victim, SAVING_PARA, 0)) {
            af.type = SPELL_POISON;
            af.duration = level * 2;
            af.modifier = -2;
            af.location = APPLY_STR;
            af.bitvector = AFF_POISON;
            affect_join(victim, &af, FALSE, FALSE);

            act("$N looks very sick as you poison $M.", TRUE, ch, 0,
                victim, TO_CHAR);
            act("$N looks very sick as $n poisons $M.", TRUE, ch, 0,
                victim, TO_NOTVICT);
            send_to_char("You feel very sick suddenly.\n\r", victim);
            if (!victim->specials.fighting && !IS_PC(victim)) {
                AddHated(victim, ch);
                set_fighting(victim, ch);
            }
        } else {
            FailPoison(victim, ch);
        }
    } else if (obj->obj_flags.type_flag == ITEM_DRINKCON ||
               obj->obj_flags.type_flag == ITEM_FOOD) {
        /*
         * Object poison
         */
        obj->obj_flags.value[3] = 1;
    }
}

void spell_protection_from_evil(int level, struct char_data *ch,
                                struct char_data *victim,
                                struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_EVIL)) {
        af.type = SPELL_PROTECT_FROM_EVIL;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_PROTECT_FROM_EVIL;
        affect_to_char(victim, &af);
        send_to_char("You have a righteous feeling!\n\r", victim);
    }
}

void spell_protection_from_evil_group(int level, struct char_data *ch,
                                      struct char_data *victim,
                                      struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *dude;

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (IS_FOLLOWING(ch, dude) &&
            !affected_by_spell(dude, SPELL_PROTECT_FROM_EVIL)) {

            af.type = SPELL_PROTECT_FROM_EVIL;
            af.duration = 24;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PROTECT_FROM_EVIL;
            affect_to_char(dude, &af);
            send_to_char("You have a righteous feeling!\n\r", dude);
        }
    }
}

void spell_protection_from_good(int level, struct char_data *ch,
                                struct char_data *victim,
                                struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_PROTECT_FROM_GOOD)) {
        af.type = SPELL_PROTECT_FROM_GOOD;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_SPELL2;
        af.bitvector = AFF2_PROTECT_FROM_GOOD;
        affect_to_char(victim, &af);
        send_to_char("You have a sinister feeling!\n\r", victim);
    }
}

void spell_protection_from_good_group(int level, struct char_data *ch,
                                      struct char_data *victim,
                                      struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *dude;

    for (dude = real_roomp(ch->in_room)->people; dude;
         dude = dude->next_in_room) {
        if (IS_FOLLOWING(ch, dude) &&
            !affected_by_spell(dude, SPELL_PROTECT_FROM_GOOD)) {

            af.type = SPELL_PROTECT_FROM_GOOD;
            af.duration = 24;
            af.modifier = 0;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_PROTECT_FROM_GOOD;
            affect_to_char(dude, &af);
            send_to_char("You have a sinister feeling!\n\r", dude);
        }
    }
}

void spell_remove_curse(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             i,
                    found = FALSE;

    assert(ch && (victim || obj));

    if (obj) {
        if (IS_SET(obj->obj_flags.extra_flags, ITEM_NODROP)) {
            act("$p briefly glows blue.", TRUE, ch, obj, 0, TO_CHAR);
            act("$p, held by $n, briefly glows blue.", TRUE, ch, obj, 0,
                TO_ROOM);
            REMOVE_BIT(obj->obj_flags.extra_flags, ITEM_NODROP);
        }
    } else {
        /*
         * Then it is a PC | NPC
         */
        if (affected_by_spell(victim, SPELL_CURSE)) {
            act("$n briefly glows red, then blue.", FALSE, victim, 0, 0,
                TO_ROOM);
            act("You feel better.", FALSE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_CURSE);
        } else {
            /*
             * uncurse items equiped by a person
             */
            i = 0;
            do {
                if (victim->equipment[i] &&
                    IS_SET(victim->equipment[i]->obj_flags.extra_flags,
                           ITEM_NODROP)) {
                    spell_remove_curse(level, victim, NULL,
                                       victim->equipment[i]);
                    found = TRUE;
                }
                i++;
            } while (i < MAX_WEAR && !found);
        }

        if (IS_PC(ch) && IS_PC(victim)) {
            GET_ALIGNMENT(ch) += 2;
        }
    }
}

void spell_remove_poison(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (victim) {
        if (affected_by_spell(victim, SPELL_POISON)) {
            affect_from_char(victim, SPELL_POISON);
            act("A warm feeling runs through your body.", FALSE, victim, 0,
                0, TO_CHAR);
            act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
        }
    } else {
        if ((obj->obj_flags.type_flag == ITEM_DRINKCON) ||
            (obj->obj_flags.type_flag == ITEM_FOOD)) {
            obj->obj_flags.value[3] = 0;
            act("The $p steams briefly.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}

void spell_fireshield(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_FIRESHIELD)) {
        if (affected_by_spell(victim, SPELL_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, victim,
                0, 0, TO_ROOM);
            act("The heat of your spell melts the icey aura surrounding you.",
                TRUE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_CHILLSHIELD);
        }
        if (IS_AFFECTED(victim, AFF_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, victim,
                0, 0, TO_ROOM);
            act("The heat of your spell melts the icey aura surrounding you.",
                TRUE, victim, 0, 0, TO_CHAR);
            REMOVE_BIT(victim->specials.affected_by, AFF_CHILLSHIELD);
        }
        if (affected_by_spell(victim, SPELL_BLADE_BARRIER)) {
            act("The whirling blades around $n burn to cinders.", TRUE,
                victim, 0, 0, TO_ROOM);
            act("The heat of your spell burns your blade barrier to cinders.",
                TRUE, victim, 0, 0, TO_CHAR);
            affect_from_char(victim, SPELL_BLADE_BARRIER);
        }
        if (IS_AFFECTED(victim, AFF_BLADE_BARRIER)) {
            act("The whirling blades around $n burn to cinders.", TRUE,
                victim, 0, 0, TO_ROOM);
            act("The heat of your spell burns your blade barrier to cinders.",
                TRUE, victim, 0, 0, TO_CHAR);
            REMOVE_BIT(victim->specials.affected_by, AFF_BLADE_BARRIER);
        }

        act("$c000R$n is surrounded by a glowing red aura.", TRUE, victim,
            0, 0, TO_ROOM);
        act("$c000RYou start glowing red.", TRUE, victim, 0, 0, TO_CHAR);

        af.type = SPELL_FIRESHIELD;
        af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_FIRESHIELD;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", victim);
    }
}

void spell_sanctuary(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_SANCTUARY) &&
        !IS_AFFECTED(victim, AFF_SANCTUARY)) {
        act("$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM);
        act("You start glowing.", TRUE, victim, 0, 0, TO_CHAR);

        af.type = SPELL_SANCTUARY;

        if (ch->specials.remortclass != CLERIC_LEVEL_IND + 1) {
            af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        } else {
            af.duration = (level < LOW_IMMORTAL) ? 4 : level;
        }
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(victim, &af);
    }
}

void spell_sleep(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    assert(victim);

    if (IsImmune(victim, IMM_SLEEP)) {
        FailSleep(victim, ch);
        return;
    }
    if (IsResist(victim, IMM_SLEEP)) {
        if (saves_spell(victim, SAVING_SPELL)) {
            FailSleep(victim, ch);
            return;
        }
        if (saves_spell(victim, SAVING_SPELL)) {
            FailSleep(victim, ch);
            return;
        }
    } else if (!IsSusc(victim, IMM_SLEEP) &&
               saves_spell(victim, SAVING_SPELL)) {
        FailSleep(victim, ch);
        return;
    }

    af.type = SPELL_SLEEP;
    af.duration = 4 + level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_SLEEP;
    affect_join(victim, &af, FALSE, FALSE);

    if (GET_POS(victim) > POSITION_SLEEPING) {
        act("You feel very sleepy ..... zzzzzz", FALSE, victim, 0, 0, TO_CHAR);
        act("$n goes to sleep.", TRUE, victim, 0, 0, TO_ROOM);
        GET_POS(victim) = POSITION_SLEEPING;
    }

    /*
     * make fighting between ch and victim stop, so ws:sleep items have a use
     */
    if (victim->specials.fighting) {
        stop_fighting(victim);
    }
    if (ch->specials.fighting == victim) {
        stop_fighting(ch);
    }
}

void spell_strength(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_STRENGTH)) {
        act("You feel stronger.", FALSE, victim, 0, 0, TO_CHAR);
        act("$n seems stronger!", FALSE, victim, 0, 0, TO_ROOM);
        af.type = SPELL_STRENGTH;
        af.duration = 2 * level;
        if (IS_NPC(victim)) {
            if (level >= CREATOR) {
                af.modifier = 25 - GET_STR(victim);
            } else {
                af.modifier = number(1, 6);
            }
        } else if (HasClass(ch, CLASS_WARRIOR) ||
                   HasClass(ch, CLASS_BARBARIAN)) {
            af.modifier = number(1, 8);
        } else if (HasClass(ch, CLASS_CLERIC) || HasClass(ch, CLASS_THIEF)) {
            af.modifier = number(1, 6);
        } else {
                af.modifier = number(1, 4);
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void spell_word_of_recall(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             location;

    void            do_look(struct char_data *ch, char *argument, int cmd);

    assert(victim);

    if (IS_NPC(victim)) {
        return;
    }
#if 0
    loc_nr = GET_HOME(ch);
#endif

    if (victim->player.hometown) {
        location = victim->player.hometown;
    } else {
        location = 3001;
    }

    if (!real_roomp(location)) {
        send_to_char("You are completely lost.\n\r", victim);
        location = 0;
        return;
    }

    if (victim->specials.fighting) {
        send_to_char("HAH, not in a fight!\n\r", ch);
        return;
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("You can't recall!, you're on a different plane!\n\r",
                     victim);
        return;
    }

    /*
     * a location has been found.
     */
    send_to_char("You think of better times and close your eyes.\n\r", victim);

    act("$n disappears.", TRUE, victim, 0, 0, TO_ROOM);
    char_from_room(victim);
    char_to_room(victim, location);
    act("$n appears in the middle of the room.", TRUE, victim, 0, 0, TO_ROOM);
    do_look(victim, "", 15);

}

void spell_summon(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp;
    struct room_data *rp;
    int             count;

    assert(ch && victim);

    if (victim->in_room <= NOWHERE) {
        send_to_char("Couldn't find any of those.\n\r", ch);
        return;
    }

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, NO_MAGIC)) {
        send_to_char("Eldritch wizardry obstructs thee.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There is no room in here to summon!\n\r", ch);
        return;
    }

    if (rp->sector_type == SECT_AIR || rp->sector_type == SECT_UNDERWATER) {
        send_to_char("Strange powers block your summons\n", ch);
        return;
    }

    if (check_peaceful(ch, "Ancient powers obstruct thy magik.\n"))
        return;

    if (check_peaceful(victim, "")) {
        send_to_char("You cannot get past the magical defenses.\n\r", ch);
        return;
    }

    if (IS_PC(victim) && IS_LINKDEAD(victim)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(victim->in_room)->room_flags, NO_SUM)) {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) > LOW_IMMORTAL) {
        send_to_char("A large hand suddenly appears before you and thumps "
                     "your head!\n\r", ch);
        return;
    }

    if (victim->specials.fighting) {
        send_to_char("You can't get a clear fix on them\n", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOSUMMON)) {
        send_to_char("A mystical fog blocks your attemps!\n", ch);
        return;
    }

    if (!IsOnPmp(victim->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n", ch);
        return;
    }

    if (CanFightEachOther(ch, victim) && saves_spell(victim, SAVING_SPELL)) {
        act("You failed to summon $N!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n tried to summon you!", FALSE, ch, 0, victim, TO_VICT);
        return;
    }

    if (!IS_PC(victim)) {
        count = 0;
        for (tmp = real_roomp(victim->in_room)->people;
             tmp; tmp = tmp->next_in_room) {
            count++;
        }

        if (count == 0) {
            send_to_char("You failed.\n\r", ch);
            return;
        } else {
            count = number(0, count);

            for (tmp = real_roomp(victim->in_room)->people;
                 count && tmp; tmp = tmp->next_in_room, count--) {
                /*
                 * Empty loop
                 */
            }

            if (tmp && GET_MAX_HIT(tmp) < GET_HIT(ch) &&
                !saves_spell(tmp, SAVING_SPELL)) {
                RawSummon(tmp, ch);
            } else {
                send_to_char("You failed\n\r", ch);
                return;
            }
        }
    } else {
        RawSummon(victim, ch);
    }
}

void RawSummon(struct char_data *v, struct char_data *c)
{
    long            target;
    struct obj_data *o,
                   *n;
    int             j;
    extern char     EasySummon;
    char            buf[400];

    /*
     * this section run if the mob is above 3 levels above the caster
     * destroys the mobs EQ
     */

    if (IS_NPC(v) && !IS_SET(v->specials.act, ACT_POLYSELF) &&
        GetMaxLevel(v) > GetMaxLevel(c) + 3) {
        act("$N struggles, and all of $S items are destroyed!", TRUE, c, 0,
            v, TO_CHAR);
        /*
         * remove objects from victim
         */
        for (j = 0; j < MAX_WEAR; j++) {
            if (v->equipment[j]) {
                o = unequip_char(v, j);
                extract_obj(o);
            }
        }

        for (o = v->carrying; o; o = n) {
            n = o->next_content;
            obj_from_char(o);
            extract_obj(o);
        }
        AddHated(v, c);
    } else if (!EasySummon) {
        send_to_char("A wave of nausea overcomes you.  You collapse!\n\r", c);
        WAIT_STATE(c, PULSE_VIOLENCE * 6);
        GET_POS(c) = POSITION_STUNNED;
    }

    act("$n disappears suddenly.", TRUE, v, 0, 0, TO_ROOM);
    target = c->in_room;
    char_from_room(v);
    char_to_room(v, target);

    act("$n arrives suddenly.", TRUE, v, 0, 0, TO_ROOM);

    sprintf(buf, "%s has summoned you!\n\r",
            (IS_NPC(c) ? c->player.short_descr : GET_NAME(c)));
    send_to_char(buf, v);
    do_look(v, "", 15);

    if (IS_NPC(v) && !IS_SET(v->specials.act, ACT_POLYSELF) &&
        (IS_SET(v->specials.act, ACT_AGGRESSIVE) ||
         IS_SET(v->specials.act, ACT_META_AGG)) && CAN_SEE(v, c)) {
        act("$n growls at you", 1, v, 0, c, TO_VICT);
        act("$n growls at $N", 1, v, 0, c, TO_NOTVICT);
        hit(v, c, TYPE_UNDEFINED);
    }
}

void spell_charm_person(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r",
                         ch);
            return;
        }

        if (!IsPerson(victim)) {
            send_to_char("Umm,  that's not a person....\n\r", ch);
            return;
        }

        if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
            FailCharm(victim, ch);
            return;
        }

        if (too_many_followers(ch)) {
            act("$N takes one look at the size of your posse and just says no!",
                TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
            act("$N takes one look at the size of $n's posse and just says no!",
                TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
            return;
        }

        if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
            FailCharm(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_CHARM)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }

            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }
        } else if (!IsSusc(victim, IMM_CHARM) &&
                   saves_spell(victim, SAVING_PARA)) {
            FailCharm(victim, ch);
            return;
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;

        if (GET_CHR(ch)) {
            af.duration = follow_time(ch);
        } else {
            af.duration = 24 * 18;
        }
        if (IS_GOOD(victim) && IS_GOOD(ch)) {
            af.duration *= 2;
        }
        if (IS_EVIL(victim) && IS_EVIL(ch)) {
            af.duration += af.duration >> 1;
        }
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);

        if (!IS_PC(ch)) {
            REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
            SET_BIT(victim->specials.act, ACT_SENTINEL);
        }
    }
}

void spell_charm_monster(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (IsVeggie(victim)) {
        send_to_char("You can't charm a plant-creature!\n\r", ch);
        return;
    }

    if (IsPerson(victim)) {
        send_to_char("You can't charm people! Try charm person for that!\n\r",
                     ch);
        return;
    }

    if (GetMaxLevel(victim) > GetMaxLevel(ch) + 3) {
        FailCharm(victim, ch);
        return;
    }

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, 0, victim, TO_ROOM);
        return;
    }

    if (!IS_AFFECTED(victim, AFF_CHARM) && !IS_AFFECTED(ch, AFF_CHARM)) {
        if (circle_follow(victim, ch)) {
            send_to_char("Sorry, following in circles can not be allowed.\n\r",
                         ch);
            return;
        }

        if (IsImmune(victim, IMM_CHARM) || (WeaponImmune(victim))) {
            FailCharm(victim, ch);
            return;
        }

        if (IsResist(victim, IMM_CHARM)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }

            if (saves_spell(victim, SAVING_PARA)) {
                FailCharm(victim, ch);
                return;
            }
        } else if (!IsSusc(victim, IMM_CHARM) &&
                   saves_spell(victim, SAVING_PARA)) {
            FailCharm(victim, ch);
            return;
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);

        af.type = SPELL_CHARM_PERSON;

        if (GET_CHR(ch)) {
            af.duration = follow_time(ch);
        } else {
            af.duration = 24 * 18;
        }
        if (IS_GOOD(victim) && IS_GOOD(ch)) {
            af.duration *= 2;
        }
        if (IS_EVIL(victim) && IS_EVIL(ch)) {
            af.duration += af.duration >> 1;
        }
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);

        if (!IS_PC(ch)) {
            REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
            SET_BIT(victim->specials.act, ACT_SENTINEL);
        }
    }
}

void spell_sense_life(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_SENSE_LIFE) &&
        !IS_AFFECTED(victim, AFF_SENSE_LIFE)) {
        if (ch != victim) {
            act("$n waves $s hands around $N's eyes.", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You help $N see hidden life forms.", FALSE, ch, 0, victim,
                TO_CHAR);
            act("$n waves $s hands around your eyes, you feel your senses "
                "improve.", FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n eyes flash briefly.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You feel your senses improve.", FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_SENSE_LIFE;
        af.duration = 5 * level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SENSE_LIFE;
        affect_to_char(victim, &af);
    } else {
        act("$N can already sense hidden life forms.", FALSE, ch, 0,
            victim, TO_CHAR);
    }
}

void spell_globe_minor_inv(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_GLOBE_MINOR_INV)) {
        if (ch != victim) {
            act("$n summons a small globe of protection about $N", FALSE,
                ch, 0, victim, TO_NOTVICT);
            act("You summon a small globe of protection about $N", FALSE,
                ch, 0, victim, TO_CHAR);
            act("$n summons a small globe of protection about you", FALSE,
                ch, 0, victim, TO_VICT);
        } else {
            act("$n summons a small globe of protection about $mself",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You summon a small globe of protection about yourself",
                FALSE, ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_GLOBE_MINOR_INV;
        af.duration = (level < LOW_IMMORTAL) ? level / 10 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        send_to_char("As you cast the spell you realize that they are "
                     "already protected.\n\r", ch);
    } else {
        send_to_char("As you cast the spell you remember that you are "
                     "already protected.\n\r", ch);
    }
}

void spell_globe_major_inv(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_GLOBE_MAJOR_INV)) {
        if (ch != victim) {
            act("$n summons a globe of protection about $N", FALSE, ch, 0,
                victim, TO_NOTVICT);
            act("You summon a globe of protection about $N", FALSE, ch, 0,
                victim, TO_CHAR);
            act("$n summons a globe of protection about you", FALSE, ch, 0,
                victim, TO_VICT);
        } else {
            act("$n summons a globe of protection about $mself", FALSE, ch,
                0, victim, TO_NOTVICT);
            act("You summon a globe of protection about yourself", FALSE,
                ch, 0, victim, TO_CHAR);
        }

        af.type = SPELL_GLOBE_MAJOR_INV;
        af.duration = (level < LOW_IMMORTAL) ? level / 10 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        send_to_char("As you cast the spell you realize that they are "
                     "already protected.\n\r", ch);
    } else {
        send_to_char("As you cast the spell you remember that you are "
                     "already protected.\n\r", ch);
    }
}

void spell_anti_magic_shell(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_ANTI_MAGIC_SHELL)) {
        send_to_char("You erect a anti-magic globe about yourself.\n\r", ch);
        spell_dispel_magic(IMPLEMENTOR, ch, ch, 0);

        af.type = SPELL_ANTI_MAGIC_SHELL;
        /*
         * one tic only!
         */
        af.duration = (level < LOW_IMMORTAL) ? 1 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        send_to_char("You are already protected by a anti-magic shell.\n\r",
                     ch);
    }
}

void spell_prismatic_spray(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    struct char_data *tmp_victim;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    /*
     * spell effects:
     * red 1 40 dam
     * orange 2 80
     * yellow 3 100
     * green 4 poison
     * blue 5 petrify
     * indigo 6 feeble
     * violet 7 teleport
     */

    send_to_char
        ("You send a rainbow of prismatic spray from you hands.\n\r", ch);
    act("$n sends a rainbow of prismatic spray from $s hands!", FALSE, ch,
        0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people;
         tmp_victim; tmp_victim = tmp_victim->next_in_room) {
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                switch (number(1, 7)) {
                case 1:
                    dam = 40;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by a red shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with a red shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with a red shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                case 2:
                    dam = 80;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with an orange shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                case 3:
                    dam = 100;
                    if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                        act("$N is hit by a yellow shaft of light!", FALSE,
                            ch, 0, tmp_victim, TO_NOTVICT);
                        act("You hit $N with a yellow shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_CHAR);
                        act("$n hits you with a yellow shaft of light!",
                            FALSE, ch, 0, tmp_victim, TO_VICT);
                        MissileDamage(ch, tmp_victim, dam,
                                      SPELL_PRISMATIC_SPRAY);
                    }
                    break;
                    /*
                     * these have internal saves already
                     */
                case 4:
                    act("$N is hit by a green shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a green shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a yellow shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_poison(level, ch, "", SPELL_TYPE_SPELL,
                                tmp_victim, NULL);
                    break;
                case 5:
                    act("$N is hit by a blue shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a blue shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a blue shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_paralyze(level, ch, "", SPELL_TYPE_SPELL,
                                  tmp_victim, NULL);
                    break;
                case 6:
                    act("$N is hit by an indigo shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with an indigo shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with an indigo shaft of light!",
                        FALSE, ch, 0, tmp_victim, TO_VICT);
                    cast_feeblemind(level, ch, "", SPELL_TYPE_SPELL,
                                    tmp_victim, NULL);
                    break;
                case 7:
                    act("$N is hit by a violet shaft of light!", FALSE, ch,
                        0, tmp_victim, TO_NOTVICT);
                    act("You hit $N with a violet shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_CHAR);
                    act("$n hits you with a violet shaft of light!", FALSE,
                        ch, 0, tmp_victim, TO_VICT);
                    cast_teleport(level, ch, "", SPELL_TYPE_SPELL,
                                  tmp_victim, NULL);
                    break;
                default:
                    /*
                     * should never get here
                     */
                    dam = 100;
                    MissileDamage(ch, tmp_victim, dam, SPELL_PRISMATIC_SPRAY);
                    break;
                }
                if (!tmp_victim->specials.fighting &&
                    ch->specials.fighting != tmp_victim) {
                    set_fighting(ch, tmp_victim);
                }
            } else {
                act("You avoid the colorful spray!", FALSE, ch, 0,
                    tmp_victim, TO_VICT);
            }
            act("$N avoids the colorful spray!", FALSE, ch, 0, tmp_victim,
                TO_NOTVICT);
            act("$N avoids your colorful spray!", FALSE, ch, 0, tmp_victim,
                TO_CHAR);
        }
    }
}

void spell_incendiary_cloud(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    dam = dice(level, 10);

    send_to_char("Billowing clouds of incendiary gases pour from your "
                 "fingertips.\n\r", ch);
    act("Billowing clouds of incendiary gases pour from $n's fingertips!",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = character_list; tmp_victim; tmp_victim = temp) {
        rdam = dam;
        temp = tmp_victim->next;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (!in_group(ch, tmp_victim) && !IS_IMMORTAL(tmp_victim)) {
                if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                    if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                        /*
                         * Fail two saves, wings burn
                         */
                        BurnWings(tmp_victim);
                    }
                    heat_blind(tmp_victim);
                    MissileDamage(ch, tmp_victim, rdam, SPELL_INCENDIARY_CLOUD);
                } else {
                    rdam >>= 1;
                    /*
                     * half dam
                     */
                    heat_blind(tmp_victim);
                    MissileDamage(ch, tmp_victim, rdam, SPELL_INCENDIARY_CLOUD);
                }
            } else {
                act("You avoid the smouldering gases!\n\r", FALSE, ch, 0,
                    tmp_victim, TO_VICT);
            }
        }
    }
}

void spell_comp_languages(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[128];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_COMP_LANGUAGES)) {
        if (ch != victim) {
            act("$n gently touches $N's ears.", FALSE, ch, 0, victim,
                TO_NOTVICT);
            act("You touch $N's ears gently.", FALSE, ch, 0, victim, TO_CHAR);
            act("$n gently touches your ears, wow you have missed so much!",
                FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n touches $s own ears.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You touch your ears, wow you missed so much!", FALSE, ch,
                0, victim, TO_CHAR);
        }
        af.type = SPELL_COMP_LANGUAGES;
        /*
         * one tic only!
         */
        af.duration = (level < LOW_IMMORTAL) ? (int) level / 2 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N can already understand languages.");
        } else {
            sprintf(buf, "You can already understand languages.");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

/*
 ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */


void spell_identify(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    char            buf[256],
                    buf2[256];
    int             i;
    bool            found,
                    auct = FALSE;

    char            color1[10],
                    color2[10];
    struct time_info_data ma;

    assert(ch && (obj || victim));

    if (obj) {
        if (auctioneer && auctioneer->specials.auction &&
            obj == auctioneer->specials.auction) {
            auct = TRUE;
        }

        if (auct) {
            sprintf(color1, "$c000c");
            sprintf(color2, "$c000w");
            send_to_char("$c000cThe item currently on auction has the "
                         "following stats:\n\r", ch);
        } else if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
            sprintf(color1, "$c000p");
            sprintf(color2, "$c000C");
            sprintf(buf, "%sYou feel informed:\n\r", color1);
            send_to_char(buf, ch);
        } else {
            sprintf(color1, "$c000B");
            sprintf(color2, "$c000w");
            sprintf(buf, "%sYou feel informed:\n\r", color1);
            send_to_char(buf, ch);
        }

        sprintf(buf, "%sObject '%s%s%s', Item type: %s", color1, color2,
                obj->name, color1, color2);
        sprinttype(GET_ITEM_TYPE(obj), item_types, buf2);
        strcat(buf, buf2);
        if (IS_WEAPON(obj)) {
            if (IS_IMMORTAL(ch)) {
                sprintf(buf2, "%s, Weapon Speed: %s%s%s (%s%.2f%s)",
                        color1, color2, SpeedDesc(obj->speed), color1,
                        color2, (float) obj->speed / 100, color1);
            } else {
                sprintf(buf2, "%s, Weapon Speed: %s%s", color1, color2,
                        SpeedDesc(obj->speed));
            }
            strcat(buf, buf2);
        }
        strcat(buf, "\n\r");
        send_to_char(buf, ch);

        /*
         * alittle more info for immortals -bcw
         */
        if (GetMaxLevel(ch) > LOW_IMMORTAL) {
            sprintf(buf, "%sR-number: [%s%d%s], V-number: [%s%ld%s]",
                    color1, color2, obj->item_number, color1, color2,
                    (obj->item_number >= 0) ?
                     obj_index[obj->item_number].virtual : 0, color1);

            if (obj->max == 0) {
                sprintf(buf2, "%s", "unlimited");
            } else {
                sprintf(buf2, "%d", obj->max);
            }

            sprintf(buf, "%s %sLoadrate: [%s%s%s], Tweak Rate: [%s%d%s], "
                         "ObjValue[%s%d%s]\n\r",
                    buf, color1, color2, buf2, color1, color2, obj->tweak,
                    color1, color2, eval(obj), color1);
            send_to_char(buf, ch);

            if (obj->level == 0) {
                sprintf(buf2, "%sEgo: %sNone%s, ", color1, color2, color1);
            } else {
                sprintf(buf2, "%sEgo: %sLevel %d%s, ", color1, color2,
                        obj->level, color1);
            }
            send_to_char(buf2, ch);

            sprintf(buf2, "%sLast modified by %s%s%s on %s%s", color1,
                    color2, obj->modBy ? obj->modBy : "unmodified", color1,
                    color2, asctime(localtime(&obj->modified)));
            send_to_char(buf2, ch);
        }

        if (obj->obj_flags.bitvector) {
            sprintf(buf2, "%sItem will give you following abilities:%s  ",
                    color1, color2);
            send_to_char(buf2, ch);

            sprintbit((unsigned) obj->obj_flags.bitvector, affected_bits, buf);
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
        }

        sprintf(buf, "%sItem is:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->obj_flags.extra_flags, extra_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        sprintf(buf, "%sWeight: %s%d%s, Value: %s%d%s, Rent cost: %s%d%s ",
                color1, color2, obj->obj_flags.weight, color1, color2,
                obj->obj_flags.cost, color1, color2,
                obj->obj_flags.cost_per_day, color1);
        send_to_char(buf, ch);

        if (IS_RARE(obj)) {
            sprintf(buf, "%s[%sRARE%s]", color1, color2, color1);
            send_to_char(buf, ch);
        }
        send_to_char("\n\r", ch);

        sprintf(buf, "%sCan be worn on:%s ", color1, color2);
        send_to_char(buf, ch);
        sprintbit((unsigned) obj->obj_flags.wear_flags, wear_bits, buf2);
        strcat(buf2, "\n\r");
        send_to_char(buf2, ch);

        switch (GET_ITEM_TYPE(obj)) {

        case ITEM_SCROLL:
        case ITEM_POTION:
            sprintf(buf, "%sLevel %s%d%s spells of:\n\r", color1, color2,
                    obj->obj_flags.value[0], color1);
            send_to_char(buf, ch);

            for( i = 1; i < 4; i++ ) {
                if (obj->obj_flags.value[i] >= 1) {
                    sprinttype(obj->obj_flags.value[i] - 1, spells, buf);
                    sprintf(buf2, "%s%s", color2, buf);
                    strcat(buf2, "\n\r");
                    send_to_char(buf2, ch);
                }
            }
            break;

        case ITEM_WAND:
        case ITEM_STAFF:
            sprintf(buf, "%sCosts %s%d%s mana to use, with %s%d%s charges "
                         "left.\n\r",
                    color1, color2, obj->obj_flags.value[1], color1,
                    color2, obj->obj_flags.value[2], color1);
            send_to_char(buf, ch);

            sprintf(buf, "%sLevel %s%d%s spell of:\n\r", color1, color2,
                    obj->obj_flags.value[0], color1);
            send_to_char(buf, ch);
            if (obj->obj_flags.value[3] >= 1) {
                sprinttype(obj->obj_flags.value[3] - 1, spells, buf);
                sprintf(buf2, "%s%s", color2, buf);
                strcat(buf2, "\n\r");
                send_to_char(buf2, ch);
            }
            break;

        case ITEM_WEAPON:
            sprintf(buf, "%sDamage Dice is '%s%dD%d%s' [%s%s%s] [%s%s%s]\n\r",
                    color1, color2, obj->obj_flags.value[1],
                    obj->obj_flags.value[2], color1, color2,
                    AttackType[obj->obj_flags.value[3]], color1, color2,
                    weaponskills[obj->weapontype].name, color1);
            send_to_char(buf, ch);
            break;

        case ITEM_ARMOR:
            sprintf(buf, "%sAC-apply is: %s%d%s,   Size of armor is: %s%s\n\r",
                    color1, color2, obj->obj_flags.value[0], color1,
                    color2, ArmorSize(obj->obj_flags.value[2]));
            send_to_char(buf, ch);
            break;
        }

        found = FALSE;

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location != APPLY_NONE &&
                obj->affected[i].modifier != 0) {
                if (!found) {
                    sprintf(buf, "%sCan affect you as:\n\r", color1);
                    send_to_char(buf, ch);
                    found = TRUE;
                }

                sprinttype(obj->affected[i].location, apply_types, buf2);
                sprintf(buf, "%s    Affects: %s%s%s by %s", color1, color2,
                        buf2, color1, color2);
                send_to_char(buf, ch);

                switch (obj->affected[i].location) {
                case APPLY_M_IMMUNE:
                case APPLY_IMMUNE:
                case APPLY_SUSC:
                    sprintbit(obj->affected[i].modifier, immunity_names, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_ATTACKS:
                    sprintf(buf2, "%.2f\n\r", obj->affected[i].modifier / 10.0);
                    break;

                case APPLY_WEAPON_SPELL:
                case APPLY_EAT_SPELL:
                    sprintf(buf2, "%s\n\r",
                            spells[obj->affected[i].modifier - 1]);
                    break;

                case APPLY_SPELL:
                    sprintbit(obj->affected[i].modifier, affected_bits, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_SPELL2:
                    sprintbit(obj->affected[i].modifier, affected_bits2, buf2);
                    strcat(buf2, "\n\r");
                    break;

                case APPLY_RACE_SLAYER:
                    sprintf(buf2, "%s\n\r",
                            RaceName[obj->affected[i].modifier]);
                    break;

                case APPLY_ALIGN_SLAYER:
                    if (obj->affected[i].modifier > 1) {
                        sprintf(buf2, "SLAY GOOD\n\r");
                    } else if (obj->affected[i].modifier == 1) {
                        sprintf(buf2, "SLAY NEUTRAL\n\r");
                    } else {
                        sprintf(buf2, "SLAY EVIL\n\r");
                    }
                    break;

                default:
                    sprintf(buf2, "%ld\n\r", obj->affected[i].modifier);
                    break;
                }
                send_to_char(buf2, ch);
            }
        }
    } else if (!IS_NPC(victim)) {
        if (IS_SET(ch->player.user_flags, OLD_COLORS)) {
            sprintf(color1, "$c000p");
            sprintf(color2, "$c000C");
        } else {
            sprintf(color1, "$c000B");
            sprintf(color2, "$c000w");
        }
        sprintf(buf, "%sYou feel informed:\n\r", color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sName: %s%s is ", color1, color2, GET_NAME(victim));
        send_to_char(buf, ch);

        age2(victim, &ma);
        sprintf(buf, "%s%d%s years,  %s%d%s months, %s%d%s days, %s%d%s hours"
                     " old.\n\r", color2, ma.year, color1, color2, ma.month,
                     color1, color2, ma.day, color1, color2, ma.hours, color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sHeight:  %s%d%s cm, Weight:  %s%d%s pounds.\n\r",
                color1, color2, GET_HEIGHT(victim), color1, color2,
                GET_WEIGHT(victim), color1);
        send_to_char(buf, ch);

        sprintf(buf, "%sArmor Class %s%d\n\r", color1, color2,
                victim->points.armor);
        send_to_char(buf, ch);

        if (GET_LEVEL(ch, BestMagicClass(ch)) > 30) {
            sprintf(buf, "%sStr %s%d%s/%s%d%s, Int %s%d%s, Wis %s%d%s, Dex "
                         "%s%d%s, Con %s%d$%s, Ch %s%d\n\r",
                    color1, color2, GET_STR(victim), color1, color2,
                    GET_ADD(victim), color1, color2, GET_INT(victim),
                    color1, color2, GET_WIS(victim), color1, color2,
                    GET_DEX(victim), color1, color2, GET_CON(victim), color1,
                    color2, GET_CHR(victim));
            send_to_char(buf, ch);
        }
    } else {
        send_to_char("You learn nothing new.\n\r", ch);
    }
    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
#if 0
        act("You are overcome by a wave of exhaustion.",FALSE,ch,0,0,TO_CHAR);
        act("$n slumps to the ground, exhausted.",FALSE,ch,0,0,TO_ROOM);
        GET_POS(ch) = POSITION_STUNNED;
#endif
        WAIT_STATE(ch, PULSE_VIOLENCE * 2);

    }
}

void spell_enchant_armor(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             i;
    int             count = 0;

    if(!ch || !obj || MAX_OBJ_AFFECT < 2) {
        return;
    }

    if ((GET_ITEM_TYPE(obj) == ITEM_ARMOR) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location == APPLY_NONE) {
                count++;
            }
            if (obj->affected[i].location == APPLY_ARMOR ||
                obj->affected[i].location == APPLY_SAVE_ALL ||
                obj->affected[i].location == APPLY_SAVING_PARA ||
                obj->affected[i].location == APPLY_SAVING_ROD ||
                obj->affected[i].location == APPLY_SAVING_PETRI ||
                obj->affected[i].location == APPLY_SAVING_BREATH ||
                obj->affected[i].location == APPLY_SAVING_SPELL ||
                obj->affected[i].location == APPLY_SAVE_ALL) {
                send_to_char("This item may not hold further enchantments.\n\r",
                             ch);
                return;
            }
        }

        if (count < 2) {
            send_to_char("This item may not be enchanted.\n\r", ch);
            return;
        }

        /*
         * find the slots
         */
        i = getFreeAffSlot(obj);

        SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);

        obj->affected[i].location = APPLY_ARMOR;
        obj->affected[i].modifier = -1;
        if (level > 20) {
            obj->affected[i].modifier -= 1;
        }
        if (level > 40) {
            obj->affected[i].modifier -= 1;
        }
        if (level > MAX_MORT) {
            obj->affected[i].modifier -= 1;
        }
        if (level >= BIG_GUY) {
            obj->affected[i].modifier -= 1;
        }
        i = getFreeAffSlot(obj);

        obj->affected[i].location = APPLY_SAVE_ALL;
        obj->affected[i].modifier = 0;
        if (level > 30) {
            obj->affected[i].modifier -= 1;
        }
        if (level > MAX_MORT) {
            obj->affected[i].modifier -= 1;
        }
        if (level >= BIG_GUY) {
            obj->affected[i].modifier -= 1;
        }
        if (IS_GOOD(ch)) {
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_EVIL | ITEM_ANTI_NEUTRAL);
            act("$p glows blue.", FALSE, ch, obj, 0, TO_CHAR);
        } else if (IS_EVIL(ch)) {
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
            act("$p glows red.", FALSE, ch, obj, 0, TO_CHAR);
        } else {
            SET_BIT(obj->obj_flags.extra_flags,
                    ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
            act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
        }
    }
}

/*
 ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;
    struct obj_data *burn;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else if (!saves_spell(victim, SAVING_SPELL - 4)) {
        /*
         * Fail two saves, burn the wings
         */
        BurnWings(victim);
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    /*
     * And now for the damage on inventory
     */

    /*
     * DamageStuff(victim, FIRE_DAMAGE);
     */

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p burns into cinders.", 0, victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_frost_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;
    struct obj_data *frozen;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FROST_BREATH);

    /*
     * And now for the damage on inventory
     */

    for (frozen = victim->carrying;
         frozen && ITEM_TYPE(frozen) != ITEM_DRINKCON &&
         ITEM_TYPE(frozen) != ITEM_POTION;
         frozen = frozen->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && frozen) {
            act("$p shatters.", 0, victim, frozen, 0, TO_CHAR);
            extract_obj(frozen);
        }
    }
}

void spell_acid_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    int             apply_ac(struct char_data *ch, int eq_pos);

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_ACID_BREATH);
}

void spell_gas_breath(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
}

void spell_lightning_breath(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_lightning, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
}

void spell_wizard_eye(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_WIZARDEYE)) {
        act("$n summons a disembodied eye to scout ahead", TRUE, victim, 0,
            0, TO_ROOM);
        act("You summon a disembodied eye to scout ahead.", TRUE, victim,
            0, 0, TO_CHAR);

        af.type = SPELL_WIZARDEYE;
        af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SCRYING;
        affect_to_char(victim, &af);
    } else if (ch != victim) {
        act("$N already uses a wizard eye.", FALSE, ch, 0, victim, TO_CHAR);
    } else {
        act("You already have a wizard eye.", FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_disintegrate(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             i,
                    damage;
    struct obj_data *x;
    char            buf[MAX_STRING_LENGTH + 30];

    if (!ch) {
        Log("!ch in spell_disintegrate");
        return;
    }

    if (!victim) {
        Log("!victim in spell_disintegrate");
        return;
    }

    damage = dice(level, 10);
    if (!saves_spell(victim, SAVING_SPELL)) {
        i = 0;
        if (!(IS_PC(ch) && IS_PC(victim))) {
            /*
             * get rid of player-player scrappage, Lennya20030320
             */
            if (!IS_SET(real_roomp(victim->in_room)->room_flags, ARENA_ROOM)) {
                /*
                 * frag their EQ
                 */
                while (i < MAX_WEAR) {
                    if (victim->equipment[i]) {
                        obj = victim->equipment[i];
                        if (!ItemSave(obj, SPELL_DISINTEGRATE)) {
                            act("$p turns red hot, $N screams, then it "
                                "disappears in a puff of smoke!", TRUE, ch,
                                obj, victim, TO_CHAR);
                            if (obj->equipped_by || obj->carried_by) {
                                act("$p, held by $N, disappears in a puff of "
                                    "smoke!", TRUE, ch, obj, victim, TO_ROOM);
                            }

                            if (obj->carried_by) {
                                obj_from_char(obj);
                            } else if (obj->equipped_by) {
                                obj = unequip_char(obj->equipped_by,
                                                   obj->eq_pos);
                            } else if (obj->in_obj) {
                                obj_from_obj(obj);
                                obj_to_room(obj, ch->in_room);
                            } else if (obj->contains) {
                                while (obj->contains) {
                                    x = obj->contains;
                                    obj_from_obj(x);
                                    obj_to_room(x, ch->in_room);
                                }
                            }
                            if (obj) {
                                extract_obj(obj);
                            }
                        } else if (obj) {
                            act("$c0010$p resists the disintegration ray "
                                "completely!", TRUE, ch, obj, victim, TO_VICT);
                            act("$c0010$p carried by $N, resists $n's "
                                "disintegration ray!", TRUE, ch, obj, victim,
                                TO_ROOM);
                        }
                    }
                    i++;
                }
            }
        } else {
            /*
             * player hitting player with disint outside arena
             */
            sprintf(buf, "%s just hit %s with a disintegrate outside the "
                         "arena!", GET_NAME(ch), GET_NAME(victim));
            Log(buf);
        }
    } else {
        /*
         * we saved ! 1/2 dam and no EQ frag
         */
        damage >>= 1;
    }

    MissileDamage(ch, victim, damage, SPELL_DISINTEGRATE);
    if (IS_PC(ch) && IS_PC(victim)) {
        GET_ALIGNMENT(ch) -= 2;
    }
}

void spell_dehydration_breath(int level, struct char_data *ch,
                              struct char_data *victim,
                              struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_dehydration, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
    spell_energy_drain(level, ch, victim, 0);
    spell_weakness(level, ch, victim, 0);
}

void spell_vapor_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_vapor, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    spell_energy_drain(level, ch, victim, 0);
    spell_weakness(level, ch, victim, 0);
}

void spell_sound_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_sound, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
    spell_paralyze(level, ch, victim, 0);
    spell_silence(level, ch, victim, 0);
    spell_fear(level, ch, victim, 0);
}

void spell_shard_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;
    struct obj_data *burn;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);

    /*
     * And now for the damage on inventory
     */

    /*
     * DamageStuff(victim, FIRE_DAMAGE);
     */

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed by the projectiles", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_sleep_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_sleep, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
    spell_sleep(level, ch, victim, 0);
    spell_sleep(level, ch, victim, 0);
}

void spell_light_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;
    struct obj_data *burn;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_light, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    spell_blindness(level, ch, victim, 0);
    spell_blindness(level, ch, victim, 0);

    /*
     * And now for the damage on inventory
     */

    /*
     * DamageStuff(victim, FIRE_DAMAGE);
     */

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blinding light", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_dark_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    int             hpch;

    if (!victim || !ch) {
        Log("!ch || !victim in breath_dark, magic.c");
        return;
    }

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    hpch = GET_MAX_HIT(ch);
    hpch *= level;
    hpch /= GetMaxLevel(ch);
    hpch /= 2;
    if (hpch < 10) {
        hpch = 10;
    }
    dam = hpch;

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
    spell_energy_drain(level, ch, victim, 0);
    spell_weakness(level, ch, victim, 0);
    spell_blindness(level, ch, victim, 0);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
