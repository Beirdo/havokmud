/*
 * DaleMUD v2.0 Released 2/1994 See license.doc for distribution terms.
 * DaleMUD is based on DIKUMUD 
 */

#include "config.h"
#include <stdio.h>
#include <string.h>

#include "protos.h"

extern struct char_data *character_list;

/*
 * struct room_data *real_roomp(int); 
 */

extern int      TrapDir[];
extern struct dex_app_type dex_app[];
extern int      top_of_world;       /* ref to the top element of world */

void            NailThisSucker(struct char_data *ch);

void            TrapPoison(struct char_data *v, struct obj_data *i);


void do_settrap(struct char_data *ch, char *arg, int cmd)
{

    /*
     * parse for directions 
     * trap that affects all directions is an AE trap 
     * parse for type 
     * parse for level 
     */

}

int CheckForMoveTrap(struct char_data *ch, int dir)
{
    struct obj_data *i;

    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
        if (ITEM_TYPE(i) == ITEM_TRAP && GET_TRAP_CHARGES(i) > 0 &&
            IS_SET(GET_TRAP_EFF(i), TrapDir[dir]) &&
            IS_SET(GET_TRAP_EFF(i), TRAP_EFF_MOVE)) {
            return (TriggerTrap(ch, i));
        }
    }
    return (FALSE);
}

int CheckForInsideTrap(struct char_data *ch, struct obj_data *i)
{
    struct obj_data *t;

    for (t = i->contains; t; t = t->next_content) {
        if (ITEM_TYPE(t) == ITEM_TRAP && GET_TRAP_CHARGES(t) > 0 &&
            IS_SET(GET_TRAP_EFF(t), TRAP_EFF_OBJECT)) {
            return (TriggerTrap(ch, t));
        }
    }
    return (FALSE);
}

int CheckForAnyTrap(struct char_data *ch, struct obj_data *i)
{
    if ((ITEM_TYPE(i) == ITEM_TRAP) && (GET_TRAP_CHARGES(i) > 0)) {
        return (TriggerTrap(ch, i));
    }
    return (FALSE);
}

int CheckForGetTrap(struct char_data *ch, struct obj_data *i)
{
    if (ITEM_TYPE(i) == ITEM_TRAP &&
        GET_TRAP_CHARGES(i) > 0 &&
        IS_SET(GET_TRAP_EFF(i), TRAP_EFF_OBJECT)) {
        return (TriggerTrap(ch, i));
    }
    return (FALSE);
}

int TriggerTrap(struct char_data *ch, struct obj_data *i)
{
    struct char_data *v;

    if (ITEM_TYPE(i) == ITEM_TRAP && i->obj_flags.value[TRAP_CHARGES]) {
        act("You hear a strange noise...", TRUE, ch, 0, 0, TO_ROOM);
        act("You hear a strange noise...", TRUE, ch, 0, 0, TO_CHAR);
        GET_TRAP_CHARGES(i) -= 1;
        /*
         * make sure room fire off works! 
         */
        if (IS_SET(GET_TRAP_EFF(i), TRAP_EFF_ROOM)) {
            for (v = real_roomp(ch->in_room)->people; v;
                 v = v->next_in_room) {
                FindTrapDamage(v, i);
            }
        } else {
            FindTrapDamage(ch, i);
        }
        return (TRUE);
    }
    return (FALSE);
}

void FindTrapDamage(struct char_data *v, struct obj_data *i)
{
    /*
     * trap types < 0 are special 
     */
    if (GET_TRAP_DAM_TYPE(i) >= 0) {
        TrapDamage(v, GET_TRAP_DAM_TYPE(i), 3 * GET_TRAP_LEV(i), i);
    } else {
        TrapDamage(v, GET_TRAP_DAM_TYPE(i), 0, i);
    }
}

void TrapDamage(struct char_data *v, int damtype, int amnt,
                struct obj_data *t)
{
    char            buf[132];

    amnt = SkipImmortals(v, amnt, damtype);
    if (amnt == -1) {
        return;
    }

    if (IS_AFFECTED(v, AFF_SANCTUARY)) {
        amnt = MAX((int) (amnt / 2), 0);
    }
    amnt = PreProcDam(v, damtype, amnt);

    if (saves_spell(v, SAVING_PETRI)) {
        amnt = MAX((int) (amnt / 2), 0);
    }
    DamageStuff(v, damtype, amnt);

    amnt = MAX(amnt, 0);

    GET_HIT(v) -= amnt;

    update_pos(v);

    TrapDam(v, damtype, amnt, t);

    InformMess(v);
    if (GET_POS(v) == POSITION_DEAD) {
        if (!IS_NPC(v) && real_roomp(v->in_room)->name) {
            sprintf(buf, "%s killed by a trap at %s",
                    GET_NAME(v), real_roomp(v->in_room)->name);
            Log(buf);
        }
        die(v, '\0');
    }
}

void TrapDam(struct char_data *v, int damtype, int amnt,
             struct obj_data *t)
{
    char            desc[20];
    char            buf[132];

    /*
     * easier than dealing with message(ug) 
     */
    switch (damtype) {
    case TRAP_DAM_PIERCE:
        strcpy(desc, "pierced");
        break;
    case TRAP_DAM_SLASH:
        strcpy(desc, "sliced");
        break;
    case TRAP_DAM_BLUNT:
        strcpy(desc, "pounded");
        break;
    case TRAP_DAM_FIRE:
        strcpy(desc, "seared");
        break;
    case TRAP_DAM_COLD:
        strcpy(desc, "frozen");
        break;
    case TRAP_DAM_ACID:
        strcpy(desc, "corroded");
        break;
    case TRAP_DAM_ENERGY:
        strcpy(desc, "blasted");
        break;
    case TRAP_DAM_SLEEP:
        strcpy(desc, "knocked out");
        break;
    case TRAP_DAM_TELEPORT:
        strcpy(desc, "transported");
        break;
    case TRAP_DAM_POISON:
        strcpy(desc, "poisoned");
        break;
    default:
        /* need a poison trap! */
        strcpy(desc, "blown away");
        break;
    }

    if (damtype != TRAP_DAM_TELEPORT && damtype != TRAP_DAM_SLEEP && 
        damtype != TRAP_DAM_POISON) {
        /* check for poison trap here */
        if (amnt > 0) {
            sprintf(buf, "$n is %s by $p!", desc);
            act(buf, TRUE, v, t, 0, TO_ROOM);
            sprintf(buf, "You are %s by $p!", desc);
            act(buf, TRUE, v, t, 0, TO_CHAR);
        } else {
            sprintf(buf, "$n is almost %s by $p!", desc);
            act(buf, TRUE, v, t, 0, TO_ROOM);
            sprintf(buf, "You are almost %s by $p!", desc);
            act(buf, TRUE, v, t, 0, TO_CHAR);
        }
    }

    if (damtype == TRAP_DAM_TELEPORT) {
        TrapTeleport(v);
    } else if (damtype == TRAP_DAM_SLEEP) {
        TrapSleep(v);
    } else if (damtype == TRAP_DAM_POISON) {
        TrapPoison(v, t);
    }
}

void TrapTeleport(struct char_data *v)
{
    int             to_room,
                    try = 0;
    struct room_data *room;

    if (saves_spell(v, SAVING_SPELL)) {
        send_to_char("You feel strange, but the effect fades.\n\r", v);
        return;
    }

    do {
        /* do .. while bug fixed, msw */
        to_room = number(0, top_of_world);
        room = real_roomp(to_room);
        if (room && (IS_SET(room->room_flags, PRIVATE) ||
                     IS_SET(room->room_flags, TUNNEL) ||
                     IS_SET(room->room_flags, NO_SUM) ||
                     IS_SET(room->room_flags, NO_MAGIC) || 
                     !IsOnPmp(to_room))) {
            room = 0;
            try++;
        }
    } while (!room && try < 10);

    if (try >= 10) {
        send_to_char("The magic fails, you got lucky!\n\r", v);
        return;
    }

    act("$n slowly fade out of existence.", FALSE, v, 0, 0, TO_ROOM);
    char_from_room(v);
    char_to_room(v, to_room);
    act("$n slowly fade in to existence.", FALSE, v, 0, 0, TO_ROOM);

    do_look(v, "", 0);

    if (IS_SET(real_roomp(to_room)->room_flags, DEATH) &&
        GetMaxLevel(v) < LOW_IMMORTAL) {
        NailThisSucker(v);
    }
}

void TrapSleep(struct char_data *v)
{
    struct affected_type af;

    if (!saves_spell(v, SAVING_SPELL)) {
        af.type = SPELL_SLEEP;
        af.duration = 12;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_SLEEP;
        affect_join(v, &af, FALSE, FALSE);

        if (GET_POS(v) > POSITION_SLEEPING) {
            act("You feel very sleepy ..... zzzzzz", FALSE, v, 0, 0, TO_CHAR);
            act("$n goes to sleep.", TRUE, v, 0, 0, TO_ROOM);
            GET_POS(v) = POSITION_SLEEPING;
        }
    } else {
        send_to_char("You feel sleepy,but you recover\n\r", v);
    }
}

void TrapPoison(struct char_data *v, struct obj_data *t)
{
    act("You feel sick.", FALSE, v, 0, 0, TO_CHAR);
    act("$n looks sick.", TRUE, v, 0, 0, TO_ROOM);
    cast_poison(GET_TRAP_LEV(t), v, "", SPELL_TYPE_SPELL, v, 0);
}

void InformMess(struct char_data *v)
{
    switch (GET_POS(v)) {
    case POSITION_MORTALLYW:
        act("$n is mortally wounded, and will die soon, if not aided.",
            TRUE, v, 0, 0, TO_ROOM);
        act("You are mortally wounded, and will die soon, if not aided.",
            FALSE, v, 0, 0, TO_CHAR);
        break;
    case POSITION_INCAP:
        act("$n is incapacitated and will slowly die, if not aided.",
            TRUE, v, 0, 0, TO_ROOM);
        act("You are incapacitated and you will slowly die, if not aided.",
            FALSE, v, 0, 0, TO_CHAR);
        break;
    case POSITION_STUNNED:
        act("$n is stunned, but will probably regain consciousness.",
            TRUE, v, 0, 0, TO_ROOM);
        act("You're stunned, but you will probably regain consciousness.",
            FALSE, v, 0, 0, TO_CHAR);
        break;
    case POSITION_DEAD:
        act("$n is dead! R.I.P.", TRUE, v, 0, 0, TO_ROOM);
        act("You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
        break;
    default:
        /* >= POSITION SLEEPING */
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
