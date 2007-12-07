/*
 * HavokMUD - necromancer skills and spells
 * REQUIRED BY necromancer
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"

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
    if (!IS_CORPSE(corpse)) {
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
    LinkedListLock( corpse->containList );
    for( item = corpse->containList->head; item; item = nextItem ) {
        nextItem = item->next;
        obj_object = CONTAIN_LINK_TO_OBJ( item );
        objectTakeFromObject(obj_object, LOCKED);
        objectGiveToChar(obj_object, mob);
    }
    LinkedListUnlock( corpse->containList );

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
    objectExtract(corpse);
}

void cast_animate_dead(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{

    struct obj_data    *i;
    LinkedListItem_t   *item;
    struct room_data   *room;

    if (NoSummon(ch)) {
        return;
    }
    switch (type) {

    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            if (IS_CORPSE(tar_obj)) {
                spell_animate_dead(level, ch, 0, tar_obj);
            } else {
                send_to_char("That's not a corpse!\n\r", ch);
                return;
            }
        } else {
            send_to_char("That isn't a corpse!\n\r", ch);
            return;
        }
        break;
    case SPELL_TYPE_POTION:
        send_to_char("Your body revolts against the magic liquid.\n\r", ch);
        ch->points.hit = 0;
        break;
    case SPELL_TYPE_STAFF:
        rp = roomFindNum(ch->in_room);
        LinkedListLock( rp->contentList );
        for( item = rp->contentList->head; item; item = item->next ) {
            i = CONTENT_LINK_TO_OBJ(item);
            if (IS_CORPSE(i)) {
                spell_animate_dead(level, ch, 0, i);
            }
        }
        LinkedListUnlock( rp->contentList );
        break;
    default:
        Log("Serious screw-up in animate_dead!");
        break;
    }
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
    rp = roomFindNum(roomnr);
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
void cast_bind_affinity(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_bind_affinity(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_bind_affinity");
        break;
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

    if (!roomFindNum(location)) {
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
    do_look(ch, NULL, 15);
}
void cast_binding(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
        spell_binding(level, ch, 0, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj)
            return;
        spell_binding(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_binding");
        break;
    }
}

#define CB_TEMPLATE 4
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

    if ((rp = roomFindNum(ch->in_room)) == NULL) {
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
    objectExtract(obj);

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
void cast_cavorting_bones(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_cavorting_bones(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in cast_cavorting_bones");
        break;
    }
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
void cast_chill_touch(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *victim,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
        spell_chill_touch(level, ch, victim, 0);
        break;
    default:
        Log("Serious screw-up in chill touch!");
        break;
    }
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
        af.duration = (!IS_IMMORTAL(ch) ? 3 : level);
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_CHILLSHIELD;
        affect_to_char(ch, &af);
    } else {
        send_to_char("Nothing new seems to happen.\n\r", ch);
    }
}
void cast_chillshield(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_chillshield(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_chillshield");
        break;
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
        !IS_SET(roomFindNum(ch->in_room)->room_flags, ARENA_ROOM)) {
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
void cast_clinging_darkness(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_clinging_darkness(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in clinging_darkness.");
        break;
    }
}

#define COLD_LIGHT 19
void spell_cold_light(int level, struct char_data *ch,
                      struct char_data *victim, struct obj_data *obj)
{
    /*
     * creates a cold light .
     */
    struct obj_data *tmp_obj;

    assert(ch);
    assert((level >= 0) && (level <= ABS_MAX_LVL));

    tmp_obj = objectRead(COLD_LIGHT);
    if (tmp_obj) {
        tmp_obj->value[2] = 24 + level;
        objectGiveToChar(tmp_obj, ch);
    } else {
        send_to_char("Sorry, I can't create the cold ball of ice.\n\r", ch);
        return;
    }

    act("$n thrusts $s hands through the ground, and pulls out $p.", TRUE,
        ch, tmp_obj, 0, TO_ROOM);
    act("You thrust your hands through the ground, and pull out $p.", TRUE,
        ch, tmp_obj, 0, TO_CHAR);
}
void cast_cold_light(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_cold_light(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cold light.");
        break;
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
void cast_dark_empathy(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dark_empathy(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_dark_empathy");
        break;
    }
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
void cast_dark_pact(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dark_pact(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_dark_pact");
        break;
    }
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
    rp = roomFindNum(ch->in_room);
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

    if (!(nrp = roomFindNum(tmp_ch->in_room))) {
        sprintf(str, "%s not in any room", GET_NAME(tmp_ch));
        Log(str);
        send_to_char("The magic cannot locate the target.\n\r", ch);
        return;
    }

    if (IS_SET(roomFindNum(tmp_ch->in_room)->room_flags, NO_SUM)) {
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
            room = roomFindNum(tmp);
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
    do_look(ch, NULL, 15);
}
void cast_darktravel(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_darktravel(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_darktravel");
        break;
    }
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
void cast_decay(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_decay(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_decay");
        break;
    }
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
        !IS_SET(roomFindNum(ch->in_room)->room_flags, ARENA_ROOM)) {
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
void cast_disease(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_disease(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in disease.");
        break;
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
void cast_dominate_undead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_dominate_undead(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in dominate_undead.");
        break;
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
void cast_endure_cold(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_endure_cold(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in endure_cold.");
        break;
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
void cast_eye_of_the_dead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_eye_of_the_dead(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_eye_of_the_dead");
        break;
    }
}

#define FG_TEMPLATE 6
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

    if ((rp = roomFindNum(ch->in_room)) == NULL) {
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
    objectExtract(obj);

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
void cast_flesh_golem(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_flesh_golem(level, ch, 0, tar_obj);
        break;
    default:
        Log("serious screw-up in cast_flesh_golem");
        break;
    }
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
void cast_invis_to_undead(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_invis_to_undead(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in invis_to_undead.");
        break;
    }
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
void cast_finger_of_death(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_finger_of_death(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_finger_of_death");
        break;
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
void cast_gather_shadows(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_gather_shadows(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in gather_shadows.");
        break;
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
void cast_life_draw(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_draw(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in life_draw.");
        break;
    }
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
void cast_life_leech(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_leech(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_life_leech");
        break;
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
        !IS_SET(roomFindNum(ch->in_room)->room_flags, ARENA_ROOM)) {
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
void cast_life_tap(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_life_tap(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in _life_tap.");
        break;
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
void cast_mend_bones(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_mend_bones(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in mend_bones.");
        break;
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
    for (t = roomFindNum(ch->in_room)->people; t; t = next) {
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
void cast_mist_of_death(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_mist_of_death(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_mist_of_death");
        break;
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
void cast_nullify(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_nullify(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_nullify");
        break;
    }
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
void cast_numb_dead(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_numb_dead(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_numb_dead");
        break;
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
void cast_protection_from_good(int level, struct char_data *ch, char
                               *arg, int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = roomFindNum(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from good!");
        break;
    }
}

#define TONGUE_ITEM 22
void spell_scourge_warlock(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    int             dam;
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
        if (obj && obj->item_number != TONGUE_ITEM) {
            act("Your cannot perform the Warlock's Scourge wearing $p.",
                FALSE, ch, obj, 0, TO_CHAR);
            return;
        }
    } else {
        /*
         * give him a tongue to wear on his face
         */
        if ((obj = objectRead(TONGUE_ITEM))) {
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
void cast_scourge_warlock(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_scourge_warlock(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_scourge_warlock");
        break;
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
            !IS_SET(roomFindNum(EXIT(ch, attempt)->to_room)->room_flags,
                    DEATH) &&
            !IS_SET(roomFindNum(EXIT(ch, attempt)->to_room)->room_flags,
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
            do_look(ch, NULL, 15);
            return;
        }
    }

    send_to_char("The shadows in the vicinity didn't serve your purpose.\n\r",
                 ch);
}
void cast_shadow_step(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_STAFF:
    case SPELL_TYPE_SCROLL:
        spell_shadow_step(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in cast_shadow_step");
        break;
    }
}

void spell_siphon_strength(int level, struct char_data *ch,
                           struct char_data *victim, struct obj_data *obj)
{
    struct affected_type af;
    int             mod = 0;

    assert(ch && victim);

    if (victim == ch) {
        send_to_char("Siphon your own strength? What a waste of mana.\n\r", ch);
        return;
    }

    if (IS_PC(victim) &&
        !IS_SET(roomFindNum(ch->in_room)->room_flags, ARENA_ROOM)) {
        send_to_char("I think not. Go pester monsters instead.\n\r", ch);
        return;
    }

    if (!affected_by_spell(ch, SPELL_SIPHON_STRENGTH)) {
        if (!saves_spell(victim, SAVING_SPELL)) {
            mod = dice(1, 5);
            act("You feel your strength being siphoned from your body.",
                FALSE, victim, 0, 0, TO_VICT);
            act("$n staggers as some of his strength is siphoned away.",
                FALSE, victim, 0, 0, TO_ROOM);

            af.type = SPELL_SIPHON_STRENGTH;
            af.duration = 3;
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
void cast_siphon_strength(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_siphon_strength(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in siphon_strength.");
        break;
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
void cast_spectral_shield(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_spectral_shield(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in spectral_shield.");
        break;
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
void cast_soul_steal(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_soul_steal(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in cast_soul_steal");
        break;
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
void cast_suit_of_bone(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_suit_of_bone(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in suit_of_bone.");
        break;
    }
}

void spell_trace_corpse(int level, struct char_data *ch,
                        struct char_data *victim, char *arg)
{
    struct obj_data *i,
                   *corpse = NULL;
    char            buf[255];
    int             j = 0,
                    found = 0,
                    player_loc = 0,
                    corpse_loc = 0;

    if(!ch) {
        return;
    }

    buf[0] = '\0';
    key = StringToKeywords( arg, NULL );

    /*
     * when starting out, no corpse has been found yet
     */
    found = 0;
    send_to_char("You open your senses to recent sites of death.\n\r", ch);

    BalancedBTreeLock( objectKeywordTree );
    for (i = objectKeywordFindFirst( objectKeywordTree, key ); i && !found; 
         i = objectKeywordFindNext( objectKeywordTree, KEYWORD_ITEM_OFFSET,
                                    key, i )) {
        if( IS_CORPSE(i) ) {
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
                         roomFindNum(i->in_room)->name));
                j = 3;
            }
            corpse = i;
        }
    }
    BalancedBTreeUnlock( objectKeywordTree );
    send_to_char(buf, ch);

    FreeKeywords(key, TRUE);

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
        do_look(ch, NULL, 15);
        char_from_room(ch);
        char_to_room(ch, player_loc);
    }
}
void cast_trace_corpse(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_trace_corpse(level, ch, 0, arg);
        break;
    default:
        Log("serious screw-up in trace_corpse.");
        break;
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
void cast_unholyword(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_holyword(-level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in unholy word.");
        break;
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
void cast_unsummon(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_unsummon(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in unsummon.");
        break;
    }
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
void cast_vampiric_embrace(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_vampiric_embrace(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_vampiric_embrace");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
