#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>

#include "protos.h"

/*
 * Extern structures 
 */
extern struct room_data *world;
extern struct char_data *character_list;

void SwitchStuff(struct char_data *giver, struct char_data *taker)
{
    struct obj_data *obj,
                   *next;
    float           ratio;
    int             j;

    /*
     *  take all the stuff from the giver, put in on the
     *  taker
     */

    for (j = 0; j < MAX_WEAR; j++) {
        if (giver->equipment[j]) {
            obj = unequip_char(giver, j);
            obj_to_char(obj, taker);
        }
    }

    for (obj = giver->carrying; obj; obj = next) {
        next = obj->next_content;
        obj_from_char(obj);
        obj_to_char(obj, taker);
    }

    /*
     *    gold...
     */

    GET_GOLD(taker) = GET_GOLD(giver);

    /*
     *   hit point ratio
     */

    ratio = (float) GET_HIT(giver) / GET_MAX_HIT(giver);
    GET_HIT(taker) = ratio * GET_MAX_HIT(taker);

    GET_HIT(taker) = MIN(GET_MAX_HIT(taker), GET_HIT(taker));

    /*
     * experience
     */

    if (!IS_IMMORTAL(taker)) {
        if (!IS_IMMORTAL(giver)) {
            GET_EXP(taker) = GET_EXP(giver);
        }
        GET_EXP(taker) = MIN(GET_EXP(taker), ABS_MAX_EXP);
    }

    /*
     *  humanoid monsters can cast spells
     */

    if (IS_NPC(taker)) {
        taker->player.class = giver->player.class;
        if (!taker->skills) {
            SpaceForSkills(taker);
        }
        for (j = 0; j < MAX_SKILLS; j++) {
            taker->skills[j].learned = giver->skills[j].learned;
            taker->skills[j].flags = giver->skills[j].flags;
            taker->skills[j].special = giver->skills[j].special;
            taker->skills[j].nummem = giver->skills[j].nummem;
        }

        for (j = MAGE_LEVEL_IND; j < MAX_CLASS; j++) {
            taker->player.level[j] = giver->player.level[j];
        }
    }

    /*
     * bring along special flags (like ansi, pagepause) & prompt -Lennya 
     */
    taker->player.user_flags = giver->player.user_flags;
    taker->specials.prompt = giver->specials.prompt;

    GET_MANA(taker) = GET_MANA(giver);
    GET_ALIGNMENT(taker) = GET_ALIGNMENT(giver);
}

void FailCharm(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER)) &&
        number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    if (!IS_PC(victim)) {
        AddHated(victim, ch);

        if (!victim->specials.fighting) {
            if (GET_POS(victim) > POSITION_SLEEPING) {
                set_fighting(victim, ch);
            } else if (number(0, 1)) {
                set_fighting(victim, ch);
            }
        }
    } else {
        send_to_char("You feel charmed, but the feeling fades.\n\r", victim);
    }
}

void FailSnare(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER)) &&
        number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    if (!IS_PC(victim)) {
        if (!victim->specials.fighting) {
            AddHated(victim, ch);
            set_fighting(victim, ch);
        }
    } else {
        send_to_char("You feel ensnared, but the feeling fades.\n\r", victim);
    }
}

void FailSleep(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER)) &&
        number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    send_to_char("You feel sleepy for a moment, but then you recover\n\r",
                 victim);
    if (!IS_PC(victim)) {
        AddHated(victim, ch);
        if (!victim->specials.fighting && GET_POS(victim) > POSITION_SLEEPING) {
            set_fighting(victim, ch);
        }
    }
}

void FailPara(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER)) &&
        number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    send_to_char("You feel frozen for a moment, but then you recover\n\r",
                 victim);

    if (!IS_PC(victim)) {
        AddHated(victim, ch);
        if (!victim->specials.fighting && GET_POS(victim) > POSITION_SLEEPING) {
            set_fighting(victim, ch);
        }
    }
}

void FailCalm(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER)) &&
        number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    send_to_char("You feel happy and easygoing, but the effect soon fades.\n\r",
                 victim);

    if (!IS_PC(victim) && !victim->specials.fighting && !number(0, 2)) {
        set_fighting(victim, ch);
    }
}

void FailPoison(struct char_data *victim, struct char_data *ch)
{
    if ((OnlyClass(ch, CLASS_MAGIC_USER) || OnlyClass(ch, CLASS_SORCERER) ||
        OnlyClass(ch, CLASS_NECROMANCER)) && number(1, 100) > 50) {
        /* 
         * give single classed mages a break. 
         */
        return;
    }

    if (!IS_PC(victim)) {
#if 0
        AddHated(victim, ch);
#endif
        if (!victim->specials.fighting) {
            if (GET_POS(victim) > POSITION_SLEEPING) {
                set_fighting(victim, ch);
            } else if (number(0, 1)) {
                set_fighting(victim, ch);
            }
        }
    } else {
        send_to_char("You feel sick, but the feeling fades.\n\r", victim);
    }
}

void            do_snoop(struct char_data *ch, char *argument, int cmd);

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

void cast_mana(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
        spell_mana(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_mana(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) {
                spell_mana(level, ch, tar_ch, 0);
            }
    default:
        Log("Serious problem in 'mana'");
        break;
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

void cast_geyser(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *victim, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_geyser(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in geyser!");
        break;
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

void cast_green_slime(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_green_slime(level, ch, victim, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (victim) {
            spell_green_slime(level, ch, victim, 0);
        } else if (!tar_obj) {
            spell_green_slime(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (victim) {
            spell_green_slime(level, ch, victim, 0);
        }
        break;
    default:
        Log("Serious screw-up in green Slime!");
        break;
    }
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

void cast_prot_dragon_breath(int level, struct char_data *ch, char *arg,
                             int type, struct char_data *tar_ch,
                             struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath.");
        break;
    }
}
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

void spell_wrath_god(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(level, 4);

    if (saves_spell(victim, SAVING_SPELL)) {
        dam >>= 1;
    }
    if (GET_RACE(ch) == RACE_GOD) {
        dam = 0;
    }
    if (!HitOrMiss(ch, victim, CalcThaco(ch))) {
        dam = 0;
    }
    damage(ch, victim, dam, SPELL_WRATH_GOD);
}

void cast_wrath_god(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        act("$n calls upons the wrath of gods to destroy $N.", FALSE, ch,
            0, tar_ch, TO_NOTVICT);
        act("You call upon the wrath of gods to destroy $N.", FALSE, ch, 0,
            tar_ch, TO_CHAR);
        spell_wrath_god(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_ch) {            
            spell_wrath_god(level, ch, tar_ch, 0);
        } else {
            spell_wrath_god(level, ch, ch, 0);
        }
        break;
    case SPELL_TYPE_WAND:
        if (tar_ch) {
            spell_wrath_god(level, ch, tar_ch, 0);
        }
        break;
    default:
        Log("Serious screw-up in wrath of god!");
        break;
    }
}

void spell_dragon_ride(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(ch, SPELL_DRAGON_RIDE)) {
        send_to_char("Already affected\n\r", ch);
        return;
    }

    af.type = SPELL_DRAGON_RIDE;
    af.duration = level;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = AFF_DRAGON_RIDE;
    affect_to_char(ch, &af);
}

void cast_dragon_ride(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dragon_ride(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in dragon_ride.");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
