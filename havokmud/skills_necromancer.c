/*
 * All Necromancer skills and spells
 */

#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"

/*
 * Global data 
 */

/*
 * Extern functions 
 */

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

void cast_animate_dead(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{

    struct obj_data *i;

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
        for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
            if (GET_ITEM_TYPE(i) == ITEM_CONTAINER && i->obj_flags.value[3]) {
                spell_animate_dead(level, ch, 0, i);
            }
        }
        break;
    default:
        Log("Serious screw-up in animate_dead!");
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




/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
