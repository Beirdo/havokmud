#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"

/*
 * Global data 
 */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct obj_data *object_list;
extern int      rev_dir[];
extern char    *dirs[];
extern int      movement_loss[];
extern struct weather_data weather_info;
extern struct time_info_data time_info;
extern struct index_data *obj_index;

/*
 * Extern procedures 
 */

void            update_pos(struct char_data *victim);
void            clone_char(struct char_data *ch);
bool            saves_spell(struct char_data *ch, sh_int spell);
void            add_follower(struct char_data *ch,
                             struct char_data *victim);
#if 0
char *strdup(char *str); 
#endif

void            ChangeWeather(int change);
void            raw_unlock_door(struct char_data *ch,
                                struct room_direction_data *exitp,
                                int door);
int             NoSummon(struct char_data *ch);

/* 
 * I think this is the highest level you get a new poly, but I ain't sure..  
 */

#define MAX_DRUID_POLY 16       /* max number of Druid polies */

struct PolyType DruidList[MAX_DRUID_POLY] = {
    {"bear", 10, 9024},
    {"spider", 10, 20010},
    {"lamia", 10, 3648},
    {"lizard", 10, 6822},
    {"bear", 12, 9056},
    {"gator", 12, 9054},
    {"basilisk", 13, 7043},
#if 0
    {"snog", 14, 27008},
#endif
    {"snake", 15, 6517},
    {"spider", 15, 6113},
    {"lizard", 16, 6505},
    {"allosaurus", 18, 21801},
    {"tiger", 28, 9027},
    {"mulichort", 30, 15830},
    {"tiger", 35, 9055},
    {"lion", 35, 13718},
    {"salamander", 35, 25506}
};

#define LAST_DRUID_MOB 16       /* last level you get a new poly type */

void cast_resurrection(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        if (!tar_obj) {
            return;
        }
        spell_resurrection(level, ch, 0, tar_obj);
        break;
    case SPELL_TYPE_STAFF:
        if (!tar_obj) {
            return;
        }
        spell_resurrection(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious problem in 'resurrection'");
        break;
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



void cast_astral_walk(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
        if (!tar_ch) {
            send_to_char("Yes, but who do you wish to walk to?\n", ch);
        } else {
            spell_astral_walk(level, ch, tar_ch, 0);
        }
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room)
            if (tar_ch != ch) {
                spell_astral_walk(level, ch, tar_ch, 0);
            }
        break;
    default:
        Log("Serious screw-up in astral walk!");
        break;
    }
}


void cast_bless(int level, struct char_data *ch, char *arg, int type,
                struct char_data *tar_ch, struct obj_data *tar_obj)
{

    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_obj) {
            /* It's an object */
            return;
        } 

        /* Then it is a PC | NPC */
        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            GET_POS(tar_ch) == POSITION_FIGHTING) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_bless(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_BLESS) ||
            GET_POS(ch) == POSITION_FIGHTING)
            return;
        spell_bless(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            /* It's an object */
            return;
        } 
        
        /* Then it is a PC | NPC */
        if (!tar_ch)
            tar_ch = ch;

        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING))
            return;
        spell_bless(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            /* It's an object */
            return;
        }
        
        /* Then it is a PC | NPC */
        if (affected_by_spell(tar_ch, SPELL_BLESS) ||
            (GET_POS(tar_ch) == POSITION_FIGHTING))
            return;
        spell_bless(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in bless!");
        break;
    }
}


void cast_true_seeing(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (IS_AFFECTED(ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj || IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
            return;
        }
        spell_true_seeing(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_TRUE_SIGHT)) {
                spell_true_seeing(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in true_seeing!");
        break;
    }
}






void cast_control_weather(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    char           *arg1;

    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        arg = get_argument(arg, &arg1);

        if (arg1 && strcmp("better", arg1) && strcmp("worse", arg1)) {
            send_to_char("Do you want it to get better or worse?\n\r", ch);
            return;
        }

        if (!OUTSIDE(ch)) {
            send_to_char("You need to be outside.\n\r", ch);
        }

        if (!strcmp("better", arg1)) {
            if (weather_info.sky == SKY_CLOUDLESS) {
                return;
            }
            if (weather_info.sky == SKY_CLOUDY) {
                send_to_outdoor("The clouds disappear.\n\r");
                weather_info.sky = SKY_CLOUDLESS;
            }
            if (weather_info.sky == SKY_RAINING) {
                if (time_info.month > 3 && time_info.month < 14) {
                    send_to_outdoor("The rain has stopped.\n\r");
                } else {
                    send_to_outdoor("The snow has stopped. \n\r");
                }
                weather_info.sky = SKY_CLOUDY;
            }
            if (weather_info.sky == SKY_LIGHTNING) {
                if (time_info.month > 3 && time_info.month < 14) {
                    send_to_outdoor("The lightning has gone, but it is still "
                                    "raining.\n\r");
                } else {
                    send_to_outdoor("The blizzard is over, but it is still "
                                    "snowing.\n\r");
                }
                weather_info.sky = SKY_RAINING;
            }
            return;
        }
        
        if (weather_info.sky == SKY_CLOUDLESS) {
            send_to_outdoor("The sky is getting cloudy.\n\r");
            weather_info.sky = SKY_CLOUDY;
            return;
        }

        if (weather_info.sky == SKY_CLOUDY) {
            if (time_info.month > 3 && time_info.month < 14) {
                send_to_outdoor("It starts to rain.\n\r");
            } else {
                send_to_outdoor("It starts to snow. \n\r");
            }
            weather_info.sky = SKY_RAINING;
        }

        if (weather_info.sky == SKY_RAINING) {
            if (time_info.month > 3 && time_info.month < 14) {
                send_to_outdoor("You are caught in lightning storm.\n\r");
            } else {
                send_to_outdoor("You are caught in a blizzard. \n\r");
            }
            weather_info.sky = SKY_LIGHTNING;
        }
        break;

    default:
        Log("Serious screw-up in control weather!");
        break;
    }
}

void cast_create_food(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        act("$n magically creates a mushroom.", FALSE, ch, 0, 0, TO_ROOM);
        spell_create_food(level, ch, 0, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (tar_ch) {
            return;
        }
        spell_create_food(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in create food!");
        break;
    }
}

void cast_create_water(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (tar_obj->obj_flags.type_flag != ITEM_DRINKCON) {
            send_to_char("It is unable to hold water.\n\r", ch);
            return;
        }
        spell_create_water(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in create water!");
        break;
    }
}




void cast_heroes_feast(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_heroes_feast(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in heroes feast");
        break;
    }
}

void cast_cure_blind(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_blind(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_blind(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_blind(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure blind!");
        break;
    }
}

void cast_cure_critic(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_critic(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_critic(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_critic(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure critic!");
        break;
    }
}

void cast_cure_light(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_light(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_light(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_light(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure light!");
        break;
    }
}

void cast_cure_serious(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_cure_serious(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_cure_serious(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_cure_serious(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in cure serious!");
        break;
    }
}





void cast_detect_evil(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_DETECT_EVIL)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_DETECT_EVIL)) {
            return;
        }
        spell_detect_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED(tar_ch, AFF_DETECT_EVIL)) {
                spell_detect_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect evil!");
        break;
    }
}

void cast_detect_good(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_DETECT_GOOD)) {
            send_to_char("Nothing seems to happen.\n\r", tar_ch);
            return;
        }
        spell_detect_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_DETECT_GOOD)) {
            return;
        }
        spell_detect_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch && !IS_AFFECTED2(tar_ch, AFF2_DETECT_GOOD)) {
                spell_detect_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in detect good!");
        break;
    }
}



void cast_detect_poison(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_detect_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_detect_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            spell_detect_poison(level, ch, 0, tar_obj);
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_detect_poison(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in detect poison!");
        break;
    }
}

void cast_dispel_evil(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_dispel_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_dispel_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_dispel_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel evil!");
        break;
    }
}

void cast_dispel_good(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_dispel_good(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_dispel_good(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_dispel_good(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in dispel good!");
        break;
    }
}



void cast_heal(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (tar_ch == ch) {
            act("$n heals $mself.", FALSE, ch, 0, 0, TO_ROOM);
            act("You heal yourself.", FALSE, ch, 0, 0, TO_CHAR);
        } else {
            act("$n heals $N.", FALSE, ch, 0, tar_ch, TO_NOTVICT);
            act("You heal $N.", FALSE, ch, 0, tar_ch, TO_CHAR);
        }
        spell_heal(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_heal(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_heal(level, ch, tar_ch, 0);
        WAIT_STATE(ch, PULSE_VIOLENCE);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (in_group(ch, tar_ch)) {
                spell_heal(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in heal!");
        break;
    }
}


void cast_locate_object(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_locate_object(level, ch, NULL, arg);
        break;
    default:
        Log("Serious screw-up in locate object!");
        break;
    }
}

void cast_poison(int level, struct char_data *ch, char *arg, int type,
                 struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_WAND:
        spell_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_poison(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in poison!");
        break;
    }
}

void cast_protection_from_evil(int level, struct char_data *ch, char *arg,
                               int type, struct char_data *tar_ch,
                               struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
    }
}

void cast_protection_from_evil_group(int level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_evil_group(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_evil_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_evil_group(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from evil!");
        break;
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
        for (tar_ch = real_roomp(ch->in_room)->people;
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

void cast_protection_from_good_group(int level, struct char_data *ch,
                                     char *arg, int type,
                                     struct char_data *tar_ch,
                                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_protection_from_good_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_protection_from_good_group(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_protection_from_good_group(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_protection_from_good_group(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in protection from good group!");
        break;
    }
}

void cast_remove_curse(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_curse(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_curse(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            spell_remove_curse(level, ch, 0, tar_obj);
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_remove_curse(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_curse(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove curse!");
        break;
    }
}

void cast_remove_poison(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_poison(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_poison(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_poison(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove poison!");
        break;
    }
}

void cast_remove_paralysis(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_remove_paralysis(level, ch, tar_ch, tar_obj);
        break;
    case SPELL_TYPE_POTION:
        spell_remove_paralysis(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_remove_paralysis(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_remove_paralysis(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in remove paralysis!");
        break;
    }
}

void cast_sanctuary(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_sanctuary(level, ch, tar_ch, 0);
        break;

    case SPELL_TYPE_WAND:
    case SPELL_TYPE_POTION:
        spell_sanctuary(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_sanctuary(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) { 
            if (tar_ch != ch) {
                spell_sanctuary(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in sanctuary!");
        break;
    }
}





void cast_word_of_recall(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    struct char_data *tmp_ch;

    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_word_of_recall(level, ch, ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_word_of_recall(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_word_of_recall(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_word_of_recall(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people; tar_ch;
             tar_ch = tmp_ch) {
            tmp_ch = tar_ch->next_in_room;
            if (tar_ch != ch) {
                spell_word_of_recall(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in word of recall!");
        break;
    }
}




void cast_sense_life(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_SPELL:
        spell_sense_life(level, ch, ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_sense_life(level, ch, ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_sense_life(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in sense life!");
        break;
    }
}


#define MAX_BREATHS 3

struct pbreath {
    int             vnum,
                    spell[MAX_BREATHS];
};

struct pbreath breath_potions[] = {
    { 3970, {SPELL_FIRE_BREATH, 0}},
    { 3971, {SPELL_GAS_BREATH, 0}},
    { 3972, {SPELL_FROST_BREATH, 0}},
    { 3973, {SPELL_ACID_BREATH, 0}},
    { 3974, {SPELL_LIGHTNING_BREATH, 0}},
    {0}
};

void cast_dragon_breath(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *potion)
{
    struct pbreath *scan;
    int             i;
    struct affected_type af;
    char            buf[MAX_STRING_LENGTH];

    if (!potion) {
        return;
    }
    for (scan = breath_potions; 
         scan->vnum && scan->vnum != obj_index[potion->item_number].virtual;
         scan++) {
        /* 
         * Empty Loop 
         */
    }

    if (scan->vnum == 0) {
        send_to_char("Hey, this potion isn't in my list!\n\r", ch);
        sprintf(buf, "unlisted breath potion %s %ld", potion->short_description,
                obj_index[potion->item_number].virtual);
        Log(buf);
        return;
    }

    for (i = 0; i < MAX_BREATHS && scan->spell[i]; i++) {
        if (!affected_by_spell(ch, scan->spell[i])) {
            af.type = scan->spell[i];
            af.duration = 1 + dice(1, 2);
            if (GET_CON(ch) < 4) {
                send_to_char("You are too weak to stomach the potion and spew "
                             "it all over the floor.\n\r", ch);
                act("$n gags and pukes glowing goop all over the floor.",
                    FALSE, ch, 0, ch, TO_NOTVICT);
                break;
            }

            if (level > MIN(GET_CON(ch) - 1, GetMaxLevel(ch))) {
                send_to_char("!GACK! You are too weak to handle the full power"
                             " of the potion.\n\r", ch);
                act("$n gags and flops around on the floor a bit.", FALSE,
                    ch, 0, ch, TO_NOTVICT);
                level = MIN(GET_CON(ch) - 1, GetMaxLevel(ch));
            }

            af.modifier = -level;
            af.location = APPLY_CON;
            af.bitvector = 0;
            affect_to_char(ch, &af);
            send_to_char("You feel powerful forces build within your "
                         "stomach...\n\r", ch);
        }
    }
}

void cast_fire_breath(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can'c cast it! 
         */
        spell_fire_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in firebreath!");
        break;
    }
}

void cast_frost_breath(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can'c cast it! 
         */
        spell_frost_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in frostbreath!");
        break;
    }
}

void cast_acid_breath(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can'c cast it! 
         */
        spell_acid_breath(level, ch, tar_ch, 0);
        break; 
    default:
        Log("Serious screw-up in acidbreath!");
        break;
    }
}

void cast_gas_breath(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /*
         * THIS ONE HURTS!! 
         */
        spell_gas_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in gasbreath!");
        break;
    }
}

void cast_lightning_breath(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        /* 
         * It's a spell.. But people can'c cast it! 
         */
        spell_lightning_breath(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in lightningbreath!");
        break;
    }
}








void cast_turn(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_turn(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_turn(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in turn");
        break;
    }
}







void cast_tree_travel(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_tree_travel(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in tree travel");
        break;
    }
}

void cast_speak_with_plants(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    if (!tar_obj) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_speak_with_plants(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious screw-up in speak_with_plants");
        break;
    }
}

void cast_transport_via_plant(int level, struct char_data *ch, char *arg,
                              int type, struct char_data *tar_ch,
                              struct obj_data *tar_obj)
{
    if (!arg || !*arg) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_transport_via_plant(level, ch, 0, arg);
        break;
    default:
        Log("Serious screw-up in transport_via_plant");
        break;
    }
}

void cast_plant_gate(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    if (!arg || !*arg) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_plant_gate(level, ch, 0, arg);
        break;
    default:
        Log("Serious screw-up in plant gate");
        break;
    }
}



void cast_reincarnate(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{

    switch (type) {
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        if (!tar_obj) {
            return;
        }
        spell_reincarnate(level, ch, 0, tar_obj);
        break;
    case SPELL_TYPE_STAFF:
        if (!tar_obj) {
            return;
        }
        spell_reincarnate(level, ch, 0, tar_obj);
        break;
    default:
        Log("Serious problem in 'reincarnate'");
        break;
    }
}

void cast_changestaff(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }

    if (!ch->equipment[HOLD]) {
        send_to_char(" You must be holding a staff!\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_changestaff(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in changestaff.");
        break;
    }
}






void cast_aid(int level, struct char_data *ch, char *arg,
              int type, struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (!tar_ch) {
        tar_ch = ch;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_aid(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in scare.");
        break;
    }
}

void cast_holyword(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_holyword(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in holy word.");
        break;
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

void cast_golem(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_golem(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in golem.");
        break;
    }
}

void cast_command(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    char           *p;
    char            buf[128];
    /*
     * have to parse the argument to get the command 
     */

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        break;
    default:
        Log("serious screw-up in command.");
        return;
        break;
    }

    arg = skip_spaces(arg);
    if (arg) {
        p = fname(arg);

        if (((GetMaxLevel(tar_ch) < 6 && GET_INT(tar_ch) < 13) ||
             !saves_spell(tar_ch, SAVING_PARA)) && strcmp(p, "quit")) {
            sprintf(buf, "$n has commanded you to '%s'.", p);
            act(buf, FALSE, ch, 0, tar_ch, TO_VICT);
            send_to_char("Ok.\n\r", ch);
            command_interpreter(tar_ch, p);
            return;
        }

        sprintf(buf, "$n just tried to command you to '%s'.", p);
        act(buf, FALSE, ch, 0, tar_ch, TO_VICT);

        if (!IS_PC(tar_ch)) {
            hit(tar_ch, ch, TYPE_UNDEFINED);
        }
    }
}

void cast_change_form(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    char           *buffer;
    int             mobn = 0,
                    X = LAST_DRUID_MOB,
                    found = FALSE;
    struct char_data *mob;
    struct affected_type af;

    arg = get_argument(arg, &buffer);
    if( !buffer ) {
        send_to_char( "Change form to what?\n\r", ch );
        return;
    }

    if (IS_NPC(ch)) {
        send_to_char("You don't really want to do that.\n\r", ch);
        return;
    }

    if (affected_by_spell(ch, SPELL_CHANGE_FORM)) {
        send_to_char("You can only change form once every 12 hours\n\r", ch);
        return;
    }

    switch (type) {
    case SPELL_TYPE_SPELL:
        while (!found) {
            if (DruidList[X].level > level) {
                X--;
            } else {
                if (!strcasecmp(DruidList[X].name, buffer)) {
                    mobn = DruidList[X].number;
                    found = TRUE;
                } else {
                    X--;
                }
                if (X < 0) {
                    break;
                }
            }
        }

        if (!found) {
            send_to_char("Couldn't find any of those\n\r", ch);
            return;
        } else {
            mob = read_mobile(mobn, VIRTUAL);
            if (mob) {
                af.type = SPELL_CHANGE_FORM;
                af.duration = 12;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = 0;

                affect_to_char(ch, &af);

                spell_poly_self(level, ch, mob, 0);
            } else {
                send_to_char("You couldn't summon an image of that "
                             "creature\n\r", ch);
            }
        }
        break;

    default:
        Log("Problem in change form");
        break;
    }
}

void cast_shillelagh(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_shillelagh(level, ch, ch, tar_obj);
        break;
    default:
        Log("serious screw-up in shillelagh.");
        break;
    }
}

void cast_goodberry(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_goodberry(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in goodberry.");
        break;
    }
}

void cast_flame_blade(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_flame_blade(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in flame blade.");
        break;
    }
}

void cast_animal_growth(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_animal_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in animal_growth.");
        break;
    }
}

void cast_insect_growth(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_insect_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in insect_growth.");
        break;
    }
}

void cast_creeping_death(int level, struct char_data *ch, char *arg,
                         int type, struct char_data *tar_ch,
                         struct obj_data *tar_obj)
{
    char           *p;
    int             i;

    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        /*
         * get the argument, parse it into a direction 
         */
        arg = skip_spaces(arg);
        if (!arg) {
            send_to_char("you must supply a direction!\n\r", ch);
            return;
        }
        p = fname(arg);
        for (i = 0; i < 6; i++) {
            if (strncmp(p, dirs[i], strlen(p)) == 0) {
                i++;
                break;
            }
        }

        if (i == 6) {
            send_to_char("you must supply a direction!\n\r", ch);
            return;
        }
        spell_creeping_death(level, ch, 0, i);
        break;

    default:
        Log("serious screw-up in creeping_death.");
        break;
    }
}

void cast_commune(int level, struct char_data *ch, char *arg,
                  int type, struct char_data *tar_ch,
                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
        spell_commune(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in commune.");
        break;
    }
}


void cast_animal_summon_1(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_animal_summon(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in animal summon.");
        break;
    }
}

void cast_animal_summon_2(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_animal_summon(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in animal summon.");
        break;
    }
}

void cast_animal_summon_3(int level, struct char_data *ch, char *arg,
                          int type, struct char_data *tar_ch,
                          struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_animal_summon(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in animal summon.");
        break;
    }
}

void cast_fire_servant(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_FIRE_SERVANT);
        break;
    default:
        Log("serious screw-up in fire servant.");
        break;
    }
}

void cast_earth_servant(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_EARTH_SERVANT);
        break;
    default:
        Log("serious screw-up in earth servant.");
        break;
    }
}

void cast_water_servant(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_WATER_SERVANT);
        break;
    default:
        Log("serious screw-up in water servant.");
        break;
    }
}

void cast_wind_servant(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_elemental_summoning(level, ch, 0, SPELL_WIND_SERVANT);
        break;
    default:
        Log("serious screw-up in wind servant.");
        break;
    }
}

void cast_veggie_growth(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_veggie_growth(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in veggie_growth.");
        break;
    }
}

void cast_charm_veggie(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        spell_charm_veggie(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (!tar_ch) {
            return;
        }
        spell_charm_veggie(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_charm_veggie(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in charm veggie!");
        break;
    }
}

void cast_tree(int level, struct char_data *ch, char *arg, int type,
               struct char_data *tar_ch, struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    if (IS_NPC(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
        spell_tree(level, ch, 0, 0);
        break;
    default:
        Log("Serious screw-up in tree");
        break;
    }
}

void cast_animate_rock(int level, struct char_data *ch, char *arg,
                       int type, struct char_data *tar_ch,
                       struct obj_data *tar_obj)
{
    if (NoSummon(ch)) {
        return;
    }
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_animate_rock(level, ch, 0, tar_obj);
        break;
    default:
        Log("serious screw-up in animate_rock.");
        break;
    }
}

void cast_travelling(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_travelling(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in travelling.");
        break;
    }
}

void cast_animal_friendship(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_animal_friendship(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in animal friendship.");
        break;
    }
}

void cast_invis_to_animals(int level, struct char_data *ch, char *arg,
                           int type, struct char_data *tar_ch,
                           struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
        spell_invis_to_animals(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in invis to animals.");
        break;
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

void cast_entangle(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_entangle(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch) && !IS_IMMORTAL(tar_ch)) {
                spell_entangle(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in entangle.");
        break;
    }
}

void cast_snare(int level, struct char_data *ch, char *arg,
                int type, struct char_data *tar_ch,
                struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_snare(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch) && !IS_IMMORTAL(tar_ch)) {
                spell_snare(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in snare.");
        break;
    }
}


void cast_barkskin(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_barkskin(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (!in_group(tar_ch, ch)) {
                spell_barkskin(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("serious screw-up in barkskin.");
        break;
    }
}

void cast_warp_weapon(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_warp_weapon(level, ch, tar_ch, tar_obj);
        break;
    default:
        Log("serious screw-up in warp_weapon.");
        break;
    }
}

void cast_heat_stuff(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_heat_stuff(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in heat_stuff.");
        break;
    }
}

void cast_sunray(int level, struct char_data *ch, char *arg,
                 int type, struct char_data *tar_ch,
                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_sunray(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in sunray.");
        break;
    }
}

void cast_find_traps(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
        spell_find_traps(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_STAFF:
        spell_find_traps(level, ch, ch, 0);
        break;
    default:
        Log("serious screw-up in find_traps.");
        break;
    }
}

void cast_firestorm(int level, struct char_data *ch, char *arg,
                    int type, struct char_data *tar_ch,
                    struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_firestorm(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in firestorm.");
        break;
    }
}

void cast_dust_devil(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_STAFF:
        spell_dust_devil(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in dust_devil.");
        break;
    }
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







void cast_prot_energy_drain(int level, struct char_data *ch, char *arg,
                            int type, struct char_data *tar_ch,
                            struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_energy_drain(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-energy drain.");
        break;
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



void cast_prot_fire(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_fire(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_fire!");
        break;
    }
}

void cast_prot_cold(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_cold(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_cold!");
        break;
    }
}

void cast_prot_energy(int level, struct char_data *ch, char *arg,
                      int type, struct char_data *tar_ch,
                      struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_energy(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_energy!");
        break;
    }
}

void cast_prot_elec(int level, struct char_data *ch, char *arg, int type,
                    struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_POTION:
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        spell_prot_elec(level, ch, tar_ch, 0);
        break;
    default:
        Log("Serious screw-up in cast_prot_elec!");
        break;
    }
}



void cast_prot_dragon_breath_fire(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_fire(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_fire.");
        break;
    }
}

void cast_prot_dragon_breath_frost(int level, struct char_data *ch,
                                   char *arg, int type,
                                   struct char_data *tar_ch,
                                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_frost(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_frost.");
        break;
    }
}

void cast_prot_dragon_breath_elec(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_elec(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_elec.");
        break;
    }
}

void cast_prot_dragon_breath_acid(int level, struct char_data *ch,
                                  char *arg, int type,
                                  struct char_data *tar_ch,
                                  struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_acid(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_acid.");
        break;
    }
}

void cast_prot_dragon_breath_gas(int level, struct char_data *ch,
                                 char *arg, int type,
                                 struct char_data *tar_ch,
                                 struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_prot_dragon_breath_gas(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in prot-dragon breath_gas.");
        break;
    }
}

void cast_holy_strength(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_SPELL:
        spell_holy_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_holy_strength(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        spell_holy_strength(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (tar_ch != ch) {
                spell_holy_strength(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in holy strength!");
        break;
    }
}

void cast_holy_armor(int level, struct char_data *ch, char *arg, int type,
                     struct char_data *tar_ch, struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            send_to_char("Nothing seems to happen.\n\r", ch);
            return;
        }

        if (ch != tar_ch) {
            act("$N is protected by the spirits of god.", FALSE, ch, 0,
                tar_ch, TO_CHAR);
        }
        spell_holy_armor(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        if (affected_by_spell(ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        if (tar_obj) {
            return;
        }
        if (!tar_ch) {
            tar_ch = ch;
        }
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    case SPELL_TYPE_WAND:
        if (tar_obj) {
            return;
        }
        if (affected_by_spell(tar_ch, SPELL_HOLY_ARMOR)) {
            return;
        }
        spell_holy_armor(level, ch, ch, 0);
        break;
    default:
        Log("Serious screw-up in holy armor!");
        break;
    }
}

void cast_aura_power(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_aura_power(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in aura of power.");
        break;
    }
}

void cast_pacifism(int level, struct char_data *ch, char *arg,
                   int type, struct char_data *tar_ch,
                   struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
        act("$n calls upons the gods to calm down $N.", FALSE, ch, 0,
            tar_ch, TO_NOTVICT);
        act("You call upon the gods to calm down $N.", FALSE, ch, 0,
            tar_ch, TO_CHAR);
        spell_pacifism(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_POTION:
        spell_pacifism(level, ch, ch, 0);
        break;
    case SPELL_TYPE_SCROLL:
        spell_pacifism(level, ch, tar_ch, 0);
        break;
    case SPELL_TYPE_STAFF:
        for (tar_ch = real_roomp(ch->in_room)->people;
             tar_ch; tar_ch = tar_ch->next_in_room) {
            if (in_group(ch, tar_ch)) {
                spell_pacifism(level, ch, tar_ch, 0);
            }
        }
        break;
    default:
        Log("Serious screw-up in pacifism!");
        break;
    }
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


void cast_enlightenment(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_enlightenment(level, ch, tar_ch, 0);
        break;
    default:
        Log("serious screw-up in enlightenment.");
        break;
    }
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

/*
 * Necromancer Spells 
 */
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

void cast_blade_barrier(int level, struct char_data *ch, char *arg,
                        int type, struct char_data *tar_ch,
                        struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_blade_barrier(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_blade_barrier");
        break;
    }
}

void cast_group_heal(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_group_heal(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_group_heal");
        break;
    }
}

void cast_iron_skins(int level, struct char_data *ch, char *arg,
                     int type, struct char_data *tar_ch,
                     struct obj_data *tar_obj)
{
    switch (type) {
    case SPELL_TYPE_SPELL:
    case SPELL_TYPE_SCROLL:
    case SPELL_TYPE_POTION:
    case SPELL_TYPE_WAND:
    case SPELL_TYPE_STAFF:
        spell_iron_skins(level, ch, 0, 0);
        break;
    default:
        Log("serious screw-up in cast_iron_skins");
        break;
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
