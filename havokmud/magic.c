#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"

/*
 * Extern structures
 */

struct time_info_data age(struct char_data *ch);

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
    int             dam;
    
    assert(ch && victim);
    dam = dice(5,10);
    
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
            do_look(tmp, NULL, 0);
        }
    }
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
#ifdef ZONE_LOCATE
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
#ifdef ZONE_LOCATE
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
#ifdef ZONE_LOCATE
                       && (real_roomp(ch->in_room)->zone ==
                           real_roomp(i->in_room)->zone)
#endif
                ) {
                sprintf(buf2, "%s in %s.\n\r", i->short_description,
                        i->in_obj->short_description);
                strcat(buf, buf2);
            } else
#ifdef ZONE_LOCATE
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
#ifdef ZONE_LOCATE
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


void spell_sanctuary(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_SANCTUARY) &&
        !IS_AFFECTED(victim, AFF_SANCTUARY)) {
        act("$n is surrounded by a white aura.", TRUE, victim, 0, 0, TO_ROOM);
        act("You start glowing.", TRUE, victim, 0, 0, TO_CHAR);

        af.type = SPELL_SANCTUARY;

        if (IS_IMMORTAL(ch)) {
            af.duration = level;
        } else {
            af.duration = (ch->specials.remortclass == CLERIC_LEVEL_IND + 1 ? 
                           4 : 3); 
        }
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SANCTUARY;
        affect_to_char(victim, &af);
    }
}



void spell_word_of_recall(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    int             location;

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
    do_look(victim, NULL, 15);

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
    do_look(v, NULL, 15);

    if (IS_NPC(v) && !IS_SET(v->specials.act, ACT_POLYSELF) &&
        (IS_SET(v->specials.act, ACT_AGGRESSIVE) ||
         IS_SET(v->specials.act, ACT_META_AGG)) && CAN_SEE(v, c)) {
        act("$n growls at you", 1, v, 0, c, TO_VICT);
        act("$n growls at $N", 1, v, 0, c, TO_NOTVICT);
        hit(v, c, TYPE_UNDEFINED);
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







/*
 ***************************************************************************
 *                     Not cast-able spells                                  *
 * ************************************************************************* */




/*
 ***************************************************************************
 *                     NPC spells..                                          *
 * ************************************************************************* */

void spell_fire_breath(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }
    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else if (!saves_spell(victim, SAVING_SPELL - 4)) {
        BurnWings(victim);
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying; burn && 
         burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p burns to cinders.", 0, victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_frost_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int              dam = 0;
    struct obj_data *frozen;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }
    
    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FROST_BREATH);

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
    int                   dam = 0;
    struct      obj_data *burn;
    
    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_ACID_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is completely corroded.", 0, victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_gas_breath(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct      obj_data *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed by the cloud of gas.", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_lightning_breath(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int                  dam = 0;
    struct  obj_data    *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
    
    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is completely electrocuted.", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}



void spell_dehydration_breath(int level, struct char_data *ch,
                              struct char_data *victim,
                              struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
        af.type = SPELL_WEAKNESS;
        af.duration = 5 + level;
        af.modifier = -dice(1,6);
        af.location = APPLY_STR;
        affect_join(victim, &af, FALSE, FALSE);
        
    } else {
        af.type = SPELL_WEAKNESS;
        af.duration = 5 + level;
        af.modifier = -dice(1,6) + 6;
        af.location = APPLY_STR;
        affect_join(victim, &af, FALSE, FALSE);
    }
        
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
}

void spell_vapor_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    
    af.type = SPELL_BLINDNESS;
    af.location = APPLY_HITROLL;
    af.modifier = -12;
    af.duration = level / 2;
    af.bitvector = AFF_BLIND;
    affect_to_char(victim, &af);

    af.location = APPLY_AC;
    af.modifier = +40;
    affect_to_char(victim, &af);
}

void spell_sound_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);    

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    if (!saves_spell(victim, SAVING_BREATH)) {
       if (GET_POS(victim) > POSITION_STUNNED) {
           send_to_char("$c000WYou are stunned by the painful vibrations!\n\r",
                        victim);
           act("$n is stunned by the vibrations!", 
               FALSE, victim, NULL, NULL, TO_ROOM);
           GET_POS(victim) = POSITION_STUNNED;
        }
    } else {
        act("$n ceases to make noise!", TRUE, victim, NULL, NULL, TO_ROOM);
        send_to_char("You can't hear anything!\n\r", victim);
        af.type = SPELL_SILENCE;
        af.duration = 1;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SILENCE;
        affect_to_char(victim, &af);

        do_flee(victim, "", 0);
    }
    MissileDamage(ch, victim, dam, SPELL_LIGHTNING_BREATH);
}

void spell_shard_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -12;
        af.duration = level;
        af.bitvector = AFF_BLIND;
        affect_to_char(victim, &af);

        af.location = APPLY_AC;
        af.modifier = +40;
        affect_to_char(victim, &af);
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
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        af.type = SPELL_SLEEP;
        af.duration = 5 + level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SLEEP;
        affect_join(victim, &af, FALSE, FALSE);
        
        act("$n goes to sleep.", TRUE, victim, NULL, NULL, TO_ROOM);
        GET_POS(victim) = POSITION_SLEEPING;
    }
    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH); 
}

void spell_light_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    spell_blindness(level, ch, victim, 0);
    spell_blindness(level, ch, victim, 0);

    /*
     * And now for the damage on inventory
     */

#if 0
    DamageStuff(victim, FIRE_DAMAGE);
#endif

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
    int             dam = 0;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    
    
    if (saves_spell(victim, SAVING_BREATH)) {
        dam = GET_HIT(victim) * .50;
    } else {
        dam = GET_HIT(victim) * .75;
        
        af.type = SPELL_BLINDNESS;
        af.location = APPLY_HITROLL;
        af.modifier = -12;
        af.duration = level / 2;
        af.bitvector = AFF_BLIND;
        affect_to_char(victim, &af);

        af.location = APPLY_AC;
        af.modifier = +40;
        affect_to_char(victim, &af);
    }

    MissileDamage(ch, victim, dam, SPELL_GAS_BREATH);
}

void spell_desertheat_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blistering heat", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_repulsion_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0,
                    attempt,
                    i;
    struct obj_data *burn;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
        Log("saved");
    } else {
        Log("failed save");
        if (GET_POS(victim) > POSITION_SLEEPING && 
            !IS_AFFECTED(victim, AFF_PARALYSIS) &&
            !IS_SET(victim->specials.affected_by2, AFF2_BERSERK) &&
            !IS_SET(victim->specials.affected_by2, AFF2_STYLE_BERSERK)) {
            Log("good to go");    
            for (i = 0; i < 6; i++) {
                attempt = i;
                if (CAN_GO(victim, attempt) &&
                    !IS_SET(real_roomp(EXIT(victim, 
                                            attempt)->to_room)->room_flags,
                            DEATH) &&
                    !IS_SET(real_roomp(EXIT(victim, 
                                            attempt)->to_room)->room_flags,
                            NO_FLEE)) {
                    act("$n is repulsed and flees!", 
                        TRUE, victim, 0, 0, TO_ROOM);
                    send_to_char("You are repulsed and flee head over "
                                 "heels.\n\r", victim);
                    if (victim->specials.fighting) {
                        stop_fighting(victim);
                    }
                    if (ch->specials.fighting == victim) {
                        stop_fighting(ch);
                    }
                    GET_POS(victim) = POSITION_STANDING;
                    MoveOne(victim, attempt);
                    Log("moving victim");
                    GET_MOVE(victim) = -100;
                    RemHated(ch, victim);
                }
            }
        }
    }
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);
    for (burn = victim->carrying;
        burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
        burn->obj_flags.type_flag != ITEM_WAND &&
        burn->obj_flags.type_flag != ITEM_STAFF &&            
        burn->obj_flags.type_flag != ITEM_BOAT;
        burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is burnt to a crisp by the blistering heat", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

void spell_slow_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        if (!affected_by_spell(victim, SPELL_SLOW)) {
            
            send_to_char("You feel very slow!\n\r", victim);
            
            af.type = SPELL_SLOW;
            af.duration = 10;
            af.modifier = 1;
            af.location = APPLY_BV2;
            af.bitvector = AFF2_SLOW;
            affect_to_char(victim, &af);
        }
    }
        
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed.", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);

        }
    }
}

void spell_paralyze_breath(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    int              dam = 0;
    struct obj_data *burn;
    struct affected_type af;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

    if (saves_spell(victim, SAVING_BREATH)) {
        dam >>= 1;
    } else {
        if (!affected_by_spell(victim, SPELL_PARALYSIS)) {
            
            send_to_char("Your limbs freeze in place!\n\r", victim);
            
            af.type = SPELL_PARALYSIS;
            af.duration = 4 + level;
            af.modifier = 0;
            af.location = APPLY_NONE;
            af.bitvector = AFF_PARALYSIS;
            affect_join(victim, &af, FALSE, FALSE);
        }
    }
        
    MissileDamage(ch, victim, dam, SPELL_FIRE_BREATH);

    for (burn = victim->carrying;
         burn && burn->obj_flags.type_flag != ITEM_SCROLL &&
         burn->obj_flags.type_flag != ITEM_WAND &&
         burn->obj_flags.type_flag != ITEM_STAFF &&
         burn->obj_flags.type_flag != ITEM_BOAT;
         burn = burn->next_content) {
        if (!saves_spell(victim, SAVING_BREATH) && burn) {
            act("$p is destroyed.", 0,
                victim, burn, 0, TO_CHAR);
            extract_obj(burn);

        }
    }
}

void spell_lozenge_breath(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             dam = 0;
    struct obj_data *burn;

    assert(victim && ch);
    
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if ( level < 30 ) {
        dam = dice(24, 6) + level;
    } else if ( level < 40 ) {
        dam = dice(24, 12) + level;
    } else if ( level < 45 ) {
        dam = dice(24, 24) + level;
    } else if ( level < 49 ) {
        dam = dice(48, 6) + level;
    } else if ( level == 50 ) {
        dam = dice(48, 12) + level;
    } else if ( level > 50 ) {
        dam = dice(level, 24) + level;
    }

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
            act("$p is destroyed by the exploding lozenge", 0, victim, burn,
                0, TO_CHAR);
            extract_obj(burn);
        }
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
