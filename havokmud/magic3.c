#include "config.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"

/*
 * Extern structures
 */
extern const struct race_type race_list[];
extern struct room_data *world;
extern struct obj_data *object_list;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern long     SystemFlags;
extern int      top_of_world;
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
void            add_follower(struct char_data *ch,
                             struct char_data *leader);
bool            circle_follow(struct char_data *ch,
                              struct char_data *victim);
void            stop_follower(struct char_data *ch);


#define STATE(d) ((d)->connected)
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))
#define TREE 6110
#define KITTEN  3090
#define PUPPY   3091
#define BEAGLE  3092
#define ROTT    3093
#define WOLF    3094
#define GOLEM 38
#define CREEPING_DEATH 39
#define ANISUM  9007
#define FIRE_ELEMENTAL  40
#define SAPLING  45
#define LITTLE_ROCK  50
#define DUST_DEVIL 60
#define PORTAL 31
#define MOUNT_ONE 65
#define MOUNT_GOOD 69
#define MOUNT_EVIL 70
#define MOUNT_NEUT 71
#define COLD_LIGHT 19
#define CB_TEMPLATE 4
#define TONGUE_ITEM 22
#define FG_TEMPLATE 6

/*
 * Extern procedures
 */


/*
 * druid spells
 */

void spell_tree_travel(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_TREE_TRAVEL)) {
        af.type = SPELL_TREE_TRAVEL;

        af.duration = 24;
        af.modifier = -5;
        af.location = APPLY_AC;
        af.bitvector = AFF_TREE_TRAVEL;
        affect_to_char(victim, &af);

        send_to_char("You feel as one with the trees... Groovy!\n\r", victim);
    } else {
        send_to_char("Nothing seems to happen\n\r", ch);
    }
}

void spell_transport_via_plant(int level, struct char_data *ch,
                               struct char_data *victim, char *arg)
{
    struct room_data *rp;
    struct obj_data *o;
    struct obj_data *i;
    struct obj_data *obj = NULL;
    char            name[254];
    int             found = 0;

    /*
     * find the tree in the room
     */
    rp = real_roomp(ch->in_room);
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_TREE) {
            break;
        }
    }

    if (!o) {
        send_to_char("You need to have a tree nearby\n\r", ch);
        return;
    }

    sprintf(name, "%s", arg);

    /*
     * find the target tree
     */
    for (i = object_list; i; i = i->next) {
        if (isname(name, i->name) && ITEM_TYPE(i) == ITEM_TREE) {
            /*
             * we found a druid tree with the right name
             */
            obj = i;
            found = 1;
            break;
        }
    }

    if (!found) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }
#if 0
    if (ITEM_TYPE(obj) != ITEM_TREE) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }
#endif

    if (obj->in_room < 0) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }

    if (!real_roomp(obj->in_room)) {
        send_to_char("That tree is nowhere to be found.\n\r", ch);
        return;
    }

    act("$n touches $p, and slowly vanishes within!", FALSE, ch, o, 0, TO_ROOM);
    act("You touch $p, and join your forms.", FALSE, ch, o, 0, TO_CHAR);
    char_from_room(ch);
    char_to_room(ch, obj->in_room);
    act("$p rustles slightly, and $n magically steps from within!", FALSE,
        ch, obj, 0, TO_ROOM);
    act("You are instantly transported to $p!", FALSE, ch, obj, 0, TO_CHAR);
    do_look(ch, "\0", 0);
}

void spell_plant_gate(int level, struct char_data *ch,
                      struct char_data *victim, char *arg)
{
    struct room_data *rp;
    struct obj_data *o,
                   *i,
                   *obj = NULL;
    struct char_data *tch,
                   *tch2;
    int             has_companions = 0;
    char            name[254];
    /*
     * find the tree in the room
     */

    rp = real_roomp(ch->in_room);
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_TREE) {
            break;
        }
    }

    if (!o) {
        send_to_char("You need to have a tree nearby.\n\r", ch);
        return;
    }

    sprintf(name, "%s", arg);
    /*
     * find the target tree
     */
    for (i = object_list; i; i = i->next) {
        if (isname(name, i->name) && ITEM_TYPE(i) == ITEM_TREE) {
            /*
             * we found a druid tree with the right name
             */
            obj = i;
            break;
        }
    }

    if (ITEM_TYPE(obj) != ITEM_TREE) {
        send_to_char("Thats not a tree!\n\r", ch);
        return;
    }

    if (obj->in_room < 0) {
        send_to_char("That tree is nowhere to be found!\n\r", ch);
        return;
    }

    if (!real_roomp(obj->in_room)) {
        send_to_char("That tree is nowhere to be found!\n\r", ch);
        return;
    }

    act("$n places $s hand on $p and its surface wavers as a large gate opens "
        "within it!", FALSE, ch, o, 0, TO_ROOM);
    act("You place a hand on $p and its surface wavers as a large gate opens "
        "within it!", FALSE, ch, o, 0, TO_CHAR);

    for (tch = real_roomp(ch->in_room)->people; tch; tch = tch2) {
        tch2 = tch->next_in_room;
        if (in_group(tch, ch) && GET_POS(tch) > POSITION_SLEEPING &&
            tch != ch) {
            act("$n holds open the magical gateway in $p and ushers you "
                "through it.", FALSE, ch, o, tch, TO_VICT);
            act("$N steps inside the magical gate in $p and vanishes!",
                FALSE, ch, o, tch, TO_NOTVICT);
            act("You hold open the plant gate as $N steps inside and vanishes!",
                FALSE, ch, o, tch, TO_CHAR);
            char_from_room(tch);
            char_to_room(tch, obj->in_room);
            if (!has_companions) {
                act("$p rustles and a large gate opens within it!", FALSE,
                    tch, obj, 0, TO_ROOM);
            }
            act("$n suddenly appears as $e steps from the gate in $p!",
                FALSE, tch, obj, 0, TO_ROOM);
            act("You are instantly transported to $p!", FALSE, tch, obj, 0,
                TO_CHAR);
            do_look(tch, "\0", 0);
            has_companions++;
        }
    }

    if (has_companions) {
        send_to_char("After your companions are safely through the gate, "
                     "you step within and join forms!\n\r", ch);
    } else {
        send_to_char("You step within the gate and your forms merge!\n\r", ch);
    }

    act("$n steps within the magical gate in $p and vanishes just before it"
        " closes!", FALSE, ch, o, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, obj->in_room);

    if (!has_companions) {
        act("$p rustles and a large gate opens within it!",
            FALSE, ch, obj, 0, TO_ROOM);
    }

    act("You are instantly transported to $p!", FALSE, ch, obj, 0, TO_CHAR);
    do_look(ch, "\0", 0);
    act("$n steps through the magical gate in $p and the gate closes "
        "behind $m!", FALSE, ch, obj, 0, TO_ROOM);
}

void spell_speak_with_plants(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    char            buffer[128];

    assert(ch && obj);

    if (ITEM_TYPE(obj) != ITEM_TREE) {
        send_to_char("Sorry, you can't talk to that sort of thing\n\r", ch);
        return;
    }

    sprintf(buffer, "%s says 'Hi $n, how ya doin?'", fname(obj->name));
    act(buffer, FALSE, ch, obj, 0, TO_CHAR);
    act("$p rustles slightly.", FALSE, ch, obj, 0, TO_ROOM);
}


void spell_changestaff(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct obj_data *s;
    struct char_data *t;

    /*
     * player must be holding staff at the time
     */

    if (!ch->equipment[HOLD]) {
        send_to_char("You must be holding a staff!\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    s = unequip_char(ch, HOLD);
    if (ITEM_TYPE(s) != ITEM_STAFF) {
        act("$p is not sufficient to complete this spell",
            FALSE, ch, s, 0, TO_CHAR);
        extract_obj(s);
        return;
    }

    if (!s->obj_flags.value[2]) {
        act("$p is not sufficiently powerful to complete this spell",
            FALSE, ch, s, 0, TO_CHAR);
        extract_obj(s);
        return;
    }

    act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_ROOM);
    act("$p vanishes in a burst of flame!", FALSE, ch, s, 0, TO_CHAR);

    t = read_mobile(TREE, VIRTUAL);
    char_to_room(t, ch->in_room);
    GET_EXP(t) = 0;

    act("$n springs up in front of you!", FALSE, t, 0, 0, TO_ROOM);

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
    } else {
        af.type = SPELL_CHARM_PERSON;

        if (IS_PC(ch) || ch->master) {
            af.duration = follow_time(ch);
           /*
            * num charges
            */
            af.duration += s->obj_flags.value[2];
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(t, &af);
        } else {
            SET_BIT(t->specials.affected_by, AFF_CHARM);
        }
        add_follower(t, ch);

        extract_obj(s);
    }
}

/*
 * mage spells
 */
void spell_pword_kill(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             max = 80;

    max += level;
    max += level / 2;

    if (GET_MAX_HIT(victim) <= max || GetMaxLevel(ch) > 53) {
        damage(ch, victim, GET_MAX_HIT(victim) * 12, SPELL_PWORD_KILL);
    } else {
        send_to_char("They are too powerful to destroy this way\n\r", ch);
    }
}

void spell_pword_blind(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    if (GET_MAX_HIT(victim) <= 100 || GetMaxLevel(ch) > 53) {
        SET_BIT(victim->specials.affected_by, AFF_BLIND);
    } else {
        send_to_char("They are too powerful to blind this way\n\r", ch);
    }
}

void spell_chain_lightn(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             lev = level,
                    dam;
    struct char_data *t,
                   *next;
    int             count = 0;

    damage(ch, victim, dice(lev, 6), SPELL_LIGHTNING_BOLT);
    lev--;
    count++;

    for (t = real_roomp(ch->in_room)->people; t; t = next) {
        next = t->next_in_room;
        if (!in_group(ch, t) && t != victim && !IS_IMMORTAL(t)) {
            if (count > 8) {
                break;
            }
            dam = dice(lev, 6);
            if (saves_spell(t, SAVING_SPELL)) {
                dam >>= 1;
            }
            damage(ch, t, dam, SPELL_LIGHTNING_BOLT);
            lev--;
            count++;
        }
    }
}

void spell_scare(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    if (GetMaxLevel(victim) <= 5) {
        do_flee(victim, "\0", 0);
    }
}

void spell_haste(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_HASTE)) {
        act("$N is already hasty!", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (affected_by_spell(victim, SPELL_SLOW)) {
        act("$N doesn't seem so slow anymore.", FALSE, ch, 0, victim,
            TO_CHAR);
        affect_from_char(victim, SPELL_SLOW);
        return;
    }
#if 0
    /*
     * Why can't mobs haste anyway? commented out by Lennya
     */
    if (IS_NPC(victim)) {
        send_to_char("It doesn't seem to work\n", ch);
        return;
    }
#endif

    if (IS_IMMUNE(victim, IMM_HOLD)) {
        act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
        act("$n just tried to haste you, but you ignored it.", FALSE, ch,
            0, victim, TO_VICT);
        if (!in_group(ch, victim) && !IS_PC(ch)) {
            hit(victim, ch, TYPE_UNDEFINED);
        }
        return;
    }

    af.type = SPELL_HASTE;
    af.duration = (level < LOW_IMMORTAL) ? level : 99;
    af.modifier = 1;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_HASTE;
    affect_to_char(victim, &af);

    send_to_char("You feel fast!\n\r", victim);
    if (!IS_NPC(victim)) {
        victim->player.time.birth -= SECS_PER_MUD_YEAR;
    } else if (victim->desc && victim->desc->original) {
        victim->desc->original->player.time.birth -= SECS_PER_MUD_YEAR;
    }

    if (!in_group(ch, victim) && !IS_PC(ch)) {
        hit(victim, ch, TYPE_UNDEFINED);
    }
}

void spell_slow(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_SLOW)) {
        act("$N is already slowed.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (affected_by_spell(victim, SPELL_HASTE)) {
        affect_from_char(victim, SPELL_HASTE);
        act("$N doesnt seem so quick anymore.", FALSE, ch, 0, victim, TO_CHAR);
        return;
    }

    if (IS_IMMUNE(victim, IMM_HOLD)) {
        act("$N seems to ignore your spell", FALSE, ch, 0, victim, TO_CHAR);
        act("$n just tried to slow you, but you ignored it.", FALSE, ch, 0,
            victim, TO_VICT);
        if (!in_group(ch, victim) && !IS_PC(ch)) {
            hit(victim, ch, TYPE_UNDEFINED);
        }
        return;
    }

    af.type = SPELL_SLOW;
    af.duration = 10;
    af.modifier = 1;
    af.location = APPLY_BV2;
    af.bitvector = AFF2_SLOW;
    affect_to_char(victim, &af);

    send_to_char("You feel very slow!\n\r", victim);

    if (!IS_PC(victim) && !IS_SET(victim->specials.act, ACT_POLYSELF)) {
        hit(victim, ch, TYPE_UNDEFINED);
    }
}


void spell_familiar(int level, struct char_data *ch,
                    struct char_data **victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *f;

    if (affected_by_spell(ch, SPELL_FAMILIAR)) {
        send_to_char("You can't have more than 1 familiar per day\n\r", ch);
        return;
    }

    /*
     * depending on the level, one of the pet shop kids.
     */

    if (level < 2) {
        f = read_mobile(KITTEN, VIRTUAL);
    } else if (level < 4) {
        f = read_mobile(PUPPY, VIRTUAL);
    } else if (level < 6) {
        f = read_mobile(BEAGLE, VIRTUAL);
    } else if (level < 8) {
        f = read_mobile(ROTT, VIRTUAL);
    } else {
        f = read_mobile(WOLF, VIRTUAL);
    }
    char_to_room(f, ch->in_room);

    af.type = SPELL_FAMILIAR;
    af.duration = 24;
    af.modifier = -1;
    af.location = APPLY_ARMOR;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    act("$n appears in a flash of light!\n\r", FALSE, f, 0, 0, TO_ROOM);

    SET_BIT(f->specials.affected_by, AFF_CHARM);
    GET_EXP(f) = 0;
    add_follower(f, ch);
    IS_CARRYING_W(f) = 0;
    IS_CARRYING_N(f) = 0;

    *victim = f;
}

/*
 * cleric
 */

void spell_aid(int level, struct char_data *ch,
               struct char_data *victim, struct obj_data *obj)
{
    /*
     * combo bless, cure light woundsish
     */
    struct affected_type af;

    if (affected_by_spell(victim, SPELL_AID)) {
        send_to_char("Already in effect\n\r", ch);
        return;
    }

    GET_HIT(victim) += number(1, 8);

    update_pos(victim);

    act("$n looks aided", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel better!\n\r", victim);

    af.type = SPELL_AID;
    af.duration = 10;
    af.modifier = 1;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
}

void spell_holyword(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             lev,
                    t_align;
    struct char_data *t,
                   *next;

    if (level > 0) {
        t_align = -300;
    } else {
        level = -level;
        t_align = 300;
    }

    for (t = real_roomp(ch->in_room)->people; t; t = next) {
        next = t->next_in_room;
        if (!IS_IMMORTAL(t) && !IS_AFFECTED(t, AFF_SILENCE)) {
            if (level > 0) {
                if (GET_ALIGNMENT(t) <= t_align) {
                    if ((lev = GetMaxLevel(t)) <= 4) {
                        damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_HOLY_WORD);
                    } else if (lev <= 8) {
                        damage(ch, t, 1, SPELL_HOLY_WORD);
                        spell_paralyze(level, ch, t, 0);
                    } else if (lev <= 12) {
                        damage(ch, t, 1, SPELL_HOLY_WORD);
                        spell_blindness(level, ch, t, 0);
                    } else if (lev <= 16) {
                        damage(ch, t, 0, SPELL_HOLY_WORD);
                        GET_POS(t) = POSITION_STUNNED;
                    }
                }
            } else if (GET_ALIGNMENT(t) >= t_align) {
                if ((lev = GetMaxLevel(t)) <= 4) {
                    damage(ch, t, GET_MAX_HIT(t) * 20, SPELL_UNHOLY_WORD);
                } else if (lev <= 8) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    spell_paralyze(level, ch, t, 0);
                } else if (lev <= 12) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    spell_blindness(level, ch, t, 0);
                } else if (lev <= 16) {
                    damage(ch, t, 1, SPELL_UNHOLY_WORD);
                    GET_POS(t) = POSITION_STUNNED;
                }
            }
        }
    }
}


void spell_golem(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    int             count = 0;
    int             armor;
    struct char_data *gol;
    struct obj_data *helm = 0,
                   *jacket = 0,
                   *leggings = 0,
                   *sleeves = 0,
                   *gloves = 0,
                   *boots = 0,
                   *o;
    struct room_data *rp;

    /*
     * you need: helm, jacket, leggings, sleeves, gloves, boots
     */

    rp = real_roomp(ch->in_room);
    if (!rp) {
        return;
    }
    for (o = rp->contents; o; o = o->next_content) {
        if (ITEM_TYPE(o) == ITEM_ARMOR) {
            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HEAD) && !helm) {
                count++;
                helm = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) && !boots) {
                count++;
                boots = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_BODY) && !jacket) {
                count++;
                jacket = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) && !leggings) {
                count++;
                leggings = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS) && !sleeves) {
                count++;
                sleeves = o;
                continue;
            }

            if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) && !gloves) {
                count++;
                gloves = o;
                continue;
            }
        }
    }

    if (count < 6) {
        send_to_char("You don't have all the correct pieces!\n\r", ch);
        return;
    }

    if (count > 6) {
        send_to_char("Smells like an error to me!\n\r", ch);
        return;
    }

    if (!boots || !sleeves || !gloves || !helm || !jacket || !leggings) {
        /*
         * shouldn't get this far
         */
        send_to_char("You don't have all the correct pieces!\n\r", ch);
        return;
    }

    gol = read_mobile(GOLEM, VIRTUAL);
    char_to_room(gol, ch->in_room);

    /*
     * add up the armor values in the pieces
     */
    armor = boots->obj_flags.value[0];
    armor += helm->obj_flags.value[0];
    armor += gloves->obj_flags.value[0];
    armor += (leggings->obj_flags.value[0] * 2);
    armor += (sleeves->obj_flags.value[0] * 2);
    armor += (jacket->obj_flags.value[0] * 3);

    GET_AC(gol) -= armor;

    gol->points.max_hit = dice((armor / 6), 10) + GetMaxLevel(ch);
    GET_HIT(gol) = GET_MAX_HIT(gol);
    GET_LEVEL(gol, WARRIOR_LEVEL_IND) = (armor / 6);
    SET_BIT(gol->specials.affected_by, AFF_CHARM);
    GET_EXP(gol) = 0;
    IS_CARRYING_W(gol) = 0;
    IS_CARRYING_N(gol) = 0;
    gol->player.class = CLASS_WARRIOR;

    if (GET_LEVEL(gol, WARRIOR_LEVEL_IND) > 10) {
        gol->mult_att += 0.5;
    }
    /*
     * add all the effects from all the items to the golem
     */
    AddAffects(gol, boots);
    AddAffects(gol, gloves);
    AddAffects(gol, jacket);
    AddAffects(gol, sleeves);
    AddAffects(gol, leggings);
    AddAffects(gol, helm);

    act("$n waves $s hand over a pile of armor on the floor", FALSE, ch, 0,
        0, TO_ROOM);
    act("You wave your hands over the pile of armor", FALSE, ch, 0, 0, TO_CHAR);
    act("The armor flys together to form a humanoid figure!", FALSE, ch, 0,
        0, TO_ROOM);
    act("$N is quickly assembled from the pieces", FALSE, ch, 0, gol, TO_CHAR);

    add_follower(gol, ch);
    extract_obj(helm);
    extract_obj(boots);
    extract_obj(gloves);
    extract_obj(leggings);
    extract_obj(sleeves);
    extract_obj(jacket);
}

/***************/

void spell_feeblemind(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SKILL_MINDBLANK) ||
        !saves_spell(victim, SAVING_SPELL)) {

        if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
            send_to_char("They are already dumb enough as it is!\n\r", ch);
            return;
        }
        if (IS_IMMORTAL(victim)) {
            send_to_char("Dur, that was stupid!\n\r", ch);
            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = -5;
            af.location = APPLY_INT;
            af.bitvector = 0;
            affect_to_char(ch, &af);

            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = 70;
            af.location = APPLY_SPELLFAIL;
            af.bitvector = 0;
            affect_to_char(ch, &af);
        } else {
            send_to_char("You feel really really dumb\n\r", victim);
            act("$n suddenly looks rather dumb.", FALSE, victim, 0, 0,
                TO_NOTVICT);
            
            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = -5;
            af.location = APPLY_INT;
            af.bitvector = 0;
            affect_to_char(victim, &af);

            af.type = SPELL_FEEBLEMIND;
            af.duration = 24;
            af.modifier = 70;
            af.location = APPLY_SPELLFAIL;
            af.bitvector = 0;
            affect_to_char(victim, &af);
#if 0
            /*
             * That is stupid... people were complaining... -MW
             */
            if (!victim->skills) {
                return;
            }
            t = number(1,100);
            while (1) {
                for (i=0;i<MAX_SKILLS;i++) {
                    if (victim->skills[i].learned)  {
                        t--;
                    }
                    if (t==0) {
                        victim->skills[i].learned = 0;
                        victim->skills[i].flags = 0;
                        break;
                    }
                }
                return;
            }
#endif
        }
    } else {
	    if (!victim->specials.fighting) {
            /*
             * they saved
             */
            set_fighting(victim, ch);
        }
        act("$n seems to resist your spell.", FALSE, victim, 0, 0, TO_CHAR);
    }
}

void spell_shillelagh(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             i;
    int             count = 0;

    if(!ch || !obj || MAX_OBJ_AFFECT < 2) {
        return;
    }

    if ((GET_ITEM_TYPE(obj) == ITEM_WEAPON) &&
        !IS_SET(obj->obj_flags.extra_flags, ITEM_MAGIC)) {

        if (!isname("club", obj->name)) {
            send_to_char("That isn't a club!\n\r", ch);
            return;
        }

        for (i = 0; i < MAX_OBJ_AFFECT; i++) {
            if (obj->affected[i].location == APPLY_NONE) {
                count++;
            }
            if (obj->affected[i].location == APPLY_HITNDAM ||
                obj->affected[i].location == APPLY_HITROLL ||
                obj->affected[i].location == APPLY_DAMROLL)
                return;
        }

        if (count < 2) {
            return;
        }
        /*
         * find the slots
         */
        i = getFreeAffSlot(obj);
        SET_BIT(obj->obj_flags.extra_flags, ITEM_MAGIC);
        obj->affected[i].location = APPLY_HITNDAM;
        obj->affected[i].modifier = 1;
        obj->obj_flags.value[1] = 2;
        obj->obj_flags.value[2] = 4;
        act("$p glows yellow.", FALSE, ch, obj, 0, TO_CHAR);
        SET_BIT(obj->obj_flags.extra_flags, ITEM_ANTI_GOOD | ITEM_ANTI_EVIL);
    }
}

void spell_goodberry(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);
    tmp_obj->name = strdup("berry blue blueberry");
    tmp_obj->short_description = strdup("a plump blueberry");
    tmp_obj->description = strdup("A scrumptions blueberry lies here.");
    tmp_obj->obj_flags.type_flag = ITEM_FOOD;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_HOLD;
    tmp_obj->obj_flags.value[0] = 10;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;
    tmp_obj->obj_flags.cost_per_day = 1;

    /*
     * give it a cure light wounds spell effect
     */

    SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);
    tmp_obj->affected[0].location = APPLY_EAT_SPELL;
    tmp_obj->affected[0].modifier = SPELL_CURE_LIGHT;
    tmp_obj->next = object_list;
    object_list = tmp_obj;
    obj_to_char(tmp_obj, ch);
    tmp_obj->item_number = -1;
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears in your hand.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void spell_flame_blade(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (ch->equipment[WIELD]) {
        send_to_char("You can't be wielding a weapon\n\r", ch);
        return;
    }

    CREATE(tmp_obj, struct obj_data, 1);
    clear_object(tmp_obj);
    tmp_obj->name = strdup("blade flame");
    tmp_obj->short_description = strdup("a flame blade");
    tmp_obj->description = strdup("A flame blade burns brightly here.");
    tmp_obj->obj_flags.type_flag = ITEM_WEAPON;
    tmp_obj->obj_flags.wear_flags = ITEM_TAKE | ITEM_WIELD;
    tmp_obj->obj_flags.value[0] = 0;
    tmp_obj->obj_flags.value[1] = 1;
    tmp_obj->obj_flags.value[2] = 4;
    tmp_obj->obj_flags.value[3] = 3;
    tmp_obj->obj_flags.weight = 1;
    tmp_obj->obj_flags.cost = 10;
    tmp_obj->obj_flags.cost_per_day = 1;
    SET_BIT(tmp_obj->obj_flags.extra_flags, ITEM_MAGIC);
    tmp_obj->affected[0].location = APPLY_DAMROLL;
    tmp_obj->affected[0].modifier = 4 + GET_LEVEL(ch, DRUID_LEVEL_IND) / 8;
    tmp_obj->next = object_list;
    object_list = tmp_obj;
    equip_char(ch, tmp_obj, WIELD);
    tmp_obj->item_number = -1;

    act("$p appears in your hand.", TRUE, ch, tmp_obj, 0, TO_CHAR);
    act("$p appears in $n's hand.", TRUE, ch, tmp_obj, 0, TO_ROOM);
}

void spell_animal_growth(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (!IsAnimal(victim)) {
        send_to_char("Thats not an animal\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_ANIMAL_GROWTH)) {
        act("$N is already affected by that spell", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
        send_to_char("You can't make it more powerful than you!\n\r", ch);
        return;
    }

    if (IS_PC(victim)) {
        send_to_char("It would be in bad taste to cast that on a player\n\r",
                     ch);
        return;
    }

    act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
    act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_ANIMAL_GROWTH;
    af.duration = 12;
    af.modifier = GET_MAX_HIT(victim);
    af.location = APPLY_HIT;
    af.bitvector = AFF_GROWTH;
    affect_to_char(victim, &af);

    af.type = SPELL_ANIMAL_GROWTH;
    af.duration = 12;
    af.modifier = 5;
    af.location = APPLY_HITNDAM;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_ANIMAL_GROWTH;
    af.duration = 12;
    af.modifier = 3;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    /*
     * GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
     */
}

void spell_insect_growth(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (GET_RACE(victim) != RACE_INSECT) {
        send_to_char("Thats not an insect.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_INSECT_GROWTH)) {
        act("$N is already affected by that spell", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
        send_to_char("You can't make it more powerful than you!\n\r", ch);
        return;
    }

    if (IS_PC(victim)) {
        send_to_char("It would be in bad taste to cast that on a player\n\r",
                     ch);
        return;
    }

    act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
    act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = GET_MAX_HIT(victim);
    af.location = APPLY_HIT;
    af.bitvector = AFF_GROWTH;
    affect_to_char(victim, &af);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = 5;
    af.location = APPLY_HITNDAM;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_INSECT_GROWTH;
    af.duration = 12;
    af.modifier = 3;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    /*
     * GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2*GetMaxLevel(victim);
     */
}


void spell_creeping_death(int level, struct char_data *ch,
                          struct char_data *victim, int dir)
{
    struct affected_type af;
    struct char_data *cd;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS) ||
        IS_SET(real_roomp(ch->in_room)->sector_type, SECT_INSIDE)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }
    /*
     * obj is really the direction that the death wishes to travel in
     */

    cd = read_mobile(CREEPING_DEATH, VIRTUAL);
    if (!cd) {
        send_to_char("None available\n\r", ch);
        return;
    }

    char_to_room(cd, ch->in_room);
    cd->points.max_hit += (number(1, 4) * 100) + 600;
    cd->points.hit = cd->points.max_hit;

    act("$n makes a horrid coughing sound.", FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("You feel an incredibly nasty feeling inside.\n\r", ch);
    act("A huge gout of poisonous insects spews forth from $n's mouth!",
        FALSE, ch, 0, 0, TO_ROOM);
    send_to_char("A huge gout of insects spews out of your mouth!\n\r", ch);
    act("The insects coalesce into a solid mass - the creeping death.",
        FALSE, ch, 0, 0, TO_ROOM);
    cd->generic = dir;
    /*
     * move the creeping death in the proper direction
     */
    do_move(cd, "\0", dir);
    act("$n slumps to the ground, exhausted.", FALSE, ch, 0, 0, TO_ROOM);
    act("You are overcome by a wave of exhaustion.", FALSE, ch, 0, 0, TO_CHAR);

    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
        GET_POS(ch) = POSITION_STUNNED;
        af.type = SPELL_CREEPING_DEATH;
        af.duration = 2;
        af.modifier = 10500;
        af.location = APPLY_SPELLFAIL;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    }
}

void spell_commune(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    struct char_data *c;
    struct room_data *rp,
                   *dp;
    char            buf[MAX_STRING_LENGTH],
                    buffer[MAX_STRING_LENGTH];

    /*
     * look up the creatures in the mob list, find the ones in this zone,
     * in rooms that are outdoors, and tell the caster about them
     */

    buffer[0] = '\0';
    buf[0] = '\0';

    dp = real_roomp(ch->in_room);
    if (!dp) {
        return;
    }
    if (IS_SET(dp->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    for (c = character_list; c; c = c->next) {
        rp = real_roomp(c->in_room);
        if (!rp) {
            return;
        }
        if (rp->zone == dp->zone) {
            if (!IS_SET(rp->room_flags, INDOORS)) {
                sprintf(buf, "%s is in %s\n\r",
                        (IS_NPC(c) ? c->player.short_descr : GET_NAME(c)),
                        rp->name);
                if (strlen(buf) + strlen(buffer) > MAX_STRING_LENGTH - 2) {
                    break;
                }
                strcat(buffer, buf);
                strcat(buffer, "\r");
            }
        }
    }

    page_string(ch->desc, buffer, 1);
}


void spell_animal_summon(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct char_data *mob;
    int             lev = 1,
                    i,
                    mlev,
                    mhps,
                    mtohit;
    struct room_data *rp;

    /*
     * modified by Lennya. Load a random mob, and adjust its stats
     * according to caster level
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
    if (IS_SET(rp->room_flags, INDOORS)) {
        send_to_char("You can only do this outdoors\n", ch);
        return;
    }
    if (affected_by_spell(ch, SPELL_ANIMAL_SUM_1)) {
        send_to_char("You can only do this once every 48 hours!\n\r", ch);
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }
    act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
    act("You perform the ritual of summoning.", TRUE, ch, 0, 0, TO_CHAR);

    lev = GetMaxLevel(ch);

    for (i = 0; i < 4; i++) {
        mob = read_mobile(ANISUM + number(0, 20), VIRTUAL);
        if (!mob) {
            continue;
        }
        /*
         * let's modify this guy according to caster level
         */
        if (lev > 45) {
            mlev = number(20, 23);
            mhps = number(37, 46);
            mtohit = 8;
        } else if (lev > 37) {
            mlev = number(17, 19);
            mhps = number(28, 35);
            mtohit = 6;
        } else if (lev > 28) {
            mlev = number(12, 14);
            mhps = number(18, 25);
            mtohit = 4;
        } else if (lev > 18) {
            mlev = number(7, 9);
            mhps = number(10, 17);
            mtohit = 2;
        } else {
            mlev = number(4, 6);
            mhps = number(0, 5);
            mtohit = 0;
        }
        mob->player.level[2] = mlev;
        mob->points.max_hit = mob->points.max_hit + mhps;
        mob->points.hit = mob->points.max_hit;
        mob->points.hitroll = mob->points.hitroll + mtohit;
        char_to_room(mob, ch->in_room);

        act("$n strides into the room.", FALSE, mob, 0, 0, TO_ROOM);
        if (too_many_followers(ch)) {
            act("$N takes one look at the size of your posse and just says no!",
                TRUE, ch, 0, victim, TO_CHAR);
            act("$N takes one look at the size of $n's posse and just says no!",
                TRUE, ch, 0, victim, TO_ROOM);
        } else {
            /*
             * charm them for a while
             */
            if (mob->master) {
                stop_follower(mob);
            }
            add_follower(mob, ch);
            af.type = SPELL_CHARM_PERSON;
            if (IS_PC(ch) || ch->master) {
                af.duration = GET_CHR(ch);
                af.modifier = 0;
                af.location = 0;
                af.bitvector = AFF_CHARM;
                affect_to_char(mob, &af);
            } else {
                SET_BIT(mob->specials.affected_by, AFF_CHARM);
            }
        }
        if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
            REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
        }
        if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
            SET_BIT(mob->specials.act, ACT_SENTINEL);
        }
    }
    af.type = SPELL_ANIMAL_SUM_1;
    af.duration = 48;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}


void spell_elemental_summoning(int level, struct char_data *ch,
                               struct char_data *victim, int spell)
{
    int             vnum;
    struct char_data *mob;
    struct affected_type af;

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to summon pets!\n\r",
                     ch);
        return;
    }

    if (affected_by_spell(ch, spell)) {
        send_to_char("You can only do this once per 24 hours\n\r", ch);
        return;
    }

    vnum = spell - SPELL_FIRE_SERVANT;
    vnum += FIRE_ELEMENTAL;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    mob = read_mobile(vnum, VIRTUAL);

    if (!mob) {
        send_to_char("None available\n\r", ch);
        return;
    }

    act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
    act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

    char_to_room(mob, ch->in_room);
    act("$n appears through a momentary rift in the ether!",
        FALSE, mob, 0, 0, TO_ROOM);
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

        if (IS_PC(ch) || ch->master) {
            af.duration = 24;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }
    }

    af.type = spell;
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);

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

void spell_reincarnate(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct char_data *newch;
    struct char_file_u st;
    struct descriptor_data *d;
    FILE           *fl;
    int             origage;
    int             origrace;
    int             newage;
    int             newrace;
    struct time_info_data my_age;

    if (!obj) {
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }
    if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
        send_to_char("You must cast this spell in the forest!\n\r", ch);
        return;
    }
    if (IS_CORPSE(obj)) {
        if (obj->char_vnum) {
            send_to_char("This spell only works on players\n\r", ch);
            return;
        }
        if (obj->char_f_pos) {
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
            if (st.talks[2] && st.abilities.con > 3) {
                st.points.exp *= 2;
                st.points.leadership_exp *= 2;
                st.talks[2] = TRUE;
                st.m_deaths--;
                st.abilities.con -= 1;

                act("The forest comes alive with the sounds of birds and "
                    "animals", TRUE, ch, 0, 0, TO_CHAR);
                act("The forest comes alive with the sounds of birds and "
                    "animals", TRUE, ch, 0, 0, TO_ROOM);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_CHAR);
                act("$p dissappears in the blink of an eye.",
                    TRUE, ch, obj, 0, TO_ROOM);
                GET_MANA(ch) = 1;
                GET_MOVE(ch) = 1;
                GET_HIT(ch) = 1;
                GET_POS(ch) = POSITION_SITTING;
                act("$n collapses from the effort!", TRUE, ch, 0, 0, TO_ROOM);
                send_to_char("You collapse from the effort\n\r", ch);

                rewind(fl);
                fseek(fl, obj->char_f_pos * sizeof(struct char_file_u), 0);
                fwrite(&st, sizeof(struct char_file_u), 1, fl);
                ObjFromCorpse(obj);

                CREATE(newch, struct char_data, 1);
                clear_char(newch);

                origage = GET_AGE(newch);
                origrace = GET_RACE(newch);

                st.race = GetNewRace(&st);

                newrace = GET_RACE(newch);

                store_to_char(&st, newch);

                reset_char(newch);

                newch->next = character_list;
                character_list = newch;

                char_to_room(newch, ch->in_room);
                newch->invis_level = 51;

                set_title(newch);
                GET_HIT(newch) = 1;
                GET_MANA(newch) = 1;
                GET_MOVE(newch) = 1;
                GET_POS(newch) = POSITION_SITTING;

                newage = race_list[newrace].start;
                newrace = GET_RACE(newch);
                age2(newch, &my_age);

                newch->player.time.birth -=
                    my_age.year * SECS_PER_MUD_YEAR * (-1);
                newch->player.time.birth -=
                    SECS_PER_MUD_YEAR * race_list[newrace].start;

                save_char(newch, AUTO_RENT);

                /*
                 * if they are in the descriptor list, suck them into the
                 * game
                 */

                for (d = descriptor_list; d; d = d->next) {
                    if (d->character && (strcmp(GET_NAME(d->character),
                                                GET_NAME(newch)) == 0)) {
                        if (STATE(d) != CON_PLYNG) {
                            free_char(d->character);
                            d->character = newch;
                            STATE(d) = CON_PLYNG;
                            newch->desc = d;
                            send_to_char("You awake to find yourself "
                                         "changed\n\r", newch);
                            break;
                        }
                    }
                }
            } else {
                send_to_char("The spirit does not have the strength to be "
                             "reincarnated\n\r", ch);
            }
            fclose(fl);
        }
    }
}

void spell_charm_veggie(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (!IsVeggie(victim)) {
        send_to_char("This can only be used on plants!\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) > GetMaxLevel(ch) + 10) {
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
        } else {
            if (!IsSusc(victim, IMM_CHARM)) {
                if (saves_spell(victim, SAVING_PARA)) {
                    FailCharm(victim, ch);
                    return;
                }
            }
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);
        af.type = SPELL_CHARM_PERSON;

        if (GET_INT(victim)) {
            af.duration = 24 * GET_CHR(ch) / GET_INT(victim);
        } else {
            af.duration = 24 * 18;
        }
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(victim, &af);

        act("Isn't $n just such a nice fellow?", FALSE, ch, 0, victim, TO_VICT);
    }
}

void spell_veggie_growth(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!IsVeggie(victim)) {
        send_to_char("Thats not a plant-creature!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_VEGGIE_GROWTH)) {
        act("$N is already affected by that spell", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    if (GetMaxLevel(victim) * 2 > GetMaxLevel(ch)) {
        send_to_char("You can't make it more powerful than you!\n\r", ch);
        return;
    }

    if (IS_PC(victim)) {
        send_to_char("It would be in bad taste to cast that on a player\n\r",
                     ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    act("$n grows to double $s original size!", FALSE, victim, 0, 0, TO_ROOM);
    act("You grow to double your original size!", FALSE, victim, 0, 0, TO_CHAR);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = GET_MAX_HIT(victim);
    af.location = APPLY_HIT;
    af.bitvector = AFF_GROWTH;
    affect_to_char(victim, &af);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = 5;
    af.location = APPLY_HITNDAM;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.type = SPELL_VEGGIE_GROWTH;
    af.duration = 2 * level;
    af.modifier = 3;
    af.location = APPLY_SAVE_ALL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    GET_LEVEL(victim, WARRIOR_LEVEL_IND) = 2 * GetMaxLevel(victim);
}


void spell_tree(int level, struct char_data *ch,
                struct char_data *victim, struct obj_data *obj)
{
    struct char_data *mob;
    int             mobn;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    mobn = SAPLING;
    if (level > 20) {
        mobn++;
    }
    if (level > 30) {
        mobn++;
    }
    if (level > 40) {
        mobn++;
    }
    if (level > 48) {
        mobn++;
    }
    mob = read_mobile(mobn, VIRTUAL);
    if (mob) {
        spell_poly_self(level, ch, mob, 0);
    } else {
        send_to_char("You couldn't summon an image of that creature\n\r",
                     ch);
    }
}


void spell_animate_rock(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *mob;
    struct affected_type af;
    int             mobn = LITTLE_ROCK;

    if (ITEM_TYPE(obj) != ITEM_ROCK) {
        send_to_char("Thats not the right kind of rock\n\r", ch);
        return;
    }

    /*
     * get the weight of the rock, make the follower based on the weight
     */

    if (GET_OBJ_WEIGHT(obj) > 20) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 40) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 80) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 160) {
        mobn++;
    }
    if (GET_OBJ_WEIGHT(obj) > 320) {
        mobn++;
    }
    mob = read_mobile(mobn, VIRTUAL);
    if (mob) {
        char_to_room(mob, ch->in_room);
        /*
         * charm them for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);

        af.type = SPELL_ANIMATE_ROCK;
        af.duration = 24;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(mob, &af);

        af.type = SPELL_CHARM_PERSON;

        if (IS_PC(ch) || ch->master) {
            af.duration = 24;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }

        /*
         * get rid of aggressive, add sentinel
         */

        if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
            REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
        }
        if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
            SET_BIT(mob->specials.act, ACT_SENTINEL);
        }

        extract_obj(obj);
    } else {
        send_to_char("Sorry, the spell isn't working today\n\r", ch);
        return;
    }
}

void spell_travelling(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (affected_by_spell(victim, SPELL_TRAVELLING)) {
        return;
    }
    af.type = SPELL_TRAVELLING;
    af.duration = level;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_TRAVELLING;

    affect_to_char(victim, &af);
    act("$n seems fleet of foot", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel fleet of foot.\n\r", victim);
}

void spell_animal_friendship(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (affected_by_spell(ch, SPELL_ANIMAL_FRIENDSHIP)) {
        send_to_char("You can only do this once per day\n\r", ch);
        return;
    }

    if (IS_GOOD(victim) || IS_EVIL(victim)) {
        send_to_char("Only neutral mobs allowed\n\r", ch);
        return;
    }

    if (!IsAnimal(victim)) {
        send_to_char("Thats no animal!\n\r", ch);
        return;
    }

    if (GetMaxLevel(ch) < GetMaxLevel(victim)) {
        send_to_char
            ("You do not have enough willpower to charm that yet\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) > 10 + GetMaxLevel(ch) / 2) {
        send_to_char("That creature is too powerful to charm\n\r", ch);
        return;
    }

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, ch->equipment[WIELD], victim, TO_ROOM);
        return;
    }

    if (IsImmune(victim, IMM_CHARM)) {
        return;
    }

    if (!saves_spell(victim, SAVING_SPELL)) {
        return;
    }
    if (victim->master) {
        stop_follower(victim);
    }
    add_follower(victim, ch);

    af.type = SPELL_ANIMAL_FRIENDSHIP;
    af.duration = 24;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = 0;
    affect_to_char(ch, &af);

    af.type = SPELL_ANIMAL_FRIENDSHIP;
    af.duration = 36;
    af.modifier = 0;
    af.location = APPLY_NONE;
    af.bitvector = AFF_CHARM;
    affect_to_char(victim, &af);

    /*
     * get rid of aggressive, add sentinel
     */
    REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
    SET_BIT(victim->specials.act, ACT_SENTINEL);
}

void spell_invis_to_animals(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (!affected_by_spell(victim, SPELL_INVIS_TO_ANIMALS)) {

        act("$n seems to fade slightly.", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You vanish, sort of.\n\r", victim);

        af.type = SPELL_INVIS_TO_ANIMALS;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_ANIMAL_INVIS;
        affect_to_char(victim, &af);
    }
}

void spell_slow_poison(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (affected_by_spell(victim, SPELL_POISON)) {
        act("$n seems to fade slightly.", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You feel a bit better!.\n\r", victim);

        af.type = SPELL_SLOW_POISON;
        af.duration = 24;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    }
}

void spell_snare(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
        send_to_char("You must cast this spell in the forest!\n\r", ch);
        return;
    }

    /*
     * if victim fails save, movement = 0
     */
    if (!saves_spell(victim, SAVING_SPELL)) {
        act("Roots and vines entangle your feet!", FALSE, victim, 0, 0,
            TO_CHAR);
        act("Roots and vines entangle $n's feet!", FALSE, victim, 0, 0,
            TO_ROOM);
        GET_MOVE(victim) = 0;
    } else {
        FailSnare(victim, ch);
    }
}

void spell_entangle(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    if (real_roomp(ch->in_room)->sector_type != SECT_FOREST) {
        send_to_char("You must cast this spell in the forest!\n\r", ch);
        return;
    }

    if (IsImmune(victim, IMM_HOLD)) {
        FailSnare(victim, ch);
        return;
    }

    if (IsResist(victim, IMM_HOLD) && saves_spell(victim, SAVING_PARA)) {
        FailSnare(victim, ch);
        return;
    }

    /*
     * if victim fails save, paralyzed for a very short time
     */
    if (saves_spell(victim, SAVING_PARA)) {
        if (IsSusc(victim, IMM_HOLD)) {
            if (saves_spell(victim, SAVING_PARA)) {
                FailSnare(victim, ch);
                return;
            }
        } else {
            FailSnare(victim, ch);
            return;
        }
    } else {
        act("Roots and vines entwine around you!", FALSE, victim, 0, 0,
            TO_CHAR);
        act("Roots and vines surround $n!", FALSE, victim, 0, 0, TO_ROOM);

        af.type = SPELL_ENTANGLE;
        af.duration = 1;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_PARALYSIS;
        affect_to_char(victim, &af);
    }
}

void spell_barkskin(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    if (!affected_by_spell(victim, SPELL_BARKSKIN)) {
        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = -10;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = -1;
        af.location = APPLY_SAVE_ALL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_BARKSKIN;
        af.duration = 24;
        af.modifier = 1;
        af.location = APPLY_SAVING_SPELL;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        send_to_char("Your skin takes on a rough, bark-like texture.\n\r",
                     victim);
        act("$n's skin takes on a rough, bark-like texture", FALSE, ch, 0,
            0, TO_ROOM);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void spell_gust_of_wind(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    send_to_char("You wave your hands, and a gust of wind boils forth!\n\r",
                 ch);
    act("$n sends a gust of wind towards you!", FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (GetMaxLevel(tmp_victim) > LOW_IMMORTAL && !IS_NPC(tmp_victim)) {
                return;
            }

            if (!in_group(ch, tmp_victim)) {
                if (!saves_spell(tmp_victim, SAVING_SPELL)) {
                    act("Despite your sudden gusts, $N manages to keep $S "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_CHAR);
                    act("Despite $n's sudden gusts, you manage to keep your "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_VICT);
                    act("Despite $n's sudden gusts, $N manages to keep $S "
                        "footing.", FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                    return;
                }
                act("Your gust of wind makes $N stumble and $E falls on $S "
                    "bum.", FALSE, ch, 0, tmp_victim, TO_CHAR);
                act("$n's gust of wind makes you stumble and you lose your "
                    "footing.", FALSE, ch, 0, tmp_victim, TO_VICT);
                act("$n's gust of wind makes $N stumble and $E falls on $S "
                    "bum.", FALSE, ch, 0, tmp_victim, TO_NOTVICT);
                GET_POS(tmp_victim) = POSITION_SITTING;
            } else {
                act("You are able to avoid the swirling gust.", FALSE, ch,
                    0, tmp_victim, TO_VICT);
            }
        }
    }
}

void spell_silence(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    extern struct index_data *mob_index;

    assert(ch && victim);

    if (!saves_spell(victim, SAVING_SPELL)) {
        act("$n ceases to make noise!", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You can't hear anything!.\n\r", victim);

        af.type = SPELL_SILENCE;
        af.duration = level;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_SILENCE;
        affect_to_char(victim, &af);
    } else {
        send_to_char("You feel quiet for a moment, but the effect fades\n\r",
                     victim);
        if (!IS_PC(victim) && !victim->specials.fighting) {
            set_fighting(victim, ch);
            if (mob_index[victim->nr].func) {
                (*mob_index[victim->nr].func) (victim, 0, "");
            }
        }
    }
}

void spell_warp_weapon(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    assert(ch && (victim || obj));

    if (!obj) {
        if (!victim->equipment[WIELD]) {
            act("$N doesn't have a weapon wielded!", FALSE, ch, 0,
                victim, TO_CHAR);
            return;
        }
        obj = victim->equipment[WIELD];
    }

    act("$p is warped and twisted by the power of the spell", FALSE,
        ch, obj, 0, TO_CHAR);
    act("$p is warped and twisted by the power of the spell", FALSE,
        ch, obj, 0, TO_ROOM);
    DamageOneItem(victim, BLOW_DAMAGE, obj);

    if (!IS_PC(victim) && !victim->specials.fighting) {
        set_fighting(victim, ch);
    }
}

void spell_heat_stuff(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             j = 0;

    assert(victim);

    if (IS_SET(victim->specials.affected_by2, AFF2_HEAT_STUFF)) {
        send_to_char("Already affected\n\r", ch);
        return;
    }

    if (IS_IMMORTAL(victim) && IS_PC(victim)) {
        act("Your magic rebounds from the divine aura surrounding $N!",
            FALSE, ch, 0, victim, TO_CHAR);
        act("$n sends a heat spell towards $N, but it rebounds off $S divine "
            "aura!", FALSE, ch, 0, victim, TO_NOTVICT);
        send_to_char("The heat spell aimed at you is rebounded by your divine"
                     " aura.\n\r", victim);
        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = -2;
        af.location = APPLY_DEX;
        af.bitvector = 0;

        affect_to_char(ch, &af);

        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_HEAT_STUFF;

        affect_to_char(ch, &af);

        if (IS_PC(ch)) {
            /*
             * all metal flagged equips zap off!
             */
            for (j = 0; j < MAX_WEAR; j++) {
                if (ch->equipment[j]) {
                    obj = ch->equipment[j];
                    if (IS_OBJ_STAT(obj, ITEM_METAL)) {
                        act("$p glows brightly from the heat, and you quickly"
                            " let go of it!", FALSE, ch, obj, 0, TO_CHAR);
                        act("$p turns so hot that $n is forced to let go of "
                            "it!", FALSE, ch, obj, 0, TO_ROOM);
                        if ((obj = unequip_char(ch, j)) != NULL) {
                            obj_to_room(obj, ch->in_room);
                        }
                    }
                }
            }
        }
        return;
    }

    if (HitOrMiss(ch, victim, CalcThaco(ch))) {
        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = -2;
        af.location = APPLY_DEX;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_HEAT_STUFF;
        af.duration = level;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_HEAT_STUFF;

        affect_to_char(victim, &af);
        send_to_char("Your armor starts to sizzle and smoke.\n\r", victim);
        act("$N's armor starts to sizzle.", FALSE, ch, 0, victim, TO_CHAR);
        act("$N's armor starts to sizzle.", FALSE, ch, 0, victim, TO_NOTVICT);

        if (IS_PC(victim)) {
            /*
             * all metal flagged equips zap off!
             */
            for (j = 0; j < MAX_WEAR; j++) {
                if (victim->equipment[j]) {
                    obj = victim->equipment[j];
                    if (IS_OBJ_STAT(obj, ITEM_METAL)) {
                        act("$p glows brightly from the heat, and you quickly"
                            " let go of it!", FALSE, victim, obj, 0, TO_CHAR);
                        act("$p turns so hot that $n is forced to let go of "
                            "it!", FALSE, victim, obj, 0, TO_ROOM);
                        if ((obj = unequip_char(victim, j)) != NULL) {
                            obj_to_room(obj, victim->in_room);
                        }
                    }
                }
            }
        } else {
            if (!victim->specials.fighting) {
                set_fighting(victim, ch);
            }
        }
    }
}


void spell_dust_devil(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             vnum;
    struct char_data *mob;
    struct affected_type af;

    if (affected_by_spell(ch, SPELL_DUST_DEVIL)) {
        send_to_char("You can only do this once per 24 hours\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(ch->in_room)->room_flags, INDOORS)) {
        send_to_char("You can't cast this spell indoors!\n\r", ch);
        return;
    }

    vnum = DUST_DEVIL;

    mob = read_mobile(vnum, VIRTUAL);

    if (!mob) {
        send_to_char("None available\n\r", ch);
        return;
    }

    act("$n performs a complicated ritual!", TRUE, ch, 0, 0, TO_ROOM);
    act("You perform the ritual of summoning", TRUE, ch, 0, 0, TO_CHAR);

    char_to_room(mob, ch->in_room);
    act("$n appears through a momentary rift in the ether!",
        FALSE, mob, 0, 0, TO_ROOM);

    /*
     * charm them for a while
     */
    if (mob->master) {
        stop_follower(mob);
    }
    add_follower(mob, ch);

    af.type = SPELL_CHARM_PERSON;

    if (IS_PC(ch) || ch->master) {
        af.duration = 24;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = AFF_CHARM;
        affect_to_char(mob, &af);
    } else {
        SET_BIT(mob->specials.affected_by, AFF_CHARM);
    }

    af.type = SPELL_DUST_DEVIL;
    af.duration = 24;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);

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

void spell_sunray(int level, struct char_data *ch,
                  struct char_data *victim, struct obj_data *obj)
{
    struct char_data *t,
                   *n;
    int             dam = 0,
                    j = 0;

    /*
     * blind all in room
     */
    for (t = real_roomp(ch->in_room)->people; t; t = n) {
        n = t->next_in_room;
        if (!in_group(ch, t) && !IS_IMMORTAL(t)) {
            spell_blindness(level, ch, t, obj);
            /*
             * if not arena, scrap any ANTI-SUN equipment worn by
             * ungroupies
             */
            if (!IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
                for (j = 0; j <= (MAX_WEAR - 1); j++) {
                    if (t->equipment[j] && t->equipment[j]->item_number >= 0 &&
                        IS_SET(t->equipment[j]->obj_flags.extra_flags,
                               ITEM_ANTI_SUN)) {
                        obj = t->equipment[j];
                        act("$n's sunray strikes your $p, causing it to fall "
                            "apart!", FALSE, ch, 0, 0, TO_VICT);
                        act("$n's sunray strikes $N's $p, causing it to fall "
                            "apart!", FALSE, ch, obj, t, TO_NOTVICT);
                        act("Your sunray strikes $N's $p, causing it to fall "
                            "apart!", FALSE, ch, obj, t, TO_CHAR);
                        MakeScrap(t, 0, obj);
                    }
                }
            }

            /*
             * hit undead target
             */
            if (IsUndead(t) || GET_RACE(t) == RACE_VEGMAN) {
                dam = dice(6, 8);
                if (saves_spell(t, SAVING_SPELL) &&
                    GET_RACE(t) != RACE_VEGMAN) {
                    dam = (t == victim ? dam >> 1 : 0);
                }
                damage(ch, t, dam, SPELL_SUNRAY);
            }
        }
    }
}

void spell_know_monster(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    char            buf[256],
                    buf2[256];
    int             exp,
                    lev,
                    hits;

    extern char    *pc_class_types[];
    extern char    *immunity_names[];
    extern char    *RaceName[];
    extern const char *RaceDesc[];
    int             att;
    int             no,
                    s;

    /*
     * depending on level, give info.. sometimes inaccurate
     */

    if (!IS_PC(victim)) {
        sprintf(buf, "$c000W$N$c000p belongs to the $c000W%s$c000p race.",
                RaceName[GET_RACE(victim)]);
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
        if (level > 5) {
            exp = GetApprox(GET_EXP(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p is worth approximately $c000W%d$c000p "
                         "experience", exp);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 10) {
            lev = GetApprox(GetMaxLevel(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p fights like a $c000W%d$c000p level "
                         "warrior, you think", lev);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 15) {
            if (IS_SET(victim->hatefield, HATE_RACE)) {
                sprintf(buf, "$c000W$N$c000p seems to hate the $c000W%s$c000p "
                             "race", RaceName[victim->hates.race]);
                act(buf, FALSE, ch, 0, victim, TO_CHAR);
            }
            if (IS_SET(victim->hatefield, HATE_CLASS)) {
                sprintbit((unsigned) victim->hates.class, pc_class_types, buf2);
                sprintf(buf, "$c000W$N$c000p seems to hate the $c000W%s$c000p "
                             "class(es)", buf2);
                act(buf, FALSE, ch, 0, victim, TO_CHAR);
            }
        }

        if (level > 20) {
            hits = GetApprox(GET_MAX_HIT(victim), 40 + level);
            sprintf(buf, "$c000W$N$c000p probably has about $c000W%d$c000p hit "
                         "points", hits);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 25 && victim->susc) {
            sprintbit(victim->susc, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is susceptible to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 30 && victim->immune) {
            sprintbit(victim->immune, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is resistant to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 35 && victim->M_immune) {
            sprintbit(victim->M_immune, immunity_names, buf2);
            sprintf(buf, "$c000W$N$c000p is immune to $c000W%s$c000p\n\r",
                    buf2);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 40) {
            att = GetApprox((int) victim->mult_att, 30 + level);
            sprintf(buf, "$c000W$N$c000p gets approx $c000W%d.0$c000p "
                    "attack(s) per round", att);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        if (level > 45) {
            no = GetApprox(victim->specials.damnodice, 30 + level);
            s = GetApprox(victim->specials.damsizedice, 30 + level);

            sprintf(buf, "$c000pEach does about $c000W%dd%d$c000p points of "
                         "damage", no, s);
            act(buf, FALSE, ch, 0, victim, TO_CHAR);
        }

        ch_printf(ch, "$c000pDescription:$c000W \n\r%s",
                  RaceDesc[victim->race]);
    } else {
        send_to_char("Thats not a REAL monster\n\r", ch);
        return;
    }
}

void spell_find_traps(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * raise their detect traps skill
     */
    if (affected_by_spell(ch, SPELL_FIND_TRAPS)) {
        send_to_char("You area already searching for traps.\n\r", ch);
        return;
    }

    af.type = SPELL_FIND_TRAPS;
    af.duration = level;
    af.modifier = 50 + level;
    af.location = APPLY_FIND_TRAPS;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void spell_firestorm(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *tmp_victim,
                   *temp;

    assert(ch);
    assert((level >= 1) && (level <= ABS_MAX_LVL));

    dam = dice(2, 8) + level + 1;

    send_to_char("Searing flame surround you!\n\r", ch);
    act("$n sends a firestorm whirling across the room!\n\r",
        FALSE, ch, 0, 0, TO_ROOM);

    for (tmp_victim = real_roomp(ch->in_room)->people; tmp_victim;
         tmp_victim = temp) {
        temp = tmp_victim->next_in_room;
        rdam = dam;
        if ((ch->in_room == tmp_victim->in_room) && (ch != tmp_victim)) {
            if (GetMaxLevel(tmp_victim) > LOW_IMMORTAL && !IS_NPC(tmp_victim)) {
                return;
            }

            if (!in_group(ch, tmp_victim)) {
                act("You are seared by the burning flame!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                if (saves_spell(tmp_victim, SAVING_SPELL)) {
                    rdam >>= 1;
                } else if (!saves_spell(tmp_victim, SAVING_SPELL - 4)) {
                    BurnWings(tmp_victim);
                }
                heat_blind(tmp_victim);
                MissileDamage(ch, tmp_victim, rdam, SPELL_BURNING_HANDS);
            } else {
                act("You are able to avoid the flames!\n\r",
                    FALSE, ch, 0, tmp_victim, TO_VICT);
                heat_blind(tmp_victim);
            }
        }
    }
}

void spell_teleport_wo_error(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    int             location;
    struct room_data *rp;

    /*
     * replaces the current functionality of astral walk
     */
    assert(ch && victim);

    location = victim->in_room;
    rp = real_roomp(location);

    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (GetMaxLevel(victim) > MAX_MORT || !rp ||
        IS_SET(rp->room_flags, PRIVATE) || IS_SET(rp->room_flags, NO_SUM) ||
        IS_SET(rp->room_flags, NO_MAGIC) ||
        (IS_SET(rp->room_flags, TUNNEL) &&
         MobCountInRoom(rp->people) > rp->moblim)) {
        send_to_char("You failed.\n\r", ch);
        return;
    }

    if (!IsOnPmp(location)) {
        send_to_char("That place is on an extra-dimensional plane!\n", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (dice(1, 20) == 20) {
        send_to_char("You fail the magic, and spin out of control!\n\r", ch);
        spell_teleport(level, ch, ch, 0);
        return;
    } else {
        act("$n opens a door to another dimension and steps through!",
            FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, location);
        act("You are blinded for a moment as $n appears in a flash of light!",
            FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, "", 15);
        check_falling(ch);

        if (IS_SET(real_roomp(ch->in_room)->room_flags, DEATH) &&
            GetMaxLevel(ch) < LOW_IMMORTAL) {
            NailThisSucker(ch);
        }
    }
}


void spell_portal(int level, struct char_data *ch,
                  struct char_data *tmp_ch, struct obj_data *obj)
{
    struct obj_data *tmp_obj;
    struct extra_descr_data *ed;
    struct room_data *rp,
                   *nrp;
    char            buf[512];
    char            str[180];

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    /*
     * check target room for legality.
     */
    rp = real_roomp(ch->in_room);
    tmp_obj = read_object(PORTAL, VIRTUAL);
    if (!rp || !tmp_obj) {
        send_to_char("The magic fails\n\r", ch);
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
        send_to_char("There is no room in here to portal!\n\r", ch);
        return;
    }

    if (!(nrp = real_roomp(tmp_ch->in_room))) {
        sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
        Log(str);
        send_to_char("The magic cannot locate the target.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(tmp_ch->in_room)->room_flags, NO_SUM)) {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (!IsOnPmp(tmp_ch->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
        send_to_char("The planes are fuzzy, you cannot portal!\n", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_LINKDEAD(tmp_ch)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_IMMORTAL(tmp_ch)) {
        send_to_char("You can't portal to someone of that magnitude!\n\r", ch);
        return;
    }

    sprintf(buf, "Through the mists of the portal, you can faintly see %s",
            nrp->name);

    CREATE(ed, struct extra_descr_data, 1);
    ed->next = tmp_obj->ex_description;
    tmp_obj->ex_description = ed;
    CREATE(ed->keyword, char, strlen(tmp_obj->name) + 1);
    strcpy(ed->keyword, tmp_obj->name);
    ed->description = strdup(buf);

    tmp_obj->obj_flags.value[0] = level / 5;
    tmp_obj->obj_flags.value[1] = tmp_ch->in_room;

    obj_to_room(tmp_obj, ch->in_room);

    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_ROOM);
    act("$p suddenly appears.", TRUE, ch, tmp_obj, 0, TO_CHAR);
}

void spell_mount(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct char_data *m;
    int             mnr;

    /*
     * create a ridable mount, and automatically mount said creature
     */

    mnr = MOUNT_ONE;
    if (level < 30) {
        if (level < 12) {
            mnr++;
        }
        if (level < 18) {
            mnr++;
        }
        if (level < 24) {
            mnr++;
        }
    } else {
        if (IS_EVIL(ch)) {
            mnr = MOUNT_EVIL;
        } else if (IS_GOOD(ch)) {
            mnr = MOUNT_GOOD;
        } else {
            mnr = MOUNT_NEUT;
        }
    }

    m = read_mobile(mnr, VIRTUAL);
    if (m) {
        char_to_room(m, ch->in_room);
        act("In a flash of light, $N appears", FALSE, ch, 0, m, TO_CHAR);
        act("In a flash of light, $N appears, and $n hops on $s back",
            FALSE, ch, 0, m, TO_ROOM);
        send_to_char("You hop on your mount's back\n\r", ch);
        MOUNTED(ch) = m;
        RIDDEN(m) = ch;
        GET_POS(ch) = POSITION_MOUNTED;
    } else {
        send_to_char("horses aren't in database\n\r", ch);
        return;
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

void spell_enlightenment(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(victim, SPELL_ENLIGHTENMENT) ||
        !saves_spell(victim, SAVING_SPELL)) {

        if (affected_by_spell(victim, SPELL_FEEBLEMIND)) {
            send_to_char("They have already been enlightened!\n\r", ch);
            return;
        }

        send_to_char("You feel enlightened by the gods\n\r", victim);

        af.type = SPELL_ENLIGHTENMENT;
        af.duration = 24;
        af.modifier = 2;
        af.location = APPLY_INT;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        af.type = SPELL_ENLIGHTENMENT;
        af.duration = 24;
        af.modifier = 2;
        af.location = APPLY_WIS;
        af.bitvector = 0;
        affect_to_char(victim, &af);
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

void spell_pacifism(int level, struct char_data *ch,
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
                send_to_char("You feel peace and harmony surrounding you.\n\r",
                             ch);
            }
        } else {
            send_to_char("You feel at peace with the universe.\n\r", victim);
        }
    } else {
        send_to_char("You feel at peace with the universe.\n\r", victim);
    }
}

void spell_aura_power(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * +2 to hit +2 dam
     */

    if (affected_by_spell(victim, SPELL_AURA_POWER)) {
        send_to_char("You already feel an aura of power surrounding you.\n\r",
                     ch);
        return;
    }

    act("$n suddenly has a godly aura surrounding $m.", FALSE, victim, 0,
        0, TO_ROOM);
    send_to_char("You suddenly feel a godly aura surrounding you.\n\r", victim);

    af.type = SPELL_AURA_POWER;
    af.duration = 10;
    af.modifier = 2;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);

    af.location = APPLY_DAMROLL;
    af.modifier = 2;
    affect_to_char(victim, &af);
}

void spell_holy_strength(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);

    if (!affected_by_spell(victim, SPELL_HOLY_STRENGTH)) {
        act("The spirits of gods make you stronger.", FALSE, victim, 0, 0,
            TO_CHAR);
        act("$n's muscles seem to expand!", FALSE, victim, 0, 0, TO_ROOM);

        af.type = SPELL_HOLY_STRENGTH;
        af.duration = 2 * level;
        if (IS_NPC(victim)) {
            if (level >= CREATOR) {
                af.modifier = 25 - GET_STR(victim);
            } else {
                af.modifier = number(1, 6);
            }
        } else {
            if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_BARBARIAN)) {
                af.modifier = number(1, 8);
            } else if (HasClass(ch, CLASS_CLERIC)
                       || HasClass(ch, CLASS_THIEF)) {
                af.modifier = number(1, 6);
            } else {
                af.modifier = number(1, 4);
            }
        }
        af.location = APPLY_STR;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void spell_holy_armor(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_HOLY_ARMOR)) {
        af.type = SPELL_HOLY_ARMOR;
        af.duration = 24;
        af.modifier = -20;
        af.location = APPLY_AC;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        send_to_char("You feel the spirits of gods protecting you.\n\r",
                     victim);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void spell_giant_growth(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    /*
     * +3 to hit +3 dam
     */

    if (affected_by_spell(victim, SPELL_GIANT_GROWTH)) {
        send_to_char("Already in effect\n\r", ch);
        return;
    }

    GET_HIT(victim) += number(1, 8);

    update_pos(victim);

    act("$n grows in size.", FALSE, victim, 0, 0, TO_ROOM);
    send_to_char("You feel larger!\n\r", victim);

    af.type = SPELL_GIANT_GROWTH;
    af.duration = 10;
    af.modifier = 3;
    af.location = APPLY_HITROLL;
    af.bitvector = 0;
    affect_to_char(victim, &af);
    af.location = APPLY_DAMROLL;
    af.modifier = 3;
    affect_to_char(victim, &af);
}

/*
 * Necromancer Spells
 */
void spell_cold_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    /*
     * creates a cold light .
     */
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = read_object(COLD_LIGHT, VIRTUAL);
    if (tmp_obj) {
        tmp_obj->obj_flags.value[2] = 24 + level;
        obj_to_char(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the cold ball of ice.\n\r", ch);
        return;
    }

    act("$n thrusts $s hands through the ground, and pulls out $p.", TRUE,
        ch, tmp_obj, 0, TO_ROOM);
    act("You thrust your hands through the ground, and pull out $p.", TRUE,
        ch, tmp_obj, 0, TO_CHAR);
}

void spell_disease(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             dam = 0;

    assert(victim && ch);

    if (victim == ch) {
        send_to_char("Disease yourself? Sick mind!\n\r", ch);
        return;
    }

    if (IS_PC(victim) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        send_to_char("Sorry. No can do.\n\r", ch);
        return;
    }

    if (IS_SET(victim->M_immune, IMM_POISON)) {
        send_to_char("Unknown forces prevent your body from the disease!\n\r",
                     victim);
        send_to_char("Your quarry seems to resist your attempt to disease "
                     "it!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_DISEASE)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    if (!ImpSaveSpell(victim, SAVING_PARA, -6)) {
        dam = dice(1, 8);

        af.type = SPELL_DISEASE;
        af.duration = 12;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        affect_to_char(victim, &af);
        act("$n places a rotting hand to $N's head, spreading disease.",
            FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n places a rotting hand to your head, spreading disease.",
            FALSE, ch, 0, victim, TO_VICT);
        act("You place a rotting hand to $N's head, spreading disease.",
            FALSE, ch, 0, victim, TO_CHAR);

        GET_HIT(victim) -= dam;

        if (!victim->specials.fighting && !IS_PC(victim)) {
            AddHated(victim, ch);
            set_fighting(victim, ch);
        }
    } else {
        send_to_char("A slight feeling of illness overwhelms you, but you "
                     "recover.\n\r", victim);
        send_to_char("Your quarry seems to resist your disease.\n\r", ch);
    }
}

void spell_life_tap(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    dicen = 1;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("Tap your own life? That could be a bad idea.\n\r", ch);
        return;
    }

    if (IS_PC(victim) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        send_to_char("I think not. Go pester monsters instead.\n\r", ch);
        return;
    }

    if (IsUndead(victim)) {
        act("But $N doesn't have any life in $M!", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    dicen = (int) level / 10 + 2;
    /*
     * avg 12.5 | max 20 | min 5
     */
    dam = dice(dicen, 5) + 1;
    damage(ch, victim, dam, SPELL_LIFE_TAP);

    if (IsResist(victim, IMM_DRAIN)) {
        dam >>= 1;
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        dam >>= 1;
    }
    if (IsImmune(victim, IMM_DRAIN)) {
        dam = 0;
    }
    GET_HIT(ch) += dam;
}

void spell_invis_to_undead(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (!affected_by_spell(victim, SPELL_INVIS_TO_UNDEAD)) {
        act("The undead no longer see $n.", TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("The undead no longer see you.\n\r", victim);

        af.type = SPELL_INVIS_TO_UNDEAD;
        af.duration = 24;
        af.modifier = 0;
        af.location = APPLY_BV2;
        af.bitvector = AFF2_INVIS_TO_UNDEAD;
        affect_to_char(victim, &af);
    }
}

void spell_suit_of_bone(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim != ch) {
        send_to_char("You can only cast this spell upon yourself.\n\r", ch);
        return;
    }

    if (!affected_by_spell(victim, SPELL_SUIT_OF_BONE)) {
        af.type = SPELL_SUIT_OF_BONE;
        af.duration = 24;
        af.modifier = -20;
        af.location = APPLY_AC;
        af.bitvector = 0;

        affect_to_char(victim, &af);
        send_to_char("Bones start forming around your armor, making it "
                     "stronger than ever.\n\r", victim);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void spell_spectral_shield(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(victim && ch);

    if (victim != ch) {
        send_to_char("You can only cast this spell upon yourself.\n\r", ch);
        return;
    }

    if (!affected_by_spell(victim, SPELL_SPECTRAL_SHIELD)) {
        act("A suit of battlescarred armor surrounds $N.", TRUE, ch, 0,
            victim, TO_NOTVICT);
        if (ch != victim) {
            act("A suit of battlescarred armor surrounds $N.", TRUE, ch, 0,
                victim, TO_CHAR);
            act("A suit of battlescarred armor surrounds you.", TRUE, ch,
                0, victim, TO_VICT);
        } else {
            act("A suit of battlescarred armor surrounds you.", TRUE, ch,
                0, 0, TO_CHAR);
        }

        af.type = SPELL_SPECTRAL_SHIELD;
        af.duration = 8 + level;
        af.modifier = -10;
        af.location = APPLY_AC;
        af.bitvector = 0;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing new seems to happen\n\r", ch);
    }
}

void spell_clinging_darkness(int level, struct char_data *ch,
                             struct char_data *victim,
                             struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("That won't help much.\n\r", ch);
        return;
    }

    if (IS_PC(victim) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        send_to_char("I think not. Go pester monsters instead.\n\r", ch);
        return;
    }

    if (IS_AFFECTED(victim, SPELL_BLINDNESS)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    if (saves_spell(victim, SAVING_SPELL)) {
        act("$n's eyes briefly turn dark, then return to normal.", FALSE,
            victim, 0, 0, TO_ROOM);
        send_to_char("A gulf of darkness threatens do blind you, but you "
                     "withstand the spell.\n\r", victim);
        return;
    }

    act("Darkness seems to overcome $n's eyes!", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("The darkness blinds you!!\n\r", victim);

    af.type = SPELL_CLINGING_DARKNESS;
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
void spell_dominate_undead(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("You like yourself even better!\n\r", ch);
        return;
    }

    if (!IsUndead(victim)) {
        send_to_char("That's not an undead creature!\n\r", ch);
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

        if (saves_spell(victim, SAVING_PARA)) {
            FailCharm(victim, ch);
            return;
        }

        if (victim->master) {
            stop_follower(victim);
        }
        add_follower(victim, ch);

        af.type = SPELL_DOMINATE_UNDEAD;

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

        act("$n's glare draws you towards $m.", FALSE, ch, 0, victim, TO_VICT);
        act("$N seems attracted by $n's aura and starts to follow $m.",
            FALSE, ch, 0, victim, TO_ROOM);
        act("$N seems attracted by your aura and starts to follow you.",
            FALSE, ch, 0, victim, TO_CHAR);

        if (!IS_PC(ch)) {
            REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
            SET_BIT(victim->specials.act, ACT_SENTINEL);
        }
    }
}

void spell_unsummon(int level, struct char_data *ch,
                    struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    if (victim->master == ch && IsUndead(victim) && IS_NPC(victim)) {
        act("$n points at $N who then crumbles to the ground.", FALSE, ch,
            0, victim, TO_ROOM);
        send_to_char("You point at the undead creature, and then it suddenly "
                     "crumbles to the ground.\n\r", ch);

        healpoints = dice(3, 8) + 3;

        if ((healpoints + GET_HIT(ch)) > hit_limit(ch)) {
            GET_HIT(ch) = hit_limit(ch);
        } else {
            GET_HIT(ch) += healpoints;
        }
        /* added +5 to make up for cast cost -Lennya */
        healpoints = dice(3, 8) + 3 + 5;

        if ((healpoints + GET_MANA(ch)) > mana_limit(ch)) {
            GET_MANA(ch) = mana_limit(ch);
        } else {
            GET_MANA(ch) += healpoints;
        }
        GET_HIT(victim) = -1;
        die(victim, '\0');

    } else {
        send_to_char("You can't unsummon other people's dead or people that "
                     "are still alive.\n\r", ch);
    }
}

void spell_siphon_strength(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
#if 0
    struct room_data *rp;
#endif
    int             mod = 0;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("Siphon your own strength? What a waste of mana.\n\r", ch);
        return;
    }

    if (IS_PC(victim) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        send_to_char("I think not. Go pester monsters instead.\n\r", ch);
        return;
    }

    if (!affected_by_spell(ch, SPELL_SIPHON_STRENGTH)) {
        if (!saves_spell(victim, SAVING_SPELL)) {
#if 0
            modifier = level/200.0;
#endif
            mod = dice(1, 5);
            act("You feel your strength being siphoned from your body.",
                FALSE, victim, 0, 0, TO_VICT);
            act("$n staggers as some of his strength is siphoned away.",
                FALSE, victim, 0, 0, TO_ROOM);

            af.type = SPELL_SIPHON_STRENGTH;
            af.duration = 3;
#if 0
            mod= victim->abilities.str * modifier;
#endif
            af.modifier = 0 - mod;
            if (victim->abilities.str_add) {
                af.modifier -= 2;
                mod += 2;
            }
            af.location = APPLY_STR;
            af.bitvector = 0;

            affect_to_char(victim, &af);

            act("You feel your muscles becoming engorged.", FALSE, ch, 0,
                0, TO_CHAR);
            act("$n's muscles suddenly become engorged!", FALSE, ch, 0, 0,
                TO_ROOM);

            af.type = SPELL_SIPHON_STRENGTH;
            af.duration = 8;
            af.modifier = mod;
            af.location = APPLY_STR;
            af.bitvector = 0;
            affect_to_char(ch, &af);

            /*
             * aggressive act
             */
            if (!victim->specials.fighting && (victim != ch)) {
                set_fighting(victim, ch);
            }
        } else {
            /*
             * made save
             */
            send_to_char("Your quarry withstands your spell.\n\r", ch);
        }
    } else {
        send_to_char("I'm not sure if your muscles can stand such power.\n\r",
                     ch);
    }
}

void spell_gather_shadows(int level, struct char_data *ch,
                          struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert((ch && obj) || victim);

    if (obj) {
        send_to_char("The shadows start to encase it, but nothing happens.",
                     ch);
    } else if (!affected_by_spell(victim, SPELL_INVISIBLE) &&
               !affected_by_spell(victim, SPELL_GATHER_SHADOWS)) {
        act("$n gathers the shadows around $m and slowly fades from view.",
            TRUE, victim, 0, 0, TO_ROOM);
        send_to_char("You gather shadows around you and blend with them.\n\r",
                     victim);

        af.type = SPELL_GATHER_SHADOWS;
        af.duration = 24;
        af.modifier = -40;
        af.location = APPLY_AC;
        af.bitvector = AFF_INVISIBLE;
        affect_to_char(victim, &af);
    } else {
        send_to_char("Nothing seems to happen.\n\r", victim);
    }
}

void spell_mend_bones(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             healpoints;

    assert(victim);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (!IsUndead(victim)) {
        send_to_char("They seem alive and well.. Why would you?\n\r", ch);
        return;
    }

    healpoints = dice(3, 8) + 3;

    if ((healpoints + GET_HIT(victim)) > hit_limit(victim)) {
        GET_HIT(victim) = hit_limit(victim);
    } else {
        GET_HIT(victim) += healpoints;
    }
    send_to_char("You expertly mend the undead's bones, making it "
                 "healthier.\n\r", ch);

    send_to_char("Your bandages seem to tighten up!\n\r", victim);

    update_pos(victim);
}

void spell_trace_corpse(int level, struct char_data *ch,
                        struct char_data *victim, char *arg)
{
    struct obj_data *i,
                   *corpse = NULL;
    char            name[255];
    char            buf[255];
    int             j = 0,
                    found = 0,
                    player_loc = 0,
                    corpse_loc = 0;

    assert(ch);
    sprintf(name, "%s", arg);
    buf[0] = '\0';

    /*
     * when starting out, no corpse has been found yet
     */
    found = 0;
    send_to_char("You open your senses to recent sites of death.\n\r", ch);

    for (i = object_list; i && !found; i = i->next) {
        if (isname(name, i->name) && i->obj_flags.value[3]) {
            found = 1;
            /*
             * we found a REAL corpse
             */
            if (i->carried_by && strlen(PERS_LOC(i->carried_by, ch))) {
                sprintf(buf, "You sense %s being carried by %s.\n\r",
                        i->short_description, PERS(i->carried_by, ch));
                j = 1;
            } else if (i->equipped_by && strlen(PERS_LOC(i->equipped_by, ch))) {
                sprintf(buf, "You sense %s, equipped by %s.\n\r",
                        i->short_description, PERS(i->equipped_by, ch));
                j = 2;
            } else if (i->in_obj) {
                sprintf(buf, "You sense %s in %s.\n\r",
                        i->short_description, i->in_obj->short_description);
                /*
                 * can't trace corpses in objects
                 */
            } else {
                sprintf(buf, "You sense %s in %s.\n\r",
                        i->short_description,
                        (i->in_room == NOWHERE ? "use but uncertain." :
                         real_roomp(i->in_room)->name));
                j = 3;
            }
            corpse = i;
        }
    }
    send_to_char(buf, ch);

    if (!found) {
        send_to_char("Your senses could not pick up traces of this specific "
                     "corpse.\n\r", ch);
        act("Black spots float across $n's eyes. Then $e blinks and sighs.",
            FALSE, ch, 0, 0, TO_ROOM);
    } else if (j == 0) {
        send_to_char("You realize that tracing down this corpse is futile.\n\r",
                     ch);
        act("Black spots float across $n's eyes. Then $e blinks and sighs.",
            FALSE, ch, 0, 0, TO_ROOM);
    } else {
        /*
         * here goes the real corpse trace. Kinda like scry.
         */
        if (ch->in_room) {
            player_loc = ch->in_room;
        }
        if (j == 1) {
            if ((corpse->carried_by)->in_room) {
                corpse_loc = (corpse->carried_by)->in_room;
            } else {
                send_to_char("Alas, you cannot view this corpse's "
                             "location.\n\r", ch);
                act("Black spots float across $n's eyes. Then $e blinks and "
                    "sighs..", FALSE, ch, 0, 0, TO_ROOM);
                return;
            }
        } else if (j == 2) {
            /*
             * equipped corpses? oh well. What a weirdo.
             */
            if ((corpse->equipped_by)->in_room) {
                corpse_loc = (corpse->equipped_by)->in_room;
            } else {
                send_to_char("Alas, you cannot view this corpse's "
                             "location.\n\r", ch);
                act("Black spots float across $n's eyes. Then $e blinks and "
                    "sighs..", FALSE, ch, 0, 0, TO_ROOM);
                return;
            }
        } else if (j == 3) {
            if (corpse->in_room) {
                corpse_loc = corpse->in_room;
            } else {
                send_to_char("Alas, you cannot view this corpse's "
                             "location.\n\r", ch);
                act("Black spots float across $n's eyes. Then $e blinks and "
                    "sighs..", FALSE, ch, 0, 0, TO_ROOM);
                return;
            }
        }

        send_to_char("\n\rYou focus your mental eye on the scene of decay.\n\r",
                     ch);
        act("Black spots float across $n's eyes, as $e loses $mself for a "
            "minute,", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, corpse_loc);
        do_look(ch, "", 15);
        char_from_room(ch);
        char_to_room(ch, player_loc);
    }
}

void spell_endure_cold(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    char            buf[254];

    if (!victim) {
        return;
    }
    if (!affected_by_spell(victim, SPELL_ENDURE_COLD)) {
        if (ch != victim) {
            act("$n points at $N, and then $S hands turn a pale blue color.",
                FALSE, ch, 0, victim, TO_NOTVICT);
            act("You point at $N, and make $S skin a pale blue color.",
                FALSE, ch, 0, victim, TO_CHAR);
            act("$n points at you.  Suddenly, you feel a cold chill "
                "throughout your entire body.", FALSE, ch, 0, victim, TO_VICT);
        } else {
            act("$n concentrates for a second, then $s skin turns a weird pale "
                "blue.", FALSE, ch, 0, victim, TO_NOTVICT);
            act("You concentrate for a second, then all of a sudden, you feel a"
                " cold chill overtake your body.", FALSE, ch, 0, victim,
                TO_CHAR);
        }

        af.type = SPELL_ENDURE_COLD;
        af.modifier = IMM_COLD;
        af.location = APPLY_IMMUNE;
        af.bitvector = 0;
        af.duration = level / 5;
        affect_to_char(victim, &af);
    } else {
        if (ch != victim) {
            sprintf(buf, "$N already seems to be able to endure the cold.");
        } else {
            sprintf(buf, "You already have the ability to endure the cold.");
        }
        act(buf, FALSE, ch, 0, victim, TO_CHAR);
    }
}

void spell_life_draw(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    dicen = 1;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("Draw your own life? That could be a bad idea.\n\r", ch);
        return;
    }

    if (IsUndead(victim)) {
        act("But $N doesn't have any life in $M!", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    dicen = (int) level / 10 + 3;
    /*
     * avg 22.5 | max 35 | min 10
     */
    dam = dice(dicen, 6) + 3;
    damage(ch, victim, dam, SPELL_LIFE_DRAW);

    if (IsResist(victim, IMM_DRAIN)) {
        dam >>= 1;
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        dam >>= 1;
    }
    if (IsImmune(victim, IMM_DRAIN)) {
        dam = 0;
    }
    GET_HIT(ch) += dam;
}

void spell_numb_dead(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    int             done = 0;

    assert(ch && victim);

    /*
     * removes (meta)aggressive bit from undead
     */
    if (IS_PC(victim)) {
        send_to_char("You can only cast this spell on monsters.\n\r", ch);
        return;
    }

    if (!IsUndead(victim)) {
        send_to_char("That's not a true undead creature!\n\r", ch);
        return;
    }

    if (IS_SET(victim->specials.act, ACT_AGGRESSIVE) &&
        HitOrMiss(ch, victim, CalcThaco(ch))) {
        REMOVE_BIT(victim->specials.act, ACT_AGGRESSIVE);
        done = 1;
    }

    if (IS_SET(victim->specials.act, ACT_META_AGG) &&
        HitOrMiss(ch, victim, CalcThaco(ch))) {
        REMOVE_BIT(victim->specials.act, ACT_META_AGG);
        done = 1;
    }

    if (done) {
        act("$n stares $N in the eyes, who then becomes docile.", FALSE,
            ch, 0, victim, TO_NOTVICT);
        act("$n stares you in the eyes, and you feel cowed.", FALSE, ch, 0,
            victim, TO_VICT);
        act("You stare at $N, who then becomes docile.", FALSE, ch, 0,
            victim, TO_CHAR);
    } else {
        send_to_char("Nothing seems to happen.\n\r", ch);
    }
}

void spell_binding(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    int             location;

    assert(ch);

    if (IS_NPC(ch)) {
        return;
    }
    if (ch->player.hometown) {
        location = ch->player.hometown;
    } else {
        location = 3001;
    }

    if (!real_roomp(location)) {
        send_to_char("You are completely lost.\n\r", ch);
        location = 0;
        return;
    }

    if (ch->specials.fighting) {
        send_to_char("HAH, not in a fight!\n\r", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("Your binding spot seems to be on a different plane.\n\r",
                     ch);
        return;
    }

    /*
     * a location has been found.
     */

    act("$n seems to biodegrade into nothingness, leaving only the stench of "
        "decay.", TRUE, ch, 0, 0, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, location);
    act("A smell of death and decay wafts by as $n emerges from nothingness.",
        TRUE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", 15);
}

void spell_decay(int level, struct char_data *ch,
                 struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             dam = 0;

    assert(victim && ch);

    if (IS_SET(victim->M_immune, IMM_POISON)) {
        send_to_char("Unknown forces prevent your body from decay!\n\r",
                     victim);
        send_to_char("Your quarry seems to resist your decay!\n\r", ch);
        return;
    }

    if (affected_by_spell(victim, SPELL_DECAY)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    if (!ImpSaveSpell(victim, SAVING_PARA, -4)) {
        dam = dice(4, 5);

        af.type = SPELL_DECAY;
        af.duration = 4;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;
        affect_to_char(victim, &af);

        act("$n's death touch causes $N's body to show signs of decay.",
            FALSE, ch, 0, victim, TO_NOTVICT);
        act("$n's death touch causes your body to show signs of decay.",
            FALSE, ch, 0, victim, TO_VICT);
        act("Your death touch causes $N's body to show signs of decay.",
            FALSE, ch, 0, victim, TO_CHAR);

        GET_HIT(victim) -= dam;

        if (!victim->specials.fighting && !IS_PC(victim)) {
            AddHated(victim, ch);
            set_fighting(victim, ch);
        }
    } else {
        send_to_char("A slight waft of decay overwhelms you, but you "
                     "recover.\n\r", victim);
        send_to_char("Your quarry seems to resist your decay!\n\r", ch);
    }
}

void spell_shadow_step(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    int             attempt = 0,
                    i = 0,
                    nroomnr = 0;

    /*
     * gotta have someone fightin ya
     */
    if (!ch->specials.fighting) {
        send_to_char("The shadows will only aid you in battle.\n\r", ch);
        return;
    }

    for (i = 0; i <= 5; i++) {  /* give em lots of tries */
        attempt = number(0, 5);
        if (CAN_GO(ch, attempt) &&
            !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                    DEATH) &&
            !IS_SET(real_roomp(EXIT(ch, attempt)->to_room)->room_flags,
                    NO_FLEE)) {
            act("$n steps into a nearby shadow and seems to dissipate.",
                FALSE, ch, 0, 0, TO_ROOM);
            send_to_char("You shift into a nearby shadow, and let it carry you"
                         " away.\n\r\n\r", ch);

            if (ch->specials.fighting->specials.fighting == ch) {
                stop_fighting(ch->specials.fighting);
            }
            if (ch->specials.fighting) {
                stop_fighting(ch);
            }
            nroomnr = (EXIT(ch, attempt)->to_room);

            char_from_room(ch);
            char_to_room(ch, nroomnr);
#if 0
            EXIT(ch, attempt)->to_room);
#endif
            do_look(ch, "\0", 15);
            return;
        }
    }

    send_to_char("The shadows in the vicinity didn't serve your purpose.\n\r",
                 ch);
}

void spell_cavorting_bones(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct room_data *rp;
    struct char_data *mob;
    int             lev = 1,
                    mlev,
                    mhps,
                    mtohit;

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (!ch) {
        Log("screw up in cavorting bones, no caster found");
        return;
    }

    if (!obj) {
        send_to_char("Which bones would you like to see cavorting?\n\r", ch);
        return;
    }

    if (!IS_CORPSE(obj)) {
        send_to_char("That's not a corpse.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There's no room to cavort those bones.\n\r", ch);
        return;
    }

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to animate "
                     "corpses!\n\r", ch);
        return;
    }

    act("$n stares at $p while mumbling some arcane incantations.", TRUE,
        ch, obj, 0, TO_ROOM);
    act("You stare at $p while phrasing some arcane incantations.", TRUE,
        ch, obj, 0, TO_CHAR);
    lev = GetMaxLevel(ch);
    mob = read_mobile(CB_TEMPLATE, VIRTUAL);

    if (!mob) {
        Log("No template found for cavorting bones spells (no mobile with "
            "vnum 4)");
        send_to_char("Screw up in this spell, no cavorting bones template "
                     "found\n\r", ch);
        return;
    }

    if (lev > 45) {
        mlev = number(30, 40);
        mhps = number(70, 100);
        mtohit = 8;
    } else if (lev > 37) {
        mlev = number(25, 35);
        mhps = number(50, 80);
        mtohit = 6;
    } else if (lev > 29) {
        mlev = number(20, 29);
        mhps = number(30, 60);
        mtohit = 4;
    } else if (lev > 21) {
        mlev = number(14, 20);
        mhps = number(10, 40);
        mtohit = 2;
    } else {
        mlev = number(4, 6);
        mhps = number(0, 20);
        mtohit = 0;
    }

    mob->player.level[2] = mlev;
    mob->points.max_hit = mob->points.max_hit + mhps;
    mob->points.hit = mob->points.max_hit;
    mob->points.hitroll = mob->points.hitroll + mtohit;
    char_to_room(mob, ch->in_room);
    extract_obj(obj);

    act("The corpse starts stirring, and rises as $n.", FALSE, mob, obj, 0,
        TO_ROOM);

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, mob, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, 0, mob, TO_NOTVICT);
        act("You take one look at the size of $n's posse and just say no!",
            TRUE, ch, 0, mob, TO_VICT);
    } else {
        /*
         * charm it for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);
        af.type = SPELL_CHARM_PERSON;
        if (IS_PC(ch) || ch->master) {
            af.duration = GET_CHR(ch);
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }
    }
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
    }
}

void spell_mist_of_death(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    rdam;
    struct char_data *t = NULL,
                   *next;

    assert(ch);
    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    act("$c0008Raising $s arms, a terrifying scream escapes $n and a cloud of"
        " death fills the air!", FALSE, ch, 0, t, TO_ROOM);
    act("$c0008Raising your arms, a terrifying scream escapes you and a cloud"
        " of death fills the air!", FALSE, ch, 0, t, TO_CHAR);

    dam = dice(level, 7);
    for (t = real_roomp(ch->in_room)->people; t; t = next) {
        next = t->next_in_room;
        rdam = dam;
        if (!in_group(ch, t) && t != victim && !IS_IMMORTAL(t)) {
            /*
             * 1% chance of instakill
             */
            if (number(1, 100) == 100 && !IS_IMMUNE(t, IMM_DRAIN)) {
                /*
                 * woop, instant death is cool
                 */
                rdam = GET_HIT(t) + 25;
                damage(ch, t, rdam, SPELL_MIST_OF_DEATH);
            } else {
                if (saves_spell(t, SAVING_PETRI)) {
                    /*
                     * save for half damage
                     */
                    rdam >>= 1;
                }
                damage(ch, t, rdam, SPELL_MIST_OF_DEATH);
            }
        }
    }
}

void spell_nullify(int level, struct char_data *ch,
                   struct char_data *victim, struct obj_data *obj)
{
    assert(ch && victim);

    if (affected_by_spell(victim, SPELL_POISON) ||
        affected_by_spell(victim, SPELL_DISEASE) ||
        affected_by_spell(victim, SPELL_DECAY)) {
        if (GetMaxLevel(victim) <= GetMaxLevel(ch) ||
            !saves_spell(victim, SAVING_SPELL)) {
            if (affected_by_spell(victim, SPELL_POISON)) {
                affect_from_char(victim, SPELL_POISON);
            }
            if (affected_by_spell(victim, SPELL_DISEASE)) {
                affect_from_char(victim, SPELL_DISEASE);
            }
            if (affected_by_spell(victim, SPELL_DECAY)) {
                affect_from_char(victim, SPELL_DECAY);
            }
            act("A warm feeling runs through your body.", FALSE, victim, 0,
                0, TO_CHAR);
            act("$N looks better.", FALSE, ch, 0, victim, TO_ROOM);
        } else {
            act("Nothing seems to happen.", FALSE, victim, 0, 0, TO_CHAR);
            act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
        }
    } else {
        act("Nothing seems to happen.", FALSE, victim, 0, 0, TO_CHAR);
        act("Nothing seems to happen.", FALSE, ch, 0, 0, TO_CHAR);
    }
}

void spell_dark_empathy(int level, struct char_data *ch,
                        struct char_data *victim, struct obj_data *obj)
{
    int             amnt = 0;
    assert(ch && victim);

    if (ch == victim) {
        send_to_char("You cannot cast this spell upon yourself.\n\r", ch);
        return;
    }

    if (GET_HIT(ch) < 66) {
        send_to_char("Your corporeal state cannot bear to degrade any "
                     "further.\n\r", ch);
        return;
    }

    if (GET_HIT(victim) >= GET_MAX_HIT(victim)) {
        act("But $N seems to be in excellent condition!", FALSE, ch, 0,
            victim, TO_CHAR);
        return;
    }

    if (GET_ALIGNMENT(victim) >= 600) {
        act("The shimmering white aura around $N repels your aid.", FALSE,
            ch, 0, victim, TO_CHAR);
        return;
    }

    amnt = GET_MAX_HIT(victim) - GET_HIT(victim);
    if (amnt <= 100) {
        /*
         * itsy bitsy heal
         */
        act("$n's hand glows blue as $e places it on your shoulder.",
            FALSE, ch, 0, victim, TO_VICT);
        act("You come back to perfect health, while $n's hand grows colder.",
            FALSE, ch, 0, victim, TO_VICT);
        act("$N's condition turns pristine as $n places $s glowing blue hand "
            "on $S shoulder.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("Your hand glows with an eerie blue light as you place it on $N's "
            "shoulder,", FALSE, ch, 0, victim, TO_CHAR);
        act("You drain yourself of some essence, reinfusing $N.", FALSE,
            ch, 0, victim, TO_CHAR);
        GET_HIT(victim) += amnt;
        amnt >>= 1;
        GET_HIT(ch) -= amnt;
    } else {
        /*
         * large chunk heal
         */
        act("$n's hand glows blue as $e places it on your shoulder.",
            FALSE, ch, 0, victim, TO_VICT);
        act("You feel better, while $n's hand grows colder.", FALSE, ch, 0,
            victim, TO_VICT);
        act("$N's condition improves as $n places $s glowing blue hand on $S "
            "shoulder.", FALSE, ch, 0, victim, TO_NOTVICT);
        act("Your hand glows with an eerie blue light as you place it on $N's "
            "shoulder,", FALSE, ch, 0, victim, TO_CHAR);
        act("You drain yourself of essence, partly reinfusing $N.", FALSE,
            ch, 0, victim, TO_CHAR);
        GET_HIT(ch) -= 50;
        GET_HIT(victim) += 100;
    }
}

void spell_eye_of_the_dead(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (!affected_by_spell(ch, SPELL_EYE_OF_THE_DEAD) &&
        !IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
        act("One of $n's eyes pulses with an eerie blue light.", FALSE, ch,
            0, 0, TO_ROOM);
        send_to_char("You summon the eye of the dead, which increases your "
                     "sight considerably.\n\r", ch);

        af.type = SPELL_EYE_OF_THE_DEAD;
        af.duration = 20;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_TRUE_SIGHT;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing seems to happen.\n\r", ch);
    }
}

void spell_soul_steal(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             mana;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("You cannot steal your own soul, it is already in the "
                     "Dark Lord's possession.\n\r", ch);
        return;
    }

    mana = 20 + number(-8, 70);

    act("Briefly, a glowing strand of energy seems to fleet from $N to $n.",
        FALSE, ch, 0, victim, TO_NOTVICT);
    act("A glowing strand of energy coming from $N fortifies your mental "
        "condition.", FALSE, ch, 0, victim, TO_CHAR);
    act("A strand of your mental energy fleets towards $n, leaving you "
        "slightly confused.", FALSE, ch, 0, victim, TO_VICT);

    if (!victim->specials.fighting) {
        AddHated(victim, ch);
        set_fighting(victim, ch);
    }

    GET_MANA(victim) -= mana;
    GET_MANA(ch) += mana;
}

void spell_life_leech(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    int             dam,
                    dicen = 1;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("Leech your own life? That could be a bad idea.\n\r", ch);
        return;
    }

    if (IsUndead(victim)) {
        act("But $N doesn't have any life in $M!", FALSE, ch, 0, victim,
            TO_CHAR);
        return;
    }

    dicen = (int) level / 10 + 4;
    /*
     * avg 35 | max 55 | min 15
     */
    dam = dice(dicen, 7) + 3;
    damage(ch, victim, dam, SPELL_LIFE_LEECH);

    if (IsResist(victim, IMM_DRAIN)) {
        dam >>= 1;
    }
    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        dam >>= 1;
    }
    if (IsImmune(victim, IMM_DRAIN)) {
        dam = 0;
    }
    GET_HIT(ch) += dam;
}

void spell_dark_pact(int level, struct char_data *ch,
                     struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (affected_by_spell(ch, SPELL_DARK_PACT)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    send_to_char("$c0008Your agreement with the Dark Lord will grant you more "
                 "mental stamina in exchange for your health.\n\r", ch);
    act("$c0008$n makes a pact with $s master.", FALSE, ch, 0, 0, TO_ROOM);

    af.type = SPELL_DARK_PACT;
    af.duration = 5;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void spell_darktravel(int level, struct char_data *ch,
                      struct char_data *tmp_ch, struct obj_data *obj)
{
    struct room_data *rp,
                   *nrp,
                   *room;
    char            str[180];
    int             location = 0,
                    tmp = 0;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    /*
     * check target room for legality.
     */
    rp = real_roomp(ch->in_room);
    if (!rp) {
        send_to_char("The magic fails\n\r", ch);
        return;
    }

    if (A_NOTRAVEL(ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (A_NOTRAVEL(tmp_ch)) {
        send_to_char("The arena rules do not permit you to use travelling "
                     "spells!\n\r", ch);
        return;
    }

    if (!(nrp = real_roomp(tmp_ch->in_room))) {
        sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
        Log(str);
        send_to_char("The magic cannot locate the target.\n\r", ch);
        return;
    }

    if (IS_SET(real_roomp(tmp_ch->in_room)->room_flags, NO_SUM)) {
        send_to_char("Ancient Magiks bar your path.\n\r", ch);
        return;
    }

    if (!IsOnPmp(ch->in_room)) {
        send_to_char("You're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (!IsOnPmp(tmp_ch->in_room)) {
        send_to_char("They're on an extra-dimensional plane!\n\r", ch);
        return;
    }

    if (IS_SET(SystemFlags, SYS_NOPORTAL)) {
        send_to_char("The planes are fuzzy, you cannot travel through the "
                     "shadows!\n", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_LINKDEAD(tmp_ch)) {
        send_to_char("Nobody playing by that name.\n\r", ch);
        return;
    }

    if (IS_PC(tmp_ch) && IS_IMMORTAL(tmp_ch)) {
        send_to_char("You can't travel to someone of that magnitude!\n\r", ch);
        return;
    }

    /*
     * target ok, let's travel
     */
    send_to_char("$c0008You step into the shadows and are relocated.\n\r", ch);
    act("$c0008$n closes $s eyes and steps into the shadows.", FALSE, ch,
        0, 0, TO_ROOM);
    if (number(1, 33) == 33) {
        /*
         * 3% chance of a mislocate
         */
        while (!location) {
            tmp = number(0, top_of_world);
            room = real_roomp(tmp);
            if (room) {
                if ((IS_SET(room->room_flags, PRIVATE)) ||
                    (IS_SET(room->room_flags, TUNNEL)) ||
                    (IS_SET(room->room_flags, NO_SUM)) ||
                    (IS_SET(room->room_flags, NO_MAGIC)) ||
                    !IsOnPmp(location)) {
                    location = 0;
                } else {
                    location = tmp;
                }
            }
        }
        send_to_char("$c0008A sudden lapse in your concentration carries you "
                     "elsewhere.\n\r", ch);
    } else {
        location = tmp_ch->in_room;
    }
    send_to_char("$c0008Travelling through the shadows, you find yourself in a"
                 " different place..\n\r", ch);
    char_from_room(ch);
    char_to_room(ch, location);
    act("$c0008$n steps out of the shadows.", FALSE, ch, 0, 0, TO_ROOM);
    do_look(ch, "", 15);
}

void spell_vampiric_embrace(int level, struct char_data *ch,
                            struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    assert(ch);

    if (affected_by_spell(ch, SPELL_VAMPIRIC_EMBRACE)) {
        send_to_char("Nothing new seems to happen.\n\r", ch);
        return;
    }

    if (ch->equipment[WIELD]) {
        send_to_char("$c0008A negative aura surrounds your weapon, swallowing"
                     " the light.\n\r", ch);
        act("$c0008A negative aura surrounds $n's weapon, drinking in the "
            "light.", FALSE, ch, 0, 0, TO_ROOM);
    } else {
        send_to_char("$c0008A negative aura surrounds your hands, swallowing "
                     "the light.\n\r", ch);
        act("$c0008A negative aura surrounds $n's hands, drinking in the "
            "light.", FALSE, ch, 0, 0, TO_ROOM);
    }

    af.type = SPELL_VAMPIRIC_EMBRACE;
    af.duration = 4;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void spell_bind_affinity(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct room_data *rp;
    int             roomnr;

    assert(ch);

    roomnr = ch->in_room;
    if (roomnr == 0) {
        send_to_char("You cannot set the void as your binding place.\n\r", ch);
        return;
    }
    rp = real_roomp(roomnr);
    if (!rp) {
        Log("some player got lost in a non existent room");
        return;
    }

    if (IS_SET(rp->room_flags, PRIVATE | NO_SUM | NO_MAGIC)) {
        send_to_char("Arcane magics prevent you from creating a lasting "
                     "connection to this place.\n\r", ch);
        return;
    }

    send_to_char("Performing the ancient ritual of binding, you increase your "
                 " affinity\n\r", ch);
    send_to_char("with your current location so it becomes your binding "
                 "space.\n\r", ch);

    ch->player.hometown = roomnr;
}

void spell_scourge_warlock(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    int             dam;
    extern struct index_data *obj_index;
    int             resist = 0;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("The Dark Lord does not permit you to harm his followers."
                     " This includes you.\n\r", ch);
        return;
    }

    if (ch->equipment[WEAR_EYES]) {
        obj = ch->equipment[WEAR_EYES];
        if (obj && obj_index[obj->item_number].virtual != TONGUE_ITEM) {
            act("Your cannot perform the Warlock's Scourge wearing $p.",
                FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
    } else {
        /*
         * give him a tongue to wear on his face
         */
        if ((obj = read_object(TONGUE_ITEM, VIRTUAL))) {
            /*
             * add a random Dark Lord's boon to it
             */
            switch (number(1, 20)) {
            case 1:
            case 2:
                /*
                 * 1dam
                 */
                obj->affected[0].location = APPLY_DAMROLL;
                obj->affected[0].modifier = 1;
                break;
            case 3:
            case 4:
                /*
                 * 10mr
                 */
                obj->affected[0].location = APPLY_MANA_REGEN;
                obj->affected[0].modifier = 10;
                break;
            case 5:
            case 6:
                /*
                 * -10sf
                 */
                obj->affected[0].location = APPLY_SPELLFAIL;
                obj->affected[0].modifier = -10;
                break;
            case 7:
                /*
                 * random resist
                 */
                switch (number(1, 8)) {
                case 1:
                    resist = IMM_FIRE;
                    break;
                case 2:
                    resist = IMM_COLD;
                    break;
                case 3:
                    resist = IMM_ELEC;
                    break;
                case 4:
                    resist = IMM_ENERGY;
                    break;
                case 5:
                    resist = IMM_ACID;
                    break;
                case 6:
                    resist = IMM_SLEEP;
                    break;
                case 7:
                    resist = IMM_CHARM;
                    break;
                default:
                    resist = IMM_HOLD;
                    break;
                }
                obj->affected[0].location = APPLY_IMMUNE;
                obj->affected[0].modifier = resist;
                break;
            case 8:
            case 9:
            case 10:
                /*
                 * -10 armor c
                 */
                obj->affected[0].location = APPLY_ARMOR;
                obj->affected[0].modifier = -10;
                break;
            case 11:
                /*
                 * +2 int c
                 */
                obj->affected[0].location = APPLY_INT;
                obj->affected[0].modifier = 2;
                break;
            case 12:
            case 13:
                /*
                 * +1 str
                 */
                obj->affected[0].location = APPLY_STR;
                obj->affected[0].modifier = 1;
                break;
            case 14:
                /*
                 * +2 wis
                 */
                obj->affected[0].location = APPLY_WIS;
                obj->affected[0].modifier = 2;
                break;
            case 15:
                /*
                 * sa:spy c
                 */
                obj->affected[0].location = APPLY_SPELL;
                obj->affected[0].modifier = AFF_SCRYING;
                break;
            default:
                break;
            }
            equip_char(ch, obj, WEAR_EYES);
            send_to_char("$c0008The Dark Lord grants you a boon, and you feel "
                         "p$c000go$c000Gi$c000gso$c0008n flow through your "
                         "veins.\n\r", ch);
        } else {
            Log("screw up in scourge of warlock, cannot load the dark lord's "
                "boon");
            send_to_char("Alas, something's not quite right with this spell "
                         "yet. Notify imms, use the bug command.\n\r", ch);
            return;
        }
    }

    if (!affected_by_spell(victim, SPELL_POISON)) {
        spell_poison(level, ch, victim, 0);
    }

    /*
     * now that we poisoned the bugger, let's do the real
     */
    dam = dice(level, 9);
    if (saves_spell(victim, SAVING_SPELL)) {
        /*
         * save for half damage
         */
        dam >>= 1;
    }
    damage(ch, victim, dam, SPELL_SCOURGE_WARLOCK);
}

void spell_finger_of_death(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    int             dam;

    assert(victim && ch);

    if (level < 0 || level > ABS_MAX_LVL) {
        return;
    }
    if (victim == ch) {
        send_to_char("It isn't a healthy idea to point a finger of death at "
                     "yourself. Try again.\n\r", ch);
        return;
    }

    if (number(1, 20) == 20 && !IS_IMMUNE(victim, IMM_DRAIN)) {
        /*
         * instant death, wheee!
         */
        dam = GET_HIT(victim) + 25;
        damage(ch, victim, dam, SPELL_FINGER_OF_DEATH);
    } else {
        dam = dice(level, 6);
        if (saves_spell(victim, SAVING_PETRI)) {
            /* save for half damage */
            dam >>= 1;
        }
        damage(ch, victim, dam, SPELL_FINGER_OF_DEATH);
    }
}

void spell_flesh_golem(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    struct room_data *rp;
    struct char_data *mob;
    int             lev = 1,
                    mlev,
                    mhps,
                    mtohit;

    if ((rp = real_roomp(ch->in_room)) == NULL) {
        return;
    }
    if (!ch) {
        Log("screw up in flesh golem, no caster found");
        return;
    }

    if (!obj) {
        send_to_char("What would you use for a component?\n\r", ch);
        return;
    }

    if (!IS_CORPSE(obj)) {
        send_to_char("That's not a corpse.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_FLESH_GOLEM)) {
        send_to_char("You do not yet have the mental reserves to create "
                     "another flesh golem.\n\r", ch);
        return;
    }

    if (IS_SET(rp->room_flags, TUNNEL)) {
        send_to_char("There's no room to create a flesh golem.\n\r", ch);
        return;
    }

    if (A_NOPETS(ch)) {
        send_to_char("The arena rules do not permit you to animate "
                     "corpses!\n\r", ch);
        return;
    }

    act("$n stares at $p while phrasing some arcane incantations.", TRUE,
        ch, obj, 0, TO_ROOM);
    act("You stare at $p while mumbling some arcane incantations.", TRUE,
        ch, obj, 0, TO_CHAR);
    lev = GetMaxLevel(ch);

    mob = read_mobile(FG_TEMPLATE, VIRTUAL);
    if (!mob) {
        Log("No template found for flesh golem spell (no mobile with vnum 6)");
        send_to_char("Screw up in this spell, no flesh golem template "
                     "found\n\r", ch);
        return;
    }

    if (lev > 45) {
        mlev = number(30, 40);
        mhps = number(140, 200);
        mtohit = 8;
    } else if (lev > 37) {
        mlev = number(25, 35);
        mhps = number(90, 120);
        mtohit = 6;
    } else if (lev > 29) {
        mlev = number(20, 29);
        mhps = number(30, 60);
        mtohit = 4;
    } else if (lev > 21) {
        mlev = number(14, 20);
        mhps = number(10, 40);
        mtohit = 2;
    } else {
        mlev = number(4, 6);
        mhps = number(0, 20);
        mtohit = 0;
    }

    mob->player.level[2] = mlev;
    mob->points.max_hit = mob->points.max_hit + mhps;
    mob->points.hit = mob->points.max_hit;
    mob->points.hitroll = mob->points.hitroll + mtohit;
    char_to_room(mob, ch->in_room);
    extract_obj(obj);

    act("The corpse starts stirring, and rises as $n.", FALSE, mob, obj, 0,
        TO_ROOM);

    if (too_many_followers(ch)) {
        act("$N takes one look at the size of your posse and just says no!",
            TRUE, ch, 0, mob, TO_CHAR);
        act("$N takes one look at the size of $n's posse and just says no!",
            TRUE, ch, 0, mob, TO_NOTVICT);
        act("You take one look at the size of $n's posse and just say no!",
            TRUE, ch, 0, mob, TO_VICT);
    } else {
        /*
         * charm it for a while
         */
        if (mob->master) {
            stop_follower(mob);
        }
        add_follower(mob, ch);
        af.type = SPELL_CHARM_PERSON;
        if (IS_PC(ch) || ch->master) {
            af.duration = GET_CHR(ch);
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
            affect_to_char(mob, &af);
        } else {
            SET_BIT(mob->specials.affected_by, AFF_CHARM);
        }
    }
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
    }
    if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
    }

    af.type = SPELL_FLESH_GOLEM;
    af.duration = 12;
    af.modifier = 0;
    af.location = 0;
    af.bitvector = 0;
    affect_to_char(ch, &af);
}

void spell_chillshield(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_CHILLSHIELD)) {
        if (affected_by_spell(ch, SPELL_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("The cold of your spell extinguishes the fire surrounding "
                "you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_FIRESHIELD);
        }

        if (IS_AFFECTED(ch, AFF_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("The cold of your spell extinguishes the fire surrounding "
                "you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_FIRESHIELD);
        }

        if (affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
            act("The whirling blades around $n freeze up and shatter.",
                TRUE, ch, 0, 0, TO_ROOM);
            act("The cold of your spell shatters the blade barrier surrounding"
                " you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_BLADE_BARRIER);
        }

        if (IS_AFFECTED(ch, AFF_BLADE_BARRIER)) {
            act("The whirling blades around $n freeze up and shatter.",
                TRUE, ch, 0, 0, TO_ROOM);
            act("The cold of your spell shatters the blade barrier surrounding"
                " you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_BLADE_BARRIER);
        }

        act("$c000C$n is surrounded by a cold blue aura.", TRUE, ch, 0, 0,
            TO_ROOM);
        act("$c000CYou summon an aura of chilly blue flames around you.",
            TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_CHILLSHIELD;
        af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_CHILLSHIELD;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void spell_blade_barrier(int level, struct char_data *ch,
                         struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_BLADE_BARRIER)) {
        if (affected_by_spell(ch, SPELL_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("Your blades rip away the last vistiges of the fireshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_FIRESHIELD);
        }

        if (IS_AFFECTED(ch, AFF_FIRESHIELD)) {
            act("The fiery aura around $n is extinguished.", TRUE, ch, 0,
                0, TO_ROOM);
            act("Your blades rip away the last vistiges of the fireshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_FIRESHIELD);
        }

        if (affected_by_spell(ch, SPELL_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, ch, 0, 0,
                TO_ROOM);
            act("Your blades rip away the last vistiges of the chillshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            affect_from_char(ch, SPELL_CHILLSHIELD);
        }
        if (IS_AFFECTED(ch, AFF_CHILLSHIELD)) {
            act("The cold aura around $n is extinguished.", TRUE, ch, 0, 0,
                TO_ROOM);
            act("Your blades rip away the last vistiges of the chillshield "
                "surrounding you.", TRUE, ch, 0, 0, TO_CHAR);
            REMOVE_BIT(ch->specials.affected_by, AFF_CHILLSHIELD);
        }

        act("$c000B$n is surrounded by a barrier of whirling blades.",
            TRUE, ch, 0, 0, TO_ROOM);
        act("$c000BYou summon a barrier of whirling blades around you.",
            TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_BLADE_BARRIER;

        if (ch->specials.remortclass != CLERIC_LEVEL_IND + 1) {
            af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        } else {
            af.duration = (level < LOW_IMMORTAL) ? 4 : level;
        }
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_BLADE_BARRIER;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void spell_mana_shield(int level, struct char_data *ch,
                       struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_MANA_SHIELD)) {
        act("$c000y$n summons forth a $c000Ygolden $c000yglobe to float next "
            "to $s head.", TRUE, ch, 0, 0, TO_ROOM);
        act("$c000yYou summon forth a $c000Ygolden $c000yglobe to float next "
            "to your head.", TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_MANA_SHIELD;
        af.duration = (level < LOW_IMMORTAL) ? 3 : level;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

void spell_group_heal(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    struct char_data *tch;

    if (real_roomp(ch->in_room) == NULL) {
        return;
    }

    if (!IS_AFFECTED(ch, AFF_GROUP)) {
        send_to_char("You cannot cast this spell when you're not in a "
                     "group!\n\r", ch);
        return;
    }

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (in_group(tch, ch)) {
            spell_heal(level, ch, tch, 0);
        }
    }
}

void spell_iron_skins(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    /*
     * put up some iron skinned barriers
     * that will absorb (duration) hps.
     */
    struct affected_type af;

    if (!affected_by_spell(ch, SPELL_MANA_SHIELD)) {
        act("$n invokes the earthen forces, and surrounds $mself with iron "
            "skins.", TRUE, ch, 0, 0, TO_ROOM);
        act("You invoke the earthen forces, and surround yourself with iron "
            "skins.", TRUE, ch, 0, 0, TO_CHAR);

        af.type = SPELL_IRON_SKINS;
        af.duration = (level * 3) / 2;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = 0;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
