#include "config.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"
#include "utils.h"

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

void cast_slow_poison(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_slow_poison(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in slow_poison.");
        break;
    }
}
