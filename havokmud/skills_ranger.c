#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

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

void cast_giant_growth(int level, struct char_data *ch, char *arg,
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
        spell_giant_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in giant growth.");
        break;
    }
}
