#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#include "protos.h"
#include "externs.h"

#define SWORD_ANCIENTS 25000
#define Ivory_Gate    1499

/*
 ********************************************************************
 *  Special procedures for rooms                                    *
 ******************************************************************** */

#define Bandits_Path   2180
#define BASIL_GATEKEEPER_MAX_LEVEL 10

#define ATTACK_ROOM 3004

#define WHO_TO_HUNT  6112       /* green dragon */
#define WHERE_TO_SIT 3007       /* tavern */
#define WHO_TO_CALL  3063       /* mercenary */

#define AST_MOB_NUM 2715

extern struct char_data *character_list;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
extern struct QuestItem QuestList[4][IMMORTAL];

extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];

#define NADIA_PILL 45431
#define PEN_MIGHT 45445
#define DRAGON_SCEPTRE_ONE 45470
#define MARBLES 45475
#define JESTER_KEY 45480
#define NADIA_KEY 45489
#define EYE_DRAGON 45488
#define GATEKEEPER_KEY 45491
#define BLACK_PILL 45492
#define BLUE_PILL 45493

#define TMK_GUARD_ONE 45401
#define TMK_GUARD_TWO 45402
#define BRAXIS 45406
#define NADIA 45409
#define ZORK 45413
#define ELAMIN 45417
#define STARVING_MAN 45440
#define GOBLIN_CHUIRGEON 45443

#define BAHAMUT_HOME 46378

#define RESCUE_VIRGIN  1950
#define RESCUE_ROOM    1983
#define SNAKE_GOD      1953

struct riddle_answer {
    char   *answer;
    int     reward;
    char   *rewardText;
};
    
int mazekeeper_riddle_common(struct char_data *ch, char *arg,
                             struct char_data *mob, struct riddle_answer *rid,
                             int ridCount, int exp, int portal);

int ReadObjs(FILE * fl, struct obj_file_u *st);

#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))
extern struct obj_data *object_list;

struct char_data *FindMobInRoomWithFunction(int room, int (*func) ())
{
    struct char_data *temp_char;

    if (room > NOWHERE) {
        for (temp_char = real_roomp(room)->people; temp_char;
             temp_char = temp_char->next_in_room) {
            if (IS_MOB(temp_char) && mob_index[temp_char->nr].func == func) {
                return( temp_char );
            }
        }
    } 
    return (NULL);
}


/*************************
 * Start mobs specs
 */

int AGGRESSIVE(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *i,
                   *next;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    if (ch->in_room > -1) {
        for (i = real_roomp(ch->in_room)->people; i; i = next) {
            next = i->next_in_room;
            if (GET_RACE(i) != GET_RACE(ch) && !IS_IMMORTAL(i) &&
                CAN_SEE(ch, i)) {
                hit(ch, i, TYPE_UNDEFINED);
                break;
            }
        }
    }
    return( TRUE );
}

int blink(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 3) {
        act("$n blinks.", TRUE, ch, 0, 0, TO_ROOM);
        cast_teleport(12, ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int CaravanGuildGuard(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
    } else {
        if (cmd >= 1 && cmd <= 6) {
            switch (ch->in_room) {
            case 16115:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 16115, 1,
                         CLASS_MAGIC_USER | CLASS_SORCERER));
                break;
            case 16126:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 16116, 1, CLASS_CLERIC));
                break;
            case 16117:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 16117, 3, CLASS_THIEF));
                break;
            case 16110:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 16110, 3, CLASS_WARRIOR));
                break;
            }
        }
    }
    return (FALSE);
}

int CarrionCrawler(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *tar;
    int             i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (i = 0; i < 8; i++) {
        if ((tar = FindAHatee(ch)) == NULL) {
            tar = FindVictim(ch);
        }
        if (tar && tar->in_room == ch->in_room &&
            HitOrMiss(ch, tar, CalcThaco(ch))) {
            act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
            act("$n touches you!", 1, ch, 0, tar, TO_VICT);
            if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
                cast_paralyze(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, tar, 0);
                return TRUE;
            }
        }
    }
    return FALSE;
}

int citizen(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (check_soundproof(ch)) {
            return (FALSE);
        }
        if (!number(0, 18)) {
            command_interpreter(ch, "shout Guards! Help me! Please!");
        } else {
            act("$n shouts 'Guards!  Help me! Please!'", TRUE, ch, 0, 0,
                TO_ROOM);
        }
    }
    return (FALSE);
}

int coldcaster(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;
    byte            lspell;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * Find a dude to to evil things upon !
     */

    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    lspell = number(0, 9);

    switch (lspell) {
    case 0:
    case 1:
    case 2:
    case 3:
        act("$N touches you!", 1, vict, 0, ch, TO_CHAR);
        cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 4:
    case 5:
    case 6:
        cast_cone_of_cold(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    case 7:
    case 8:
    case 9:
        cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        break;
    }

    return (TRUE);
}

int Drow(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!ch->specials.fighting) {
        if (!affected_by_spell(ch, SPELL_GLOBE_DARKNESS)
            && !IS_AFFECTED(ch, AFF_DARKNESS)) {
            act("$n uses $s innate powers of darkness", FALSE, ch, 0, 0,
                TO_ROOM);
            cast_globe_darkness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
            return (TRUE);
        }

        if (ch->specials.hunting) {
            act("$n uses $s innate powers of levitation", FALSE, ch, 0, 0,
                TO_ROOM);
            cast_flying(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }
    } else {
        if (!affected_by_spell(ch->specials.fighting, SPELL_BLINDNESS) &&
            number(1, 100) > 25) {
            /*
             * simulate casting darkness on a person...
             */
            act("$n uses $s innate powers of darkness on $N!", FALSE, ch,
                0, ch->specials.fighting, TO_NOTVICT);
            act("$n drops a pitch black globe around you!", FALSE, ch, 0,
                ch->specials.fighting, TO_VICT);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                           ch->specials.fighting, 0);
            return (TRUE);
        }

        if (!affected_by_spell(ch->specials.fighting, SPELL_FAERIE_FIRE)
            && number(1, 100) > 50) {
            /*
             * simulate faerie fire
             */
            act("$n tries to outline $N with $s faerie fire!", FALSE, ch,
                0, ch->specials.fighting, TO_NOTVICT);
            act("$n tries to outline you with a faerie fire glow!", FALSE,
                ch, 0, ch->specials.fighting, TO_VICT);
            cast_faerie_fire(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             ch->specials.fighting, 0);
            return (TRUE);
        }
    }

    return (archer(ch, cmd, arg, mob, type));
}

int fido(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    register struct obj_data *i,
                   *temp,
                   *next_obj,
                   *next_r_obj;
    register struct char_data *v,
                   *next;
    register struct room_data *rp;
    char            found = FALSE;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if ((rp = real_roomp(ch->in_room)) == 0) {
        return (FALSE);
    }
    for (v = rp->people; (v && (!found)); v = next) {
        next = v->next_in_room;
        if (IS_NPC(v) && mob_index[v->nr].vnum == 100 && CAN_SEE(ch, v)) {
            /*
             * is a zombie
             */
            if (v->specials.fighting) {
                stop_fighting(v);
            }
            make_corpse(v, '\0');
            extract_char(v);
            found = TRUE;
        }
    }

    for (i = real_roomp(ch->in_room)->contents; i; i = next_r_obj) {
        next_r_obj = i->next_content;
        if (IS_CORPSE(i)) {
            act("$n savagely devours a corpse.", FALSE, ch, 0, 0, TO_ROOM);
            for (temp = i->contains; temp; temp = next_obj) {
                next_obj = temp->next_content;
                objectTakeFromObject(temp);
                objectPutInRoom(temp, ch->in_room);
            }
            objectExtract(i);
            return (TRUE);
        }
    }
    return (FALSE);
}

int fighter(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) == POSITION_FIGHTING) {
            FighterMove(ch);
        } else {
            StandUp(ch);
        }
        FindABetterWeapon(ch);
    }
    return (FALSE);
}

int geyser(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (number(0, 3) == 0) {
        act("You erupt.", 1, ch, 0, 0, TO_CHAR);
        cast_geyser(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, 0, 0);
        send_to_zone("The volcano rumbles slightly.\n\r", ch);
        return (TRUE);
    }
    return( FALSE );
}

int ghostsoldier(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *tch,
                   *good,
                   *master;
    int             max_good;
    int             (*gs) (),
                    (*gc) ();

    gs = ghostsoldier;
    gc = keystone;

    if (cmd) {
        return (FALSE);
    }
    if (time_info.hours > 4 && time_info.hours < 22) {
        act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
        extract_char(ch);
        return (TRUE);
    }

    max_good = -1001;
    good = 0;

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (!(mob_index[tch->nr].func == gs) &&
            !(mob_index[tch->nr].func == gc) &&
            GET_ALIGNMENT(tch) > max_good && !IS_IMMORTAL(tch) &&
            GET_RACE(tch) >= 4) {
            max_good = GET_ALIGNMENT(tch);
            good = tch;
        }
    }
    /*
     * What is a ghost Soldier doing in a peaceful room?
     */
    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    if (good) {
        if (!check_soundproof(ch)) {
            act("$N attacks you with an unholy scream!", FALSE, good, 0,
                ch, TO_CHAR);
        }
        hit(ch, good, TYPE_UNDEFINED);
        return (TRUE);
    }

    if (ch->specials.fighting) {
        if (IS_NPC(ch->specials.fighting) &&
            !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF) &&
            (master = (ch->specials.fighting)->master) && CAN_SEE(ch, master)) {
            stop_fighting(ch);
            hit(ch, master, TYPE_UNDEFINED);
        }

        if (GET_POS(ch) == POSITION_FIGHTING) {
            FighterMove(ch);
        } else {
            StandUp(ch);
        }
        CallForGuard(ch, ch->specials.fighting, 3, OUTPOST);
    }
    return (FALSE);
}

int ghoul(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    struct char_data *tar;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    tar = ch->specials.fighting;

    if (tar && tar->in_room == ch->in_room &&
        !IS_AFFECTED2(tar, AFF2_PROTECT_FROM_EVIL) &&
        !IS_AFFECTED(tar, AFF_SANCTUARY) &&
        HitOrMiss(ch, tar, CalcThaco(ch))) {

        act("$n touches $N!", 1, ch, 0, tar, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, tar, TO_VICT);
        if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
            cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tar, 0);
            return TRUE;
        }
    }
    return FALSE;
}

#define SHAMAN_NAME "shaman"

int golgar(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *shaman,
                   *tch;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (magic_user(ch, cmd, arg, mob, type));
    }

    if (!(shaman = get_char_room_vis(ch, SHAMAN_NAME))) {
        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (IS_NPC(tch) && GET_RACE(tch) == RACE_TROGMAN &&
                tch->specials.fighting && !IS_NPC(tch->specials.fighting)) {
                act("$n growls 'Death to those attacking my people!'",
                    FALSE, ch, 0, 0, TO_ROOM);
                hit(ch, tch->specials.fighting, TYPE_UNDEFINED);
                return (FALSE);
            }
        }

        if (number(0, 5) == 0) {
            act("$n slowly fades into ethereal emptiness.",
                FALSE, ch, 0, 0, TO_ROOM);
            extract_char(ch);
        }
    } else {
        if (!shaman->specials.fighting) {
            act("$n growls 'How dare you summon me!'",
                FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, shaman, TYPE_UNDEFINED);
            return (FALSE);
        } else {
            act("$n screams 'You dare touch my holy messenger!? DIE!'",
                FALSE, ch, 0, 0, TO_ROOM);
            hit(ch, shaman->specials.fighting, TYPE_UNDEFINED);
            return (FALSE);
        }
    }
    return( FALSE );
}

int green_slime(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *cons;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (cons = real_roomp(ch->in_room)->people; cons;
         cons = cons->next_in_room) {
        if (cons != ch  && !IS_IMMORTAL(cons)) {
            cast_green_slime(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             cons, 0);
        }
    }
    return( TRUE );
}

int guild_guard(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
    } else {
        switch (ch->in_room) {
        case 3017:
            return (CheckForBlockedMove(ch, cmd, arg, 3017, 2,
                                        CLASS_MAGIC_USER | CLASS_SORCERER));
            break;
        case 3004:
            return (CheckForBlockedMove(ch, cmd, arg, 3004, 0, CLASS_CLERIC));
            break;
        case 3027:
            return (CheckForBlockedMove(ch, cmd, arg, 3027, 1, CLASS_THIEF));
            break;
        case 3021:
            return (CheckForBlockedMove(ch, cmd, arg, 3021, 1, CLASS_WARRIOR));
            break;
        }
    }

    return FALSE;
}

int janitor(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct obj_data *i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
        if (IS_OBJ_STAT(i, wear_flags, ITEM_TAKE) &&
            (i->type_flag == ITEM_TYPE_DRINKCON || i->cost <= 10)) {
            act("$n picks up some trash.", FALSE, ch, 0, 0, TO_ROOM);
            objectTakeFromRoom(i);
            objectGiveToChar(i, ch);
            return (TRUE);
        }
    }
    return (FALSE);
}

int jugglernaut(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct obj_data *tmp_obj;
    int             i,
                    j;

    if (cmd) {
        return (FALSE);
    }
    if (GET_POS(ch) == POSITION_STANDING) {
        if (number(0, 2)) {
            return FALSE;
        }
        /*
         * juggle something
         */

        if (IS_CARRYING_N(ch) < 1) {
            return FALSE;
        }
        i = number(0, IS_CARRYING_N(ch));
        j = 0;
        for (tmp_obj = ch->carrying; (tmp_obj) && (j < i); j++) {
            tmp_obj = tmp_obj->next_content;
        }

        if (number(0,5)) {
            if (number(0,1)) {
                act("$n tosses $p high into the air and catches it.", TRUE,
                    ch, tmp_obj, NULL, TO_ROOM);
            } else {
                act("$n sends $p whirling.", TRUE, ch, tmp_obj, NULL, TO_ROOM);
            }
        } else {
            act("$n tosses $p but fumbles it!", TRUE, ch, tmp_obj, NULL,
                TO_ROOM);
            objectTakeFromChar(tmp_obj);
            objectPutInRoom(tmp_obj, ch->in_room);
        }
        return (TRUE);
    }
    return (FALSE);
}

int Keftab(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    int             found,
                    targ_item;
    struct char_data *i;

    if (cmd) {
        return (FALSE);
    }
    if (!ch->specials.hunting) {
        /*
         * find a victim
         */

        for (i = character_list; i; i = i->next) {
            if (!IS_NPC(ch)) {
                targ_item = SWORD_ANCIENTS;
                found = FALSE;
                while (!found) {
                    if ((HasObject(i, targ_item)) && (GetMaxLevel(i) < 30)) {
                        AddHated(ch, i);
                        SetHunting(ch, i);
                        return (TRUE);
                    } else {
                        targ_item++;
                        if (targ_item > SWORD_ANCIENTS + 20)
                            found = TRUE;
                    }
                }
            }
        }
        return (FALSE);
    }

    /*
     * check to make sure that the victim still has an item
     */
    found = FALSE;
    targ_item = SWORD_ANCIENTS;
    while (!found) {
        if (HasObject(ch->specials.hunting, targ_item)) {
            return (FALSE);
        } else {
            targ_item++;
            if (targ_item == SWORD_ANCIENTS + 20)
                found = FALSE;
        }
        ch->specials.hunting = 0;
        found = TRUE;
    }
    return (FALSE);
}

int mayor(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    static char     open_path[] =
        "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

    static char     close_path[] =
        "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

    static char    *path;
    static int      index;
    static bool     move = FALSE;

    if (type == EVENT_WINTER) {
        GET_POS(ch) = POSITION_STANDING;
        command_interpreter(ch, "shout Aieee!   The rats!  The rats are "
                                "coming!  Aieeee!");
        return (TRUE);
    }

    if (!move) {
        if (time_info.hours == 6) {
            move = TRUE;
            path = open_path;
            index = 0;
        } else if (time_info.hours == 20) {
            move = TRUE;
            path = close_path;
            index = 0;
        }
    }

    if (cmd || !move || GET_POS(ch) < POSITION_SLEEPING ||
        GET_POS(ch) == POSITION_FIGHTING) {
        if (!ch->specials.fighting) {
            return (FALSE);
        }
        return (fighter(ch, cmd, arg, mob, type));
    }

    switch (path[index]) {
    case '0':
    case '1':
    case '2':
    case '3':
        do_move(ch, NULL, path[index] - '0' + 1);
        break;

    case 'W':
        GET_POS(ch) = POSITION_STANDING;
        act("$n awakens and groans loudly.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'S':
        GET_POS(ch) = POSITION_SLEEPING;
        act("$n lies down and instantly falls asleep.", FALSE, ch, 0, 0,
            TO_ROOM);
        break;

    case 'a':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'Hello Honey!'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'b':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'What a view! I must get something done about that dump!'",
            FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'c':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'Vandals! Youngsters nowadays have no respect for "
            "anything!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'd':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'Good day, citizens!'", FALSE, ch, 0, 0, TO_ROOM);
        break;

    case 'e':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'I hereby declare the bazaar open!'", FALSE, ch, 0, 0,
            TO_ROOM);
        break;

    case 'E':
        if (check_soundproof(ch)) {
            return (FALSE);
        }
        act("$n says 'I hereby declare Midgaard closed!'", FALSE, ch, 0, 0,
            TO_ROOM);
        break;

    case 'O':
        command_interpreter(ch, "unlock gate");
        command_interpreter(ch, "open gate");
        break;

    case 'C':
        command_interpreter(ch, "close gate");
        command_interpreter(ch, "lock gate");
        break;

    case '.':
        move = FALSE;
        break;

    }

    index++;
    return FALSE;
}

int MidgaardCityguard(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    return (GenericCityguardHateUndead(ch, cmd, arg, mob, MIDGAARD));
}

int MordGuard(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    return (GenericCityguardHateUndead(ch, cmd, arg, mob, MORDILNIA));
}

int MordGuildGuard(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
    } else {
        if (cmd >= 1 && cmd <= 6) {
            switch (ch->in_room) {
            case 18266:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 18266, 2,
                         CLASS_MAGIC_USER | CLASS_SORCERER));
                break;
            case 18276:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 18276, 2, CLASS_CLERIC));
                break;
            case 18272:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 18272, 2, CLASS_THIEF));
                break;
            case 18256:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 18256, 0, CLASS_WARRIOR));
                break;
            }
        }
    }
    return (FALSE);
}

#define NTMOFFICE  13554
#define NTMNGATE   13422
#define NTMEGATE   13431
#define NTMSGATE   13413
#define NTMWGATE   13423

#define NTMWMORN    0
#define NTMSTARTM   1
#define NTMGOALNM   2
#define NTMGOALEM   3
#define NTMGOALSM   4
#define NTMGOALWM   5
#define NTMGOALOM   6
#define NTMWNIGHT   7
#define NTMSTARTN   8
#define NTMGOALNN   9
#define NTMGOALEN   10
#define NTMGOALSN   11
#define NTMGOALWN   12
#define NTMGOALON   13
#define NTMSUSP     14
#define NTM_FIX     15

int NewThalosMayor(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    int             dir;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!ch->specials.fighting) {
        /*
         * state info
         */
        switch (ch->generic) {
        case NTMWMORN:
            /*
             * wait for morning
             */
            if (time_info.hours == 6) {
                ch->generic = NTMGOALNM;
                return (FALSE);
            }
            break;
        case NTMGOALNM:
            /*
             * north gate
             */
            if (ch->in_room != NTMNGATE) {
                dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * unlock and open door.
                 */
                command_interpreter(ch, "unlock gate");
                command_interpreter(ch, "open gate");
                ch->generic = NTMGOALEM;
            }
            return (FALSE);
            break;
        case NTMGOALEM:
            if (ch->in_room != NTMEGATE) {
                dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * unlock and open door.
                 */
                command_interpreter(ch, "unlock gate");
                command_interpreter(ch, "open gate");
                ch->generic = NTMGOALSM;
            }
            return (FALSE);
            break;
        case NTMGOALSM:
            if (ch->in_room != NTMSGATE) {
                dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * unlock and open door.
                 */
                command_interpreter(ch, "unlock gate");
                command_interpreter(ch, "open gate");
                ch->generic = NTMGOALWM;
            }
            return (FALSE);
            break;
        case NTMGOALWM:
            if (ch->in_room != NTMWGATE) {
                dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * unlock and open door.
                 */
                command_interpreter(ch, "unlock gate");
                command_interpreter(ch, "open gate");
                ch->generic = NTMGOALOM;
            }
            return (FALSE);
            break;
        case NTMGOALOM:
            if (ch->in_room != NTMOFFICE) {
                dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                ch->generic = NTMWNIGHT;
            }
            return (FALSE);
            break;
        case NTMWNIGHT:
            /*
             * go back to wait for 7pm
             */
            if (time_info.hours == 19) {
                ch->generic = NTMGOALNN;
            }
        case NTMGOALNN:
            /*
             * north gate
             */
            if (ch->in_room != NTMNGATE) {
                dir = choose_exit_global(ch->in_room, NTMNGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * lock and open door.
                 */
                command_interpreter(ch, "lock gate");
                command_interpreter(ch, "close gate");
                ch->generic = NTMGOALEN;
            }
            return (FALSE);
            break;
        case NTMGOALEN:
            if (ch->in_room != NTMEGATE) {
                dir = choose_exit_global(ch->in_room, NTMEGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * lock and open door.
                 */
                command_interpreter(ch, "lock gate");
                command_interpreter(ch, "close gate");
                ch->generic = NTMGOALSN;
            }
            return (FALSE);
            break;
        case NTMGOALSN:
            if (ch->in_room != NTMSGATE) {
                dir = choose_exit_global(ch->in_room, NTMSGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * lock and open door.
                 */
                command_interpreter(ch, "lock gate");
                command_interpreter(ch, "close gate");
                ch->generic = NTMGOALWN;
            }
            return (FALSE);
            break;
        case NTMGOALWN:
            if (ch->in_room != NTMWGATE) {
                dir = choose_exit_global(ch->in_room, NTMWGATE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                /*
                 * unlock and open door.
                 */
                command_interpreter(ch, "lock gate");
                command_interpreter(ch, "close gate");
                ch->generic = NTMGOALOM;
            }
            return (FALSE);
            break;
        case NTMGOALON:
            if (ch->in_room != NTMOFFICE) {
                dir = choose_exit_global(ch->in_room, NTMOFFICE, -100);
                if (dir < 0) {
                    ch->generic = NTM_FIX;
                    return (FALSE);
                } else {
                    go_direction(ch, dir);
                }
            } else {
                ch->generic = NTMWMORN;
            }
            return (FALSE);
            break;
        case NTM_FIX:
            /*
             * move to correct spot (office)
             */
            command_interpreter(ch, "say Woah! How did i get here!");
            char_from_room(ch);
            char_to_room(ch, NTMOFFICE);
            ch->generic = NTMWMORN;
            return (FALSE);
            break;
        default:
            ch->generic = NTM_FIX;
            return (FALSE);
            break;
        }
    }
    return( FALSE );
}

int NewThalosGuildGuard(struct char_data *ch, int cmd, char *arg,
                        struct char_data *mob, int type)
{
    if (!cmd) {
        if (ch->specials.fighting) {
            return (fighter(ch, cmd, arg, mob, type));
        }
    } else {
        if (cmd >= 1 && cmd <= 6) {
            switch (ch->in_room) {
            case 13532:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 13532, 2, CLASS_THIEF));
                break;
            case 13512:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 13512, 2, CLASS_CLERIC));
                break;
            case 13526:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 13526, 2, CLASS_WARRIOR));
                break;
            case 13525:
                return (CheckForBlockedMove
                        (ch, cmd, arg, 13525, 0,
                         CLASS_MAGIC_USER | CLASS_SORCERER));
                break;
            }
        }
    }
    return (FALSE);
}

#define NN_LOOSE  0
#define NN_FOLLOW 1
#define NN_STOP   2

int NudgeNudge(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{

    struct char_data *vict;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (FALSE);
    }

    switch (ch->generic) {
    case NN_LOOSE:
        /*
         * find a victim
         */
        vict = FindVictim(ch);
        if (!vict) {
            return (FALSE);
        }
        /*
         * start following
         */
        if (circle_follow(ch, vict)) {
            return (FALSE);
        }
        if (ch->master) {
            stop_follower(ch);
        }
        add_follower(ch, vict);
        ch->generic = NN_FOLLOW;
        if (!check_soundproof(ch)) {
            command_interpreter(ch, "say Good Evenin' Squire!");
        }
        act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
        act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
        break;
    case NN_FOLLOW:
        switch (number(0, 20)) {
        case 0:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say Is your wife a goer?  Know what "
                                        "I mean, eh?");
            }
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            break;
        case 1:
            act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say Say no more!  Say no MORE!");
            }
            break;
        case 2:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say You been around, eh?");
                command_interpreter(ch, "say ...I mean you've ..... done it, "
                                        "eh?");
            }
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            break;
        case 3:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say A nod's as good as a wink to a "
                                        "blind bat, eh?");
            }
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n nudges you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n nudges $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            break;
        case 4:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say You're WICKED, eh!  WICKED!");
            }
            act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n winks at you.", FALSE, ch, 0, 0, TO_CHAR);
            act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            act("$n winks at $N.", FALSE, ch, 0, ch->master, TO_ROOM);
            break;
        case 5:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say Wink. Wink.");
            }
            break;
        case 6:
            if (!check_soundproof(ch)) {
                command_interpreter(ch, "say Nudge. Nudge.");
            }
            break;
        case 7:
        case 8:
            ch->generic = NN_STOP;
            break;
        default:
            break;
        }
        break;
    case NN_STOP:
        /*
         * Stop following
         */
        if (!check_soundproof(ch)) {
            command_interpreter(ch, "say Evening, Squire");
        }
        stop_follower(ch);
        ch->generic = NN_LOOSE;
        break;
    default:
        ch->generic = NN_LOOSE;
        break;
    }
    return (TRUE);
}

int PaladinGuildGuard(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            fighter(ch, cmd, arg, mob, type);
        }
    } else if (cmd >= 1 && cmd <= 6) {
        if ((cmd == 2) || (cmd == 1) || (cmd == 4)) {
            return (FALSE);
        }
        if (!HasClass(ch, CLASS_PALADIN)) {
            send_to_char("The guard shakes his head, and blocks your way.\n\r",
                         ch);
            act("The guard shakes his head, and blocks $n's way.", TRUE,
                ch, 0, 0, TO_ROOM);
            return (TRUE);
        }
    }
    return (FALSE);
}

int regenerator(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (cmd) {
        return (FALSE);
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
        GET_HIT(ch) += 9;
        GET_HIT(ch) = MIN(GET_HIT(ch), GET_MAX_HIT(ch));

        act("$n regenerates.", TRUE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    }
    return( FALSE );
}

#define ONE_RING 1105
int Ringwraith(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{

    if (!AWAKE(ch) || !IS_NPC(ch) || cmd) {
        return (FALSE);
    }

    if (ch->specials.fighting) {
        if (GET_POS(ch) > POSITION_SLEEPING &&
            GET_POS(ch) < POSITION_FIGHTING) {
            do_stand(ch, NULL, 0);
        } else {
            wraith(ch, cmd, arg, mob, type);
        }

        return (FALSE);
    }
    if (number(1, 25) == 1) {
        if (!check_soundproof(ch)) {
            act("$n says 'Give me The Ring *NOW*'.", FALSE, mob, NULL,
                NULL, TO_ROOM);
        } else {
            act("$n pokes you in the ribs very painfully.", FALSE, mob,
                NULL, NULL, TO_ROOM);
        }
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int RustMonster(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *vict;
    struct obj_data *t_item;
    int             t_pos;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * find a victim
     */
    if (ch->specials.fighting) {
        vict = ch->specials.fighting;
    } else {
        vict = FindVictim(ch);
        if (!vict) {
            return (FALSE);
        }
    }

    /*
     * choose an item of armor or a weapon that is metal
     * since metal isn't defined, we'll just use armor and weapons
     */

    /*
     * choose a weapon first, then if no weapon, choose a shield,
     * if no shield, choose breast plate, then leg plate, sleeves,
     * helm
     */

    if (vict->equipment[WIELD]) {
        t_item = vict->equipment[WIELD];
        t_pos = WIELD;
    } else if (vict->equipment[WEAR_SHIELD]) {
        t_item = vict->equipment[WEAR_SHIELD];
        t_pos = WEAR_SHIELD;
    } else if (vict->equipment[WEAR_BODY]) {
        t_item = vict->equipment[WEAR_BODY];
        t_pos = WEAR_BODY;
    } else if (vict->equipment[WEAR_LEGS]) {
        t_item = vict->equipment[WEAR_LEGS];
        t_pos = WEAR_LEGS;
    } else if (vict->equipment[WEAR_ARMS]) {
        t_item = vict->equipment[WEAR_ARMS];
        t_pos = WEAR_ARMS;
    } else if (vict->equipment[WEAR_HEAD]) {
        t_item = vict->equipment[WEAR_HEAD];
        t_pos = WEAR_HEAD;
    } else {
        return (FALSE);
    }

    /*
     * item makes save (or not)
     */
    if (DamageOneItem(vict, ACID_DAMAGE, t_item)) {
        t_item = unequip_char(vict, t_pos);
        if (t_item) {
            /*
             * if it doesn't make save, falls into a pile of scraps
             */
            MakeScrap(vict, NULL, t_item);
        }
    }
    return (FALSE);
}

int shadow(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_chill_touch(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                         ch->specials.fighting, 0);
        if (ch->specials.fighting) {
            cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                          ch->specials.fighting, 0);
        }
        return TRUE;
    }
    return FALSE;
}


#define DEITY 21124
#define DEITY_NAME "golgar"

int shaman(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *god,
                   *tch;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (!number(0, 3)) {
            for (tch = real_roomp(ch->in_room)->people; tch;
                 tch = tch->next_in_room) {
                if (!IS_NPC(tch) && GetMaxLevel(tch) > 20 && CAN_SEE(ch, tch)) {
                    if (!(god = get_char_room_vis(ch, DEITY_NAME))) {
                        act("$n screams 'Golgar, I summon thee to aid thy "
                            "servants!'", FALSE, ch, 0, 0, TO_ROOM);

                        if (!number(0, 8)) {
                            act("There is a blinding flash of light!",
                                FALSE, ch, 0, 0, TO_ROOM);
                            god = read_mobile(DEITY, VIRTUAL);
                            char_to_room(god, ch->in_room);
                        }
                    } else if (!number(0, 2)) {
                        act("$n shouts 'Now you will die!'", FALSE, ch, 0,
                            0, TO_ROOM);
                    }
                }
            }
        } else {
            return (cleric(ch, cmd, arg, mob, type));
        }
    }
    return( FALSE );
}

#define SISYPHUS_MAX_LEVEL 9

/*
 * This is the highest level of PC that can enter.  The highest level
 * monster currently in the section is 14th.  It should require a fairly
 * large party to sweep the section.
 */

int sisyphus(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    /*
     * use this as a switch, to avoid double challenges
     */
    static int      b = 1;
    char           *name;

    if (cmd) {
        if (cmd <= 6 && cmd >= 1 && IS_PC(ch)) {
            if (b) {
                b = 0;
                name = strdup(GET_NAME(ch));
                do_look(mob, name, 0);
                if( name ) {
                    free(name);
                }
            } else {
                b = 1;
            }
            if (ch->in_room == Ivory_Gate && cmd == 4 &&
                SISYPHUS_MAX_LEVEL < GetMaxLevel(ch) &&
                !IS_IMMORTAL(ch)) {
                if (!check_soundproof(ch)) {
                    act("$n tells you 'First you'll have to get past me!'",
                        TRUE, mob, 0, ch, TO_VICT);
                }
                act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
                return (TRUE);
            }
            return (FALSE);
        }

        /*
         * doorbash
         */
        if (cmd == 258) {
            act("$n tells you 'I don't think so.'", FALSE, mob, 0, ch, TO_VICT);
            return (TRUE);
        }

        return (FALSE);
    } else {
        if (ch->specials.fighting) {
            if (GET_POS(ch) < POSITION_FIGHTING &&
                GET_POS(ch) > POSITION_STUNNED) {
                StandUp(ch);
            } else {
                switch (number(1, 10)) {
                case 1:
                    command_interpreter(ch, "say heal");
                    break;
                case 2:
                    command_interpreter(ch, "say pzar");
                    break;
                default:
                    FighterMove(ch);
                    break;
                }
            }
            return (FALSE);
        }
    }
    return (FALSE);
}

int snake(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) != POSITION_FIGHTING) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n poisons $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n poisons you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                    ch->specials.fighting, 0);
        return TRUE;
    }
    return FALSE;
}

int StatTeller(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    int             choice;
    char            buf[200];
    extern struct str_app_type str_app[];

    if (cmd) {
        if (cmd == 56) {
            /*
             * buy
             *
             * for Ash's dam/hitroll teller
             */

            if (mob_index[mob->nr].vnum == 31818) {
                if (GET_GOLD(ch) < 10000) {
                    act("$c0013[$c0015$N$c0013] tells you 'You do not have the"
                        " money to pay me.\n\r", FALSE, ch, 0, mob, TO_CHAR);
                    return (TRUE);
                } else {
                    GET_GOLD(ch) -= 10000;
                }

                choice = number(0, 1);
                if (choice == 0) {
                    sprintf(buf, "$c0013[$c0015$N$c0013] tells you 'I sense "
                                 "your damroll is about %d.'",
                            ch->points.damroll +
                            str_app[STRENGTH_APPLY_INDEX(ch)].todam );
                } else {
                    sprintf(buf, "$c0013[$c0015$N$c0013] tells you 'I sense "
                                 "your hitroll is about %d.'",
                            ch->points.hitroll +
                            str_app[STRENGTH_APPLY_INDEX(ch)].tohit);
                }

                act(buf, FALSE, ch, 0, mob, TO_CHAR);
                act("$n gives some gold to $N", FALSE, ch, 0, mob, TO_ROOM);
                act("You give $N some gold.", FALSE, ch, 0, mob, TO_CHAR);
                return (TRUE);
            }

            /*
             * randomly tells a player 3 of his/her stats.. for a price
             */
            if (GET_GOLD(ch) < 1000) {
                send_to_char("You do not have the money to pay me.\n\r", ch);
                return (TRUE);
            } else {
                GET_GOLD(ch) -= 1000;
            }

            choice = number(0, 2);
            switch (choice) {
            case 0:
                sprintf(buf, "STR: %d, WIS: %d, DEX: %d\n\r", GET_STR(ch),
                        GET_WIS(ch), GET_DEX(ch));
                send_to_char(buf, ch);
                break;
            case 1:
                sprintf(buf, "INT: %d, DEX:  %d, CON: %d \n\r",
                        GET_INT(ch), GET_DEX(ch), GET_CON(ch));
                send_to_char(buf, ch);
                break;
            case 2:
                sprintf(buf, "CON: %d, INT: %d , WIS: %d \n\r",
                        GET_CON(ch), GET_INT(ch), GET_WIS(ch));
                send_to_char(buf, ch);
                break;
            default:
                send_to_char("We are experiencing Technical difficulties\n\r",
                             ch);
                return (TRUE);
            }
            return (TRUE);
        }
    } else {
        /*
         * in combat, issues a more potent curse.
         */
        if (ch->specials.fighting) {
            act("$n gives you the evil eye!  You feel your hitpoints ebbing "
                "away", FALSE, ch, 0, ch->specials.fighting, TO_VICT);
            act("$n gives $N the evil eye!  $N seems weaker!",
                FALSE, ch, 0, ch->specials.fighting, TO_NOTVICT);
            ch->specials.fighting->points.max_hit -= 10;
            ch->specials.fighting->points.hit -= 10;
        }
    }
    return (FALSE);
}

int StormGiant(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if ((GET_POS(ch) < POSITION_FIGHTING) &&
            (GET_POS(ch) > POSITION_STUNNED)) {
            StandUp(ch);
        } else {
            if (number(0, 5)) {
                fighter(ch, cmd, arg, mob, type);
            } else {
                act("$n creates a lightning bolt", TRUE, ch, 0, 0,
                    TO_ROOM);
                if ((vict = FindAHatee(ch)) == NULL) {
                    vict = FindVictim(ch);
                }
                if (!vict) {
                    return (FALSE);
                }
                cast_lightning_bolt(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, vict, 0);
            }
        }
    }
    return (FALSE);
}

int SultanGuard(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    return (GenericCityguard(ch, cmd, arg, mob, NEWTHALOS));
}

int temple_labrynth_liar(struct char_data *ch, int cmd, char *arg,
                         struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (0);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    switch (number(0, 15)) {
    case 0:
        command_interpreter(ch, "say I'd go west if I were you.");
        return (TRUE);
    case 1:
        command_interpreter(ch, "say I heard that Vile is a cute babe.");
        return (TRUE);
    case 2:
        command_interpreter(ch, "say Going east will avoid the beast!");
        return (TRUE);
    case 4:
        command_interpreter(ch, "say North is the way to go.");
        return (TRUE);
    case 6:
        command_interpreter(ch, "say Dont dilly dally go south.");
        return (TRUE);
    case 8:
        command_interpreter(ch, "say Great treasure lies ahead");
        return (TRUE);
    case 10:
        command_interpreter(ch, "say I wouldn't kill the sentry if I were more"
                                " than level 9.  No way!");
        return (TRUE);
    case 12:
        command_interpreter(ch, "say I am a very clever liar.");
        return (TRUE);
    case 14:
        command_interpreter(ch, "say Loki is a really great guy!");
        command_interpreter(ch, "say Well.... maybe not...");
        return (TRUE);
    default:
        command_interpreter(ch, "say Then again I could be wrong!");
        return (TRUE);
    }
}

int temple_labrynth_sentry(struct char_data *ch, int cmd, char *arg,
                           struct char_data *mob, int type)
{
    struct char_data *tch;
    int             counter;

    if (cmd || !AWAKE(ch) || GET_POS(ch) != POSITION_FIGHTING ||
        !ch->specials.fighting || check_soundproof(ch)) {
        return (FALSE);
    }

    /*
     * Find a dude to do very evil things upon !
     */

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (GetMaxLevel(tch) > 10 && CAN_SEE(ch, tch)) {
            act("The sentry snaps out of his trance and ...", 1, ch, 0, 0,
                TO_ROOM);
            command_interpreter(ch, "say You will die for your insolence, "
                                    "pig-dog!");
            for (counter = 0; counter < 4; counter++) {
                if (check_nomagic(ch, 0, 0)) {
                    return (FALSE);
                }
            }
            if (GET_POS(tch) > POSITION_SITTING) {
                cast_fireball(15, ch, "", SPELL_TYPE_SPELL, tch, 0);
            } else {
                return TRUE;
            }
            return TRUE;
        } else {
            act("The sentry looks concerned and continues to push you away",
                1, ch, 0, 0, TO_ROOM);
            command_interpreter(ch, "say Leave me alone. My vows do not permit "
                                    "me to kill you!");
        }
    }
    return TRUE;
}

int thief(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    struct char_data *cons;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) != POSITION_STANDING) {
        return (FALSE);
    }
    for (cons = real_roomp(ch->in_room)->people; cons;
         cons = cons->next_in_room) {
        if (cons != ch && !IS_IMMORTAL(cons) && !number(0, 4)) {
            if (ch->master && cons != ch->master && 
                !in_group(ch->master, cons)) {
                npc_steal(ch, cons);
            }
        }
    }

    return (FALSE);
}

int ThrowerMob(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;

    /*
     * Throws people in various directions
     */

    if (!cmd) {
        if (AWAKE(ch) && ch->specials.fighting) {
            /*
             * take this person and throw them
             */
            vict = ch->specials.fighting;
            switch (ch->in_room) {
            case 13912:
                ThrowChar(ch, vict, 1);
                /*
                 * throw chars to the east
                 */
                break;
            default:
                break;
            }
        }
    } else {
        switch (ch->in_room) {
        case 13912:
            if (cmd == 1) {
                /*
                 * north+1
                 */
                send_to_char("The Troll blocks your way.\n", ch);
                return (TRUE);
            }
            break;
        default:
            break;
        }
    }
    return (FALSE);
}

int tormentor(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    /* Is in hell, blocks all commands except if the char is an immortal */
    if (!cmd || IS_IMMORTAL(ch) || !IS_PC(ch)) {
        return (FALSE);
    }

    return (TRUE);
}

int trapper(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *tch;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * Okay, the idea is this: If the PC or NPC in this room isn't flying,
     * it is walking on the trapper. Doesn't matter if it's sneaking, or
     * invisible, or whatever. The trapper will attack both PCs and NPCs,
     * so don't have a lot of wandering NPCs around it.
     */

    if (!ch->specials.fighting) {
        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (ch != tch && !IS_IMMORTAL(tch) &&
                !IS_AFFECTED(tch, AFF_FLYING)) {
                set_fighting(ch, tch);
                return (TRUE);
            }
        }
        /*
         * Nobody here
         */
        return (FALSE);
    } else {
        if (GetMaxLevel(ch->specials.fighting) > MAX_MORT) {
            return (FALSE);
        }

        /*
         * Equipment must save against crush - will fail 25% of the time
         */
        DamageStuff(ch->specials.fighting, TYPE_CRUSH, number(0, 7));

        /*
         * Make the poor sucker save against paralzyation, or suffocate
         */
        if (saves_spell(ch->specials.fighting, SAVING_PARA)) {
            act("You can hardly breathe, $N is suffocating you!",
                FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
            act("$N is stifling $n, who will suffocate soon!",
                FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
            return (FALSE);
        } else {
            act("You gasp for air inside $N!",
                FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
            act("$N stifles you. You asphyxiate and die!",
                FALSE, ch->specials.fighting, 0, ch, TO_CHAR);
            act("$n has suffocated inside $N!",
                FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
            act("$n is dead!", FALSE, ch->specials.fighting, 0, ch, TO_ROOM);
            Log("%s has suffocated to death.", GET_NAME(ch->specials.fighting));

            die(ch->specials.fighting, '\0');
            ch->specials.fighting = 0x0;
            return (TRUE);
        }
    }
}

int trogcook(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *tch;
    struct obj_data *corpse;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) != POSITION_FIGHTING) {
            StandUp(ch);
        }
        return (FALSE);
    }

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (IS_NPC(tch) && IsAnimal(tch) && CAN_SEE(ch, tch)) {
            if (!check_soundproof(ch)) {
                act("$n cackles 'Something else for the pot!'", FALSE, ch,
                    0, 0, TO_ROOM);
            }
            hit(ch, tch, TYPE_UNDEFINED);
            return (TRUE);
        }
    }

    corpse = get_obj_in_list_vis(ch, "corpse",
                                 real_roomp(ch->in_room)->contents);

    if (corpse) {
        command_interpreter(ch, "get corpse");
        act("$n cackles 'Into the soup with it!'", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "put corpse pot");
        return (TRUE);
    }
    return( FALSE );
}



int troguard(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *tch,
                   *good;
    int             max_good;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) == POSITION_FIGHTING) {
            FighterMove(ch);
        } else {
            StandUp(ch);
        }

        if (!check_soundproof(ch)) {
            act("$n shouts 'The enemy is upon us! Help me, my brothers!'",
                TRUE, ch, 0, 0, TO_ROOM);
            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, TROGCAVES);
            }
            return (TRUE);
        }
    }

    max_good = -1001;
    good = 0;

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (GET_ALIGNMENT(tch) > max_good && !IS_IMMORTAL(tch) &&
            GET_RACE(tch) != RACE_TROGMAN && GET_RACE(tch) != RACE_ARACHNID) {
            max_good = GET_ALIGNMENT(tch);
            good = tch;
        }
    }

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    if (good) {
        if (!check_soundproof(ch)) {
            act("$n screams 'Die invading scum! Take that!'",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        hit(ch, good, TYPE_UNDEFINED);
        return (TRUE);
    }

    return (FALSE);
}

int Tyrannosaurus_swallower(struct char_data *ch, int cmd, char *arg,
                            struct char_data *mob, int type)
{
    struct obj_data *co,
                   *o;
    struct char_data *targ;
    struct room_data *rp;
    int             i,
                    index;

    if (cmd && cmd != 156) {
        return (FALSE);
    }
    if (cmd == 156) {
        send_to_char("You're much too afraid to steal anything!\n\r", ch);
        return (TRUE);
    }


    /*
     * swallow
     */
    if (AWAKE(ch) && (targ = FindAnAttacker(ch)) != '\0') {
        act("$n opens $s gaping mouth", TRUE, ch, 0, 0, TO_ROOM);
        if (!CAN_SEE(ch, targ) && saves_spell(targ, SAVING_PARA)) {
            act("$N barely misses being swallowed whole!",
                FALSE, ch, 0, targ, TO_NOTVICT);
            act("You barely avoid being swallowed whole!",
                FALSE, ch, 0, targ, TO_VICT);
            return (FALSE);
        }

        if (!saves_spell(targ, SAVING_PARA)) {
            act("In a single gulp, $N is swallowed whole!",
                TRUE, ch, 0, targ, TO_ROOM);
            send_to_char("In a single gulp, you are swallowed whole!\n\r",
                 targ);
            send_to_char("The horror!  The horror!\n\r", targ);
            send_to_char("MMM.  yum!\n\r", ch);
            /*
             * kill target:
             */
            GET_HIT(targ) = 0;
            Log("%s killed by being swallowed whole", GET_NAME(targ));
            die(targ, '\0');

            /*
             * all stuff to monster: this one is tricky.  assume that
             * corpse is top item on item_list now that corpse has
             * been made.
             */
            rp = real_roomp(ch->in_room);
            if (!rp)
                return (FALSE);

            for (co = rp->contents; co; co = co->next_content) {
                if (IS_CORPSE(co)) {
                    /*
                     * assume 1st corpse is victim's
                     */
                    while (co->contains) {
                        o = co->contains;
                        objectTakeFromObject(o);
                        objectGiveToChar(o, ch);

                        if (ITEM_TYPE(o) == ITEM_TYPE_POTION) {
                            /*
                             * do the effects of the potion:
                             */
                            for (i = 1; i < 4; i++) {
                                if (o->value[i] >= 1) {
                                    index = spell_index[o->value[i]];
                                    if( spell_info[index].spell_pointer ) {
                                        ((*spell_info[index].spell_pointer)
                                         ((byte)o->value[0], ch,
                                          "", SPELL_TYPE_POTION, ch, o));
                                    }
                                }
                            }
                            objectExtract(o);

                        }
                    }
                    objectExtract(co);
                    return (TRUE);
                }
            }
        } else {
            act("$N barely misses being swallowed whole!", FALSE, ch,
                0, targ, TO_NOTVICT);
            act("You barely avoid being swallowed whole!", FALSE, ch,
                0, targ, TO_VICT);
        }
    }
    return( FALSE );
}

char           *quest_one[] = {
    "say The second artifact you must find is the ring of Tlanic.",
    "say Tlanic was an elven warrior who left Rhyodin five years after",
    "say Lorces; he also was given an artifact to aid him.",
    "say He went to find out what happened to Lorces, his friend, and to",
    "say find a way to the north if he could.",
    "say He also failed. Return the ring to me for further instructions."
};

char           *quest_two[] = {
    "say When Tlanic had been gone for many moons; his brother Evistar",
    "say went to find him.",
    "say Evistar, unlike his brother, was not a great warrior, but he was",
    "say the finest tracker among our people.",
    "say Given to him as an aid to his travels, was a neverempty chalice.",
    "say Bring this magical cup to me, if you wish to enter the kingdom",
    "say of the Rhyodin."
};

char           *quest_three[] = {
    "say When Evistar did not return, ages passed before another left.",
    "say A mighty wizard was the next to try. His name was C*zarnak.",
    "say It is feared that he was lost in the deep caves, like the others.",
    "say He wore an enchanted circlet on his brow.",
    "say Find it and bring it to me, and I will tell you more."
};

char           *necklace[] = {
    "say You have brought me all the items lost by those who sought the",
    "say path from the kingdom to the outside world.",
    "say Furthermore, you have found the way through the mountains",
    "say yourself, proving your ability to track and map.",
    "say You are now worthy to be an ambassador to my kingdom.",
    "say Take this necklace, and never part from it!",
    "say Give it to the gatekeeper, and he will let you pass.",
};

char           *nonecklace[] = {
    "say You have brought me all the items lost by those who sought the",
    "say path from the kingdom to the outside world.",
    "say Furthermore, you have found the way through the mountains",
    "say yourself, proving your ability to track and map.",
    "say You are now worthy to be an ambassador to my kingdom.",
    "say Your final quest is to obtain the Necklace of Wisdom.",
    "say The gatekeeper will recognize it, and let you pass.",
};

char           *quest_intro[] = {
    "say My name is Valik, and I am the Lorekeeper of the Rhyodin.",
    "say Rhyodin is kingdom southeast of the Great Eastern Desert.",
    "say To enter the kingdom of Rhyodin, you must first pass this test.",
    "say When the only route to the kingdom collapsed, commerce with",
    "say the outside world was cutoff.",
    "say Lorces was the first to try to find a path back to the outside,",
    "say and for his bravery to him a shield was given.",
    "say The first step towards entry to the kingdom of Rhyodin is to",
    "say return to me the shield of Lorces.",
};


#define Valik_Wandering   0
#define Valik_Meditating  1
#define Valik_Qone        2
#define Valik_Qtwo        3
#define Valik_Qthree      4
#define Valik_Qfour       5
#define Shield            21113
#define Ring              21120
#define Chalice           21121
#define Circlet           21117
#define Necklace          21122
#define Med_Chambers      21324

/**
 * @todo make quest_lines and valik_dests preloaded static arrays!
 */
int Valik(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];
    int             i;
    struct char_data *vict,
                   *master;
    struct obj_data *obj = NULL;
    int             (*valik) ();
    bool            gave_this_click = FALSE;
    short           quest_lines[4];
    short           valik_dests[9];

    if ((cmd && cmd != 72 && cmd != 86) || !AWAKE(ch)) {
        return (FALSE);
    }

    quest_lines[0] = 6;
    quest_lines[1] = 7;
    quest_lines[2] = 5;
    quest_lines[3] = 7;

    valik_dests[0] = 104;
    valik_dests[1] = 1638;
    valik_dests[2] = 7902;
    valik_dests[3] = 13551;
    valik_dests[4] = 16764;
    valik_dests[5] = 17330;
    valik_dests[6] = 19244;
    valik_dests[7] = 21325;
    valik_dests[8] = 25230;

    if (!cmd) {
        if (ch->specials.fighting) {
            if (IS_NPC(ch->specials.fighting) &&
                !IS_SET((ch->specials.fighting)->specials.act, ACT_POLYSELF) &&
                (master = (ch->specials.fighting)->master) &&
                CAN_SEE(ch, master)) {

                stop_fighting(ch);
                hit(ch, master, TYPE_UNDEFINED);
            }
            return (magic_user(ch, cmd, arg, mob, type));
        }
    }

    vict = FindMobInRoomWithFunction(ch->in_room, Valik);
    assert(vict != 0);

    valik = Valik;

    switch (vict->generic) {
    case Valik_Wandering:
    case Valik_Qone:
    case Valik_Qtwo:
    case Valik_Qthree:
        if (cmd == 72) {
            /*
             * give
             */
            arg = get_argument(arg, &obj_name);
            if (!obj_name ||
                !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
                return (FALSE);

            vict_name = skip_spaces(arg);
            if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)))
                return (FALSE);

            /*
             * the target is valik
             */
            if (mob_index[vict->nr].func == valik) {
                act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
                act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
            } else {
                return (FALSE);
            }
            gave_this_click = TRUE;
        } else if (cmd == 86) {
            /*
             * ask
             */
            arg = get_argument(arg, &vict_name);
            if (!vict_name || !(vict = get_char_room_vis(ch, vict_name))) {
                return (FALSE);
            }

            if (mob_index[vict->nr].func != valik) {
                return (FALSE);
            } else {
                if (!(strcmp(arg, " What is the quest of the Rhyodin?"))) {
                    for (i = 0; i < 9; ++i) {
                        sprintf(buf, "tell %s %s", GET_NAME(ch),
                                quest_intro[i]);
                        command_interpreter(vict, buf);
                    }
                }
                return (TRUE);
            }
        }
        break;
    case Valik_Meditating:
        if (time_info.hours < 22 && time_info.hours > 5) {
            do_stand(ch, NULL, -1);
            act("$n says 'Perhaps today will be different.'", FALSE, ch, 0,
                0, TO_ROOM);
            act("$n slowly fades out of existence.", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, valik_dests[number(0, 8)]);

            act("The world warps, dissolves, and reforms.", FALSE, ch, 0,
                0, TO_ROOM);
            ch->generic = Valik_Wandering;
            return (FALSE);
        } else {
            for (vict = real_roomp(ch->in_room)->people; vict;
                 vict = vict->next_in_room) {
                if (ch != vict && !IS_NPC(vict) && !IS_IMMORTAL(vict) && 
                    !number(0, 3)) {
                    act("$n snaps out of his meditation.", FALSE, ch, 0, 0,
                        TO_ROOM);
                    do_stand(ch, NULL, -1);
                    hit(ch, vict, TYPE_UNDEFINED);
                    return (FALSE);
                }
            }
            return (FALSE);
        }
        break;
    default:
        ch->generic = Valik_Wandering;
        return (FALSE);
    }

    /*
     * There are four valid objects
     */
    switch (vict->generic) {
    case Valik_Wandering:
        if (gave_this_click) {
            /*
             * Take it, in either case
             */
            objectTakeFromChar(obj);
            objectGiveToChar(obj, vict);
            if ( obj->item_number == Shield) {
                if (!check_soundproof(ch)) {
                    act("$N says 'The Shield of Lorces!'", FALSE, ch, 0,
                        vict, TO_CHAR);
                    act("$N says 'You may now undertake the first quest.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                }
                vict->generic = Valik_Qone;
            } else {
                act("$N takes the $p and bows in thanks.'",
                    FALSE, ch, obj, vict, TO_CHAR);
                act("$N takes the $p from $n and bows in thanks.'",
                    FALSE, ch, obj, vict, TO_ROOM);
                return (TRUE);
            }
        } else if (cmd) {
            return (FALSE);
        } else {
            if (time_info.hours > 21) {
                if (!check_soundproof(ch)) {
                    act("$n says 'It is time to meditate.'", FALSE, ch, 0,
                        0, TO_ROOM);
                }
                act("$n disappears in a flash of light!", FALSE, ch, 0, 0,
                    TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, Med_Chambers);

                act("Reality warps and spins around you!", FALSE, ch, 0, 0,
                    TO_ROOM);
                sprintf(buf, "close mahogany");
                command_interpreter(ch, buf);
                do_rest(ch, NULL, -1);
                ch->generic = Valik_Meditating;
                return (FALSE);
            }
            return (magic_user(ch, cmd, arg, mob, type));
        }
        break;
    case Valik_Qone:
        if (gave_this_click) {
            if (obj->item_number == Ring) {
                if (!check_soundproof(ch)) {
                    act("$N says 'You have brought me the ring of Tlanic.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                    act("$N says 'You may now undertake the second quest.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                }
                objectTakeFromChar(obj);
                objectGiveToChar(obj, vict);
                vict->generic = Valik_Qtwo;
            } else {
                act("$N shakes his head - it is the wrong item.",
                    FALSE, ch, 0, vict, TO_CHAR);
                act("$N gives $p back to you.", TRUE, ch, obj, vict,
                    TO_CHAR);
                act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
        break;
    case Valik_Qtwo:
        if (gave_this_click) {
            if (obj->item_number == Chalice) {
                if (!check_soundproof(ch)) {
                    act("$N says 'You have brought me the chalice of Evistar.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                    act("$N says 'You may now undertake the third quest.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                }
                objectTakeFromChar(obj);
                objectGiveToChar(obj, vict);
                vict->generic = Valik_Qthree;
            } else {
                act("$N shakes his head - it is the wrong item.'",
                    FALSE, ch, 0, 0, TO_CHAR);
                act("$N gives $p back to you.", TRUE, ch, obj, vict,
                    TO_CHAR);
                act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
        break;
    case Valik_Qthree:
        if (gave_this_click) {
            if (obj->item_number == Circlet) {
                if (!check_soundproof(ch)) {
                    act("$N says 'You have brought me the circlet of "
                        "C*zarnak.'", FALSE, ch, 0, vict, TO_CHAR);
                    act("$N says 'You may now undertake the final quest.'",
                        FALSE, ch, 0, vict, TO_CHAR);
                }
                objectTakeFromChar(obj);
                objectGiveToChar(obj, vict);
                vict->generic = Valik_Qfour;
            } else {
                act("$N says 'That is not the item I require.'",
                    FALSE, ch, 0, vict, TO_CHAR);
                act("$N gives $p back to you.", TRUE, ch, obj, vict, TO_CHAR);
                act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
        break;
    default:
        return (FALSE);
    }

    /*
     * The final switch, where we tell the player what the quests are
     */
    switch (vict->generic) {
    case Valik_Qone:
        for (i = 0; i < quest_lines[vict->generic - 2]; ++i)
            command_interpreter(vict, quest_one[i]);
        break;
    case Valik_Qtwo:
        for (i = 0; i < quest_lines[vict->generic - 2]; ++i)
            command_interpreter(vict, quest_two[i]);
        break;
    case Valik_Qthree:
        for (i = 0; i < quest_lines[vict->generic - 2]; ++i)
            command_interpreter(vict, quest_three[i]);
        break;
    case Valik_Qfour:
        if (vict->equipment[WEAR_NECK_1] &&
            vict->equipment[WEAR_NECK_1]->item_number == Necklace) {

            for (i = 0; i < quest_lines[vict->generic - 2]; ++i) {
                command_interpreter(vict, necklace[i]);
            }

            act("$N takes the Necklace of Wisdom and hands it to you.",
                FALSE, ch, 0, vict, TO_CHAR);
            act("$N takes the Necklace of Wisdom and hands it to $n.",
                FALSE, ch, 0, vict, TO_ROOM);
            objectGiveToChar(unequip_char(vict, WEAR_NECK_1), ch);
        } else {
            for (i = 0; i < quest_lines[vict->generic - 2]; ++i) {
                command_interpreter(vict, nonecklace[i]);
            }
        }
        vict->generic = Valik_Wandering;
        break;
    default:
        Log("Ack! Foo! Heimdall screws up!");
        return (FALSE);
    }
    return (TRUE);
}

int vampire(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                          ch->specials.fighting, 0);

        if (ch->specials.fighting &&
            ch->specials.fighting->in_room == ch->in_room) {
            cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              ch->specials.fighting, 0);
        }
        return TRUE;
    }
    return FALSE;
}

int web_slinger(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *vict;

    if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (GET_POS(ch) > POSITION_STUNNED && GET_POS(ch) < POSITION_FIGHTING) {
        StandUp(ch);
        return (TRUE);
    }

    /*
     * Find a dude to to evil things upon !
     */
    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    if (!number(0, 3)) {
        /*
         * Noop, for now
         */
        act("$n casts sticky webs upon you!", TRUE, ch, 0, 0, TO_ROOM);
        cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return( TRUE );
    }
    return( FALSE );
}

int wraith(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                          ch->specials.fighting, 0);
        return TRUE;
    }
    return FALSE;
}

/*
 * Avatar of Blibdoolpoolp - will shout every hour (approx) 
 */
int ABShout(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    static time_t   time_diff = 0;
    static time_t   last_time = 0;

    if (cmd) {
        return (FALSE);
    }
    /*
     * If it has been an hour, there is a 33% chance he will shout 
     */
    if (time_diff > 3600 && !number(0, 2)) {
        time_diff = 0;          
        /* 
         * reset 
         */
        last_time = time(NULL);
        command_interpreter(ch, "shout You puny mortals, come release me!  I "
                                "have a pest dragon whom imprisoned me to "
                                "slay, and some sea elves to boot!");
        return (TRUE);
    } else {
        /* 
         * not been an hour, update how long it has been 
         */
        time_diff = time(NULL) - last_time;
    }

    return (FALSE);
}

/*
 * Avatar of Posereisn 
 */
int AvatarPosereisn(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                   *temp,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0;
    static time_t   time_diff = 0;
    static time_t   last_time = 0;

    if (!cmd) {
        /*
         * If it has been an hour, there is a 33% chance he will shout 
         */
        if ((time_diff > 3600) && (!number(0, 2))) {
            time_diff = 0;      
            /* 
             * reset 
             */
            last_time = time(NULL);
            command_interpreter(ch, "shout Might a noble mortal bring me what "
                                    "is rightfully mine?  You shall be "
                                    "generously rewarded!");
            return (TRUE);
        } 
        
        time_diff = time(NULL) - last_time;
    }

    if (!AWAKE(ch)) {
        return (FALSE);
    }

    if (cmd == 72) {
        /* 
         * give 
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name) {
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (!(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (IS_PC(vict)) {
            return (FALSE);
        }
        
        /*
         * the target is not the Avatar of Posereisn or is a PC 
         */
        if (mob_index[vict->nr].vnum != 28042) {
            return (FALSE);
        }
        
        /*
         * The object is not the Ankh of Posereisn 
         */
        if (!IS_IMMORTAL(ch)) {
            if (obj->item_number != 28180) {
                sprintf(buf, "tell %s That is not the item I seek.",
                        GET_NAME(ch));
                command_interpreter(vict, buf);
                return (TRUE);
            }
        } else {
            sprintf(buf, "give %s %s", obj_name, vict_name);
            command_interpreter(ch, buf);
            if (obj->item_number == 28180) {
                test = 1;
            }
        }

        if (!IS_IMMORTAL(ch)) {
            test = 1;
            temp = strdup("Ankh-Posereisn Avatar-Posereisn");
            do_give(ch, temp, 0);
            if( temp ) {
                free( temp );
            }
        }
    } else
        return (FALSE);

    if (test == 1) {
        /* 
         * It is the Avatar, and the Ankh 
         */
        if (vict->equipment[WIELD]) {
            /* 
             * This is only done if he is wielding hellreaper 
             */
            sprintf(buf, "tell %s Thank you mighty hero.  Take this as a "
                         "token of my appreciation.", GET_NAME(ch));
            command_interpreter(vict, buf);

            temp = KeywordsToString( &vict->equipment[WIELD]->keywords, NULL );
            sprintf(buf, "remove %s", temp);
            free( temp );

            command_interpreter(vict, buf);
            sprintf(buf, "give Hellreaper %s", GET_NAME(ch));
            command_interpreter(vict, buf);
            return (TRUE);
        } 
        
        /* 
         * This is done if he is not wielding hellreaper 
         */
        sprintf(buf, "tell %s You are indeed a mighty hero, but I cannot "
                     "take this, for I have nothing to offer you in "
                     "return.", GET_NAME(ch));
        command_interpreter(vict, buf);
        sprintf(buf, "give Ankh-Posereisn %s", GET_NAME(ch));
        command_interpreter(vict, buf);
        return (TRUE);
    }
    return( FALSE );
}

int AbyssGateKeeper(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            fighter(ch, cmd, arg, mob, type);
        }
    } else if (cmd == 5) {
        send_to_char("The gatekeeper shakes his head, and blocks your way.\n\r",
                     ch);
        act("The guard shakes his head, and blocks $n's way.", TRUE, ch, 0,
            0, TO_ROOM);
        return (TRUE);
    }
    return (FALSE);
}


int astral_destination[] = {
    41925,     /* mob 2715 */
    21108,     /* mob 2716 */
    1474,      /* ... */
    1633,
    4109,
    5000,
    5126,
    5221,
    6513,
    7069,
    6601,
    9359,
    13809,
    16925,
    20031,
    27431,
    21210,
    25041,    /* mob 2732 */
    26109,    /* mob 2733 */
    18233,
    15841,
    13423,
    44980,    /* mob 2737 */
    31908,    /* Ash's winterfell */
    49302    /* Tarantis Bazaar */
};
int astral_dest_count = NELEMENTS(astral_destination);

int astral_portal(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    char           *arg1;
    int             i;
    int             j;
    struct char_data *portal;

    /*
     * To add another color pool, create another mobile (2733, etc) and
     * add another destination.  
     */

    if (cmd != 7) {
        /* 
         * enter 
         */
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1 && (is_abbrev(arg1, "pool") || is_abbrev(arg1, "color pool")) && 
        (portal = get_char_room("color pool", ch->in_room))) {
        i = mob_index[portal->nr].vnum - AST_MOB_NUM;
        if( i < 0 || i >= astral_dest_count ) {
            Log( "Astral destination bugger-up: index %d", i );
            return(FALSE);
        }

        j = astral_destination[i];
        if (j > 0 && j < 50000) {
            send_to_char("\n\r", ch);
            send_to_char("You attempt to enter the pool, and it gives.\n\r",
                         ch);
            send_to_char("You press on further and the pool surrounds you, "
                         "like some soft membrane.\n\r", ch);
            send_to_char("There is a slight wrenching sensation, and then the "
                         "color disappears.\n\r", ch);
            send_to_char("\n\r", ch);
            act("$n enters a color and disappears!", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, j);

            act("$n appears in a dazzling explosion of light!",
                FALSE, ch, 0, 0, TO_ROOM);
            command_interpreter(ch, "look");
            return (TRUE);
        }
    }
    return (FALSE);
}

int attack_rats(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             dir;

    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == EVENT_WINTER) {
        ch->generic = ATTACK_ROOM + number(0, 26);
        /* 
         * so they'll attack beggars, etc 
         */
        AddHatred(ch, OP_RACE, RACE_HUMAN);
    }

    if (type == EVENT_SPRING) {
        ch->generic = 0;
    }
    if (ch->generic == 0) {
        return (FALSE);
    }
    dir = choose_exit_global(ch->in_room, ch->generic, MAX_ROOMS);
    if (dir == -1) {
        /* 
         * assume we found it.. start wandering 
         */
        ch->generic = 0;
        /* 
         * We Can't Go Anywhere. 
         */
        return (FALSE);
    }

    go_direction(ch, dir);
    return( TRUE );
}

int board_ship(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    int             j;
    char           *arg1;
    struct char_data *ship;

    if (cmd != 620) {
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1) {
        if ((!strcasecmp("ship", arg1) || !strcasecmp("corsair", arg1) || 
             !strcasecmp("corsair ship", arg1)) &&  
            (ship = get_char_room("corsair ship", ch->in_room))) {
            j = 32033;

            send_to_char("You enter the ship.\n\r", ch);

            act("$n enters the ship.", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, j);

            act("Walks onto the ship.", FALSE, ch, 0, 0, TO_ROOM);
            do_look(ch, NULL, 0);
            return (TRUE);
        }
    }
    return (FALSE);
}

static funcp    breaths[] = {
    (funcp)cast_acid_breath, NULL, (funcp)cast_frost_breath, NULL, 
    (funcp)cast_lightning_breath, NULL, (funcp)cast_fire_breath, NULL,
    (funcp)cast_acid_breath, (funcp)cast_fire_breath, 
    (funcp)cast_lightning_breath, NULL
};

struct breather breath_monsters[] = {
    {230, 55, breaths + 0},
    {233, 55, breaths + 4},
    {243, 55, breaths + 2},
    {3670, 30, breaths + 2},
    {3674, 45, breaths + 6},
    {3675, 45, breaths + 8},
    {3676, 30, breaths + 6},
    {3952, 20, breaths + 8},
    {5005, 55, breaths + 4},
    {6112, 55, breaths + 4},
    {6635, 55, breaths + 0},
    {6609, 30, breaths + 0},
    {6642, 45, breaths + 2},
    {6801, 55, breaths + 2},
    {6802, 55, breaths + 2},
    {6824, 55, breaths + 0},
    {7040, 55, breaths + 6},
    {9217, 45, breaths + 4},
    {9418, 45, breaths + 2},
    {9419, 45, breaths + 2},
    {9727, 30, breaths + 0},
    {12010, 45, breaths + 6},
    {12011, 45, breaths + 6},
    {15858, 45, breaths + 0},
    {15879, 30, breaths + 0},
    {16620, 45, breaths + 0},
    {16700, 45, breaths + 4},
    {16738, 75, breaths + 6},
    {18003, 20, breaths + 8},
    {20002, 55, breaths + 6},
    {20017, 55, breaths + 6},
    {20016, 55, breaths + 6},
    {20016, 55, breaths + 6},
    {25009, 30, breaths + 6},
    {25504, 30, breaths + 4},
    {27016, 30, breaths + 6},
    {28022, 55, breaths + 6},
    {-1},
};

int BreathWeapon(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct breather *scan;
    int             count;

    if (cmd) {
        return FALSE;
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {

        for (scan = breath_monsters;
             scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].vnum;
             scan++) {
            /*
             * Empty loop
             */
        }

        if (scan->vnum < 0) {
            Log("monster %s tries to breath, but isn't listed.",
                ch->player.short_descr);
            return FALSE;
        }

        for (count = 0; scan->breaths[count]; count++) {
            /* 
             * Empty loop 
             */
        }

        if (count < 1) {
            Log("monster %s has no breath weapons", ch->player.short_descr);
            return FALSE;
        }

        use_breath_weapon(ch, ch->specials.fighting, scan->cost,
                          scan->breaths[dice(1, count) - 1]);
    }

    return (FALSE);
}

int cleric(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *vict;
    byte            lspell,
                    healperc = 0;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if (!MobCastCheck(ch, 0)) {
        return (TRUE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (!ch->specials.fighting) {
        if (GET_HIT(ch) < GET_MAX_HIT(ch) - 10) {
            if ((lspell = GetMaxLevel(ch)) >= 20) {
                act("$n utters the words 'What a Rush!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            } else if (lspell > 12) {
                act("$n utters the words 'Woah! I feel GOOD! Heh.'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            } else if (lspell > 8) {
                act("$n utters the words 'I feel much better now!'", 1, ch,
                    0, 0, TO_ROOM);
                cast_cure_serious(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
            } else {
                act("$n utters the words 'I feel good!'", 1, ch, 0, 0, TO_ROOM);
                cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
            }
        }

#ifdef PREP_SPELLS
        if (!ch->desc) {
            /* 
             * make sure it is a mob 
             *
             * low level prep 
             */
            if (!affected_by_spell(ch, SPELL_ARMOR)) {
                act("$n utters the words 'protect'", FALSE, ch, 0, 0, TO_ROOM);
                cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_BLESS)) {
                act("$n utters the words 'bless'", FALSE, ch, 0, 0, TO_ROOM);
                cast_bless(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_AID)) {
                act("$n utters the words 'aid'", FALSE, ch, 0, 0, TO_ROOM);
                cast_aid(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_DETECT_MAGIC)) {
                act("$n utters the words 'detect magic'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_detect_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL) && 
                !IS_EVIL(ch)) {
                act("$n utters the words 'anti evil'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (GetMaxLevel(ch) > 24) {
                if (!affected_by_spell(ch, SPELL_PROT_FIRE)) {
                    act("$n utters the words 'resist fire'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_prot_fire(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_COLD)) {
                    act("$n utters the words 'resist cold'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_prot_cold(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ENERGY)) {
                    act("$n utters the words 'resist energy'", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_prot_energy(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_PROT_ELEC)) {
                    act("$n utters the words 'resist electricity'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_prot_elec(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
                
                if (GetMaxLevel(ch) > 44 &&
                    !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                    !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                    !affected_by_spell(ch, SPELL_CHILLSHIELD) && 
                    !number(0, 2)) {
                    act("$n utters the words 'butcher's blade'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_blade_barrier(GetMaxLevel(ch), ch, "",
                                       SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }

            /*
             * low level removes 
             */
            if (affected_by_spell(ch, SPELL_POISON)) {
                act("$n utters the words 'remove poison'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_remove_poison(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (affected_by_spell(ch, SPELL_BLINDNESS)) {
                act("$n utters the words 'cure blind'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                ch, 0);
                return (TRUE);
            }

            /*
             * hi level removes 
             */
            if (GetMaxLevel(ch) > 24) {
                if (affected_by_spell(ch, SPELL_CURSE)) {
                    act("$n utters the words 'remove curse'", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_remove_curse(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (affected_by_spell(ch, SPELL_PARALYSIS)) {
                    act("$n utters the words 'remove paralysis'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_remove_paralysis(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }

            if (GET_MOVE(ch) < GET_MAX_MOVE(ch) / 2) {
                act("$n utters the words 'lemon aid'", FALSE, ch, 0, 0,
                    TO_ROOM);
                cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }
        }
#endif
    }

    /*
     * Find a dude to to evil things upon ! 
     */
    if ((vict = FindAHatee(ch)) == NULL) {
        vict = FindVictim(ch);
    }
    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    /*
     * gen number from 0 to level 
     */

    lspell = number(0, GetMaxLevel(ch));
    lspell += GetMaxLevel(ch) / 5;
    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 31 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n utters the words 'Woah! I'm outta here!'", 1, ch, 0, 0,
            TO_ROOM);
        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            return (TRUE);
        }
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    /*
     * first -- hit a foe, or help yourself? 
     */

    if (ch->points.hit < (ch->points.max_hit / 2)) {
        healperc = 7;
    } else if (ch->points.hit < (ch->points.max_hit / 4)) {
        healperc = 5;
    } else if (ch->points.hit < (ch->points.max_hit / 8)) {
        healperc = 3;
    }
    if (number(1, healperc + 2) > 3) {
        /* 
         * do harm 
         *
         * call lightning 
         */
        if (OUTSIDE(ch) && weather_info.sky >= SKY_RAINING && lspell >= 15 && 
            !number(0, 5)) {
            act("$n whistles.", 1, ch, 0, 0, TO_ROOM);
            act("$n utters the words 'Here Lightning!'", 1, ch, 0, 0, TO_ROOM);
            cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
        case 3:
            act("$n utters the words 'muhahaha!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 4:
        case 5:
        case 6:
            act("$n utters the words 'hocus pocus'", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 7:
            act("$n utters the words 'Va-Voom!'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 8:
            act("$n utters the words 'urgle blurg'", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 9:
        case 10:
            act("$n utters the words 'take that!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 11:
            act("$n utters the words 'burn baby burn'", 1, ch, 0, 0, TO_ROOM);
            cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            break;
        case 13:
        case 14:
        case 15:
        case 16:
            if (!IS_SET(vict->M_immune, IMM_FIRE)) {
                act("$n utters the words 'burn baby burn'", 1, ch, 0,
                    0, TO_ROOM);
                cast_flamestrike(GetMaxLevel(ch), ch, "",
                                 SPELL_TYPE_SPELL, vict, 0);
            } else if (IS_AFFECTED(vict, AFF_SANCTUARY)
                       && (GetMaxLevel(ch) > GetMaxLevel(vict))) {
                act("$n utters the words 'Va-Voom!'", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'take that!'", 1, ch, 0, 0, TO_ROOM);
                cast_cause_critic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 17:
        case 18:
        case 19:
        default:
            act("$n utters the words 'Hurts, doesn't it?'", 1, ch, 0, 0,
                TO_ROOM);
            cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        }

        return (TRUE);
    } else {
        /*
         * do heal 
         */

        if (IS_AFFECTED(ch, AFF_BLIND) && lspell >= 4 && !number(0, 3)) {
            act("$n utters the words 'Praise Celestian, I can SEE!'", 1,
                ch, 0, 0, TO_ROOM);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_CURSE) && lspell >= 6 && !number(0, 6)) {
            act("$n utters the words 'I'm rubber, you're glue'", 1, ch, 0,
                0, TO_ROOM);
            cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (IS_AFFECTED(ch, AFF_POISON) && lspell >= 5 && !number(0, 6)) {
            act("$n utters the words 'Praise Dog, I don't feel sick no more!'",
                1, ch, 0, 0, TO_ROOM);
            cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               ch, 0);
            return (TRUE);
        }

        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'abrazak'", 1, ch, 0, 0, TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'I feel good!'", 1, ch, 0, 0, TO_ROOM);
            cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            act("$n utters the words 'I feel much better now!'", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
            act("$n utters the words 'Woah! I feel GOOD! Heh.'", 1, ch, 0,
                0, TO_ROOM);
            cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        case 17:
        case 18:
            act("$n utters the words 'What a Rush!'", 1, ch, 0, 0, TO_ROOM);
            cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;
        default:
            act("$n utters the words 'Oooh, pretty!'", 1, ch, 0, 0, TO_ROOM);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            break;

        }

        return (TRUE);
    }
}

int Devil(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    return (magic_user(ch, cmd, arg, mob, type));
}

int Demon(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    return (magic_user(ch, cmd, arg, mob, type));
}

int DragonHunterLeader(struct char_data *ch, int cmd, char *arg,
                       struct char_data *mob, int type)
{
    register struct char_data *i,
                   *j;
    int             found = FALSE,
                    dir,
                    count;
    char            buf[255];

    if (type == PULSE_TICK) {
        if (ch->specials.position == POSITION_SITTING) {
            ch->generic = 0;
            switch (number(1, 10)) {
            case 1:
                command_interpreter(ch, "emote mumbles something about in his "
                                        "day the tavern being a popular "
                                        "hangout.");
                break;
            case 2:
                command_interpreter(ch, "say I really miss the good old days "
                                        "of fighting dragons all day.");
                command_interpreter(ch, "say I really should do it more often "
                                        "to keep in shape.");
                break;
            default:
                break;
            }
            return (TRUE);
        }

        if (ch->specials.position == POSITION_STANDING) {
            if (ch->generic <= 20) {
                ch->generic++;
                return (FALSE);
            } else if (ch->generic == 21) {
                for (i = character_list; i; i = i->next) {
                    if (IS_MOB(i) && mob_index[i->nr].vnum == WHO_TO_HUNT) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    ch->generic = 25;
                    command_interpreter(ch, "say Ack! The dragon is dead! I'm "
                                            "going back to the bar!");
                } else {
                    command_interpreter(ch, "say Ok, Follow me and let's go "
                                            "kill ourselves a dragon!");
                    ch->generic = 23;
                    count = 1;
                    for (i = real_roomp(ch->in_room)->people; i;
                         i = i->next_in_room) {
                        if (IS_MOB(i) && 
                            mob_index[i->nr].vnum == WHO_TO_CALL) {
                            (*mob_index[i->nr].func) (i, 0, "", ch,
                                                      EVENT_FOLLOW);
                            sprintf(buf, "group %d.%s", count, GET_NAME(i));
                            command_interpreter(ch, buf);
                            count++;
                        } else if (i->master && i->master == ch && 
                                   GetMaxLevel(i) > 10) {
                            sprintf(buf, "group %s", GET_NAME(i));
                            command_interpreter(ch, buf);
                        } else if (i->master && i->master == ch) {
                            sprintf(buf, "tell %s You're too little! Get Lost!",
                                    GET_NAME(i));
                            command_interpreter(ch, buf);
                        }
                    }
                }

                if (!IS_AFFECTED(ch, AFF_GROUP)) {
                    SET_BIT(ch->specials.affected_by, AFF_GROUP);
                }
                spell_fly_group(40, ch, 0, 0);
                return (FALSE);
            } else if (ch->generic == 23) {
                for (i = character_list; i; i = i->next) {
                    if (IS_MOB(i) && mob_index[i->nr].vnum == WHO_TO_HUNT) {
                        found = TRUE;
                        break;
                    }
                }

                if (!found) {
                    ch->generic = 25;
                    command_interpreter(ch, "say Ack! The dragon is dead! I'm "
                                            "going back to the bar!");
                } else {
                    dir = choose_exit_global(ch->in_room, i->in_room,
                                             MAX_ROOMS);
                    if (dir == -1) {
                        /* 
                         * can't go anywhere, wait... 
                         */
                        return (FALSE);
                    }
                    go_direction(ch, dir);

                    if (ch->in_room == i->in_room) {
                        /* 
                         * we're here! 
                         */
                        command_interpreter(ch, "shout The dragon must die!");

                        for (j = real_roomp(ch->in_room)->people; j;
                             j = j->next_in_room) {
                            if (IS_MOB(j) && 
                                mob_index[j->nr].vnum == WHO_TO_CALL) {
                                (*mob_index[j->nr].func) (j, 0, "", i,
                                                          EVENT_ATTACK);
                            }
                        }

                        ch->generic = 24;
                        hit(ch, i, TYPE_UNDEFINED);
                    }
                    return (FALSE);
                }
            } else if (ch->generic == 24) {
                command_interpreter(ch, "say Guess it's back to the bar for me!"
                                        " I need a drink!");
                ch->generic = 25;
            } else if (ch->generic == 25) {
                dir = choose_exit_global(ch->in_room, WHERE_TO_SIT, MAX_ROOMS);
                if (dir == -1) {
                    /* 
                     * no place to go, wait 
                     */
                    return (FALSE);
                }
                go_direction(ch, dir);
                if (ch->in_room == WHERE_TO_SIT) {
                    command_interpreter(ch, "say Ahhh, time for a drink!");
                    for (i = real_roomp(ch->in_room)->people; i;
                         i = i->next_in_room) {
                        if (IS_MOB(i) && 
                            mob_index[i->nr].vnum == WHO_TO_CALL) {
                            (*mob_index[i->nr].func) (i, 0, "", i,
                                                      EVENT_FOLLOW);
                        }
                    }

                    do_sit(ch, NULL, 0);
                    command_interpreter(ch, "say Bartender, how about a "
                                            "drink?");
                    ch->generic = 0;
                }
            }
        }
    }

    if (type == EVENT_WEEK) {
        /* 
         * months are TOO long 
         */
        if (ch->specials.position != POSITION_SITTING) {
            /* 
             * We're doing something else, ignore 
             */
            return (FALSE);
        }

        for (i = character_list; i; i = i->next) {
            if (IS_MOB(i) && mob_index[i->nr].vnum == WHO_TO_HUNT) {
                found = TRUE;
                break;
            }
        }

        if (!found) {
            /* 
             * No Dragon in the game, ignore 
             */
            return (FALSE);
        }

        for (i = character_list; i; i = i->next) {
            if (IS_MOB(i) && (mob_index[i->nr].vnum == WHO_TO_CALL)) {
                (*mob_index[i->nr].func) (i, 0, "", ch, EVENT_GATHER);
            }
        }

        command_interpreter(ch, "shout All who want to hunt a dragon, come to "
                                "me!");
        do_stand(ch, NULL, 0);
    }

    return (FALSE);
}

int HuntingMercenary(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    int             dir;

    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == PULSE_TICK) {
        if (ch->generic == 1) {
            /* 
             * Going to room 
             */
            if (!IS_SET(ch->specials.act, ACT_SENTINEL)) {
                SET_BIT(ch->specials.act, ACT_SENTINEL);
            }
            dir = choose_exit_global(ch->in_room, WHERE_TO_SIT, MAX_ROOMS);
            if (dir == -1) {
                return (FALSE);
            }
            go_direction(ch, dir);

            if (ch->in_room == WHERE_TO_SIT) {
                ch->generic = 0;
            }
        }
        return (FALSE);
    }

    if (type == EVENT_GATHER) {
        ch->generic = 1;
        return (FALSE);
    }

    if (type == EVENT_ATTACK) {
        hit(ch, mob, TYPE_UNDEFINED);
        return (FALSE);
    }

    if (type == EVENT_FOLLOW) {
        if (ch == mob) {
            if (IS_SET(ch->specials.act, ACT_SENTINEL)) {
                REMOVE_BIT(ch->specials.act, ACT_SENTINEL);
            }
            ch->generic = 0;
            stop_follower(ch);
        } else {
            add_follower(ch, mob);
        }
    }

    return (FALSE);
}

int druid_protector(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    static int      b = 1;      
    /* 
     * use this as a switch, to avoid double
     * challenges 
     */

    if (cmd) {
        if (cmd <= 6 && cmd >= 1 && IS_PC(ch)) {
            if (b) {
                b = 0;
                send_to_char("Basil Great Druid looks at you\n\r", ch);
                if (ch->in_room == Bandits_Path && cmd == 1 &&
                    BASIL_GATEKEEPER_MAX_LEVEL < GetMaxLevel(ch) &&
                    !IS_IMMORTAL(ch)) {
                    if (!check_soundproof(ch)) {
                        act("Basil the Great Druid tells you 'Begone "
                            "Unbelievers!'", TRUE, ch, 0, 0, TO_CHAR);
                    }
                    act("Basil Great Druid grins evilly.", TRUE, ch, 0,
                        0, TO_CHAR);
                    return (TRUE);
                }
            } else {
                b = 1;
            }
            return (FALSE);
        }                       
        /* 
         * cmd 1 - 6 
         */
        return (FALSE);
    } 
    
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        } else {
            FighterMove(ch);
        }
    }
    return (FALSE);
}


extern int      gevent;
extern float    shop_multiplier;

int DwarvenMiners(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    if (type == PULSE_COMMAND) {
        return (FALSE);
    }
    if (type == EVENT_END_STRIKE && ch->specials.position == POSITION_SITTING) {
        command_interpreter(ch, "emote is off strike.");
        do_stand(ch, NULL, 0);
        ch->specials.default_pos = POSITION_STANDING;
        ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                 sizeof(char) * 54);
        strcpy(ch->player.long_descr,
               "A dwarven mine-worker is here, working the mines.\n\r");
        gevent &= ~DWARVES_STRIKE;
        shop_multiplier = 0;
    }

    if (type == EVENT_DWARVES_STRIKE) {
        if (ch->specials.position == POSITION_STANDING) {
            command_interpreter(ch, "emote is on strike.");
            do_sit(ch, NULL, 0);
            ch->specials.default_pos = POSITION_SITTING;
            ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                     sizeof(char) * 55);
            strcpy(ch->player.long_descr,
                   "A dwarven mine-worker is sitting here on-strike\n\r");
        }
        ch->generic = 30;
        return (FALSE);
    }

    if (type == PULSE_TICK) {
        if (!(gevent & DWARVES_STRIKE)) {
            ch->generic = 0;
            return (FALSE);
        }

        ch->generic++;
        if (ch->generic == 30) {
            /* 
             * strike over, back to work 
             */
            gevent &= ~DWARVES_STRIKE;
            PulseMobiles(EVENT_END_STRIKE);
            if (ch->specials.position == POSITION_SITTING) {
                command_interpreter(ch, "emote is off strike.");
                do_stand(ch, NULL, 0);
                ch->specials.default_pos = POSITION_STANDING;
                ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                         sizeof(char) * 65);
                strcpy(ch->player.long_descr,
                       "A dwarven mine-worker is here, working the mines.\n\r");
            }
            ch->generic = 0;
            shop_multiplier = 0;
        }

        return (FALSE);
    }

    if ((type == EVENT_BIRTH) && (!(gevent & DWARVES_STRIKE))) {
        return (FALSE);
    }
    if (type == EVENT_BIRTH) {
        if (ch->specials.position == POSITION_STANDING) {
            command_interpreter(ch, "emote is on strike.");
            do_sit(ch, NULL, 0);
            ch->specials.default_pos = POSITION_SITTING;
            ch->player.long_descr = (char *) realloc(ch->player.long_descr,
                                                     sizeof(char) * 55);
            strcpy(ch->player.long_descr,
                   "A dwarven mine-worker is sitting here on-strike\n\r");
        }
        ch->generic = 30;
        return (FALSE);
    }

    if (type == EVENT_WEEK) {
        if (gevent != 0) {
            /* 
             * something else happening? FORGET IT! 
             */
            return (FALSE);
        }

        if (!number(1, 6)) {
            /* 
             * 1 in 6 chance of striking this week 
             */
            return (FALSE);
        }

        gevent |= DWARVES_STRIKE;
        PulseMobiles(EVENT_DWARVES_STRIKE);

        switch (number(1, 5)) {
        case 1:
            shop_multiplier = 0.25;
            break;
        case 2:
            shop_multiplier = 0.5;
            break;
        case 3:
            shop_multiplier = 0.75;
            break;
        case 4:
            shop_multiplier = 1.0;
            break;
        case 5:
            shop_multiplier = 1.5;
            break;
        }
    }
    return (FALSE);
}


const int       post_list[] = {
    32001,
    32004,
    32009,
    32011,
    32030,
    32024,
    32032,
    33180,                      /* split */
    31804,
    32600,
    32300,
    32801,
    32803,
    32802
};
int post_count = NELEMENTS(post_list);

const int       post_destination[] = {
    31804,
    33180,
    32300,
    32600,
    32838,
    3014,
    25002,
    32004,
    32000,
    32011,
    32009,
    32030,
    32024,
    32032
};

int Etheral_post(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    int             check = -1,
                    x = 0;
    char           *arg1;
    int             j;
    struct char_data *post;

    if (cmd != 463) {
        /* 
         * touch 
         */
        return (FALSE);
    }

    arg = get_argument(arg, &arg1);
    if (arg1) {
        if (!strcasecmp("post", arg1) || !strcasecmp("ethereal", arg1) ||
            !strcasecmp("ethereal post", arg1)) {
            if ((post = get_char_room("ethereal post", ch->in_room))) {
                /*
                 * Check to see where the post is going 
                 */
                check = -1;

                for (x = 0; x < post_count; x++) {
                    if (mob_index[post->nr].vnum == post_list[x]) {
                        check = x;
                    }
                }

                if (check == -1) {
                    return (FALSE);
                }
                if (GetMaxLevel(ch) < 41) {
                    /* 
                     * level fix 
                     */
                    send_to_char("You touch the post, and a brief sensation "
                                 "of unworthiness runs through you.\n\r", ch);
                    return (TRUE);
                }

                j = post_destination[check];

                send_to_char("You touch the strange post and suddenly feel "
                             "your mind", ch);
                send_to_char(" and body being torn appart.\n\r", ch);

                act("$n touches the strange post and suddenly disappears!",
                    FALSE, ch, 0, 0, TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, j);

                act("A strange rift like portal appears and $n steps out!",
                    FALSE, ch, 0, 0, TO_ROOM);
                do_look(ch, NULL, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

int ghost(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting && 
        ch->specials.fighting->in_room == ch->in_room) {
        act("$n touches $N!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n touches you!", 1, ch, 0, ch->specials.fighting, TO_VICT);
        cast_energy_drain(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                          ch->specials.fighting, 0);

        return TRUE;
    }
    return FALSE;
}

int magic_user(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *vict;

    byte            lspell;
    char            buf[200];

    if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS)) {
        return (FALSE);
    }
    if (!MobCastCheck(ch, 0)) {
        return (TRUE);
    }

    if (!ch->specials.fighting && !IS_PC(ch)) {
        if (GET_POS(ch) > POSITION_STUNNED && GET_POS(ch) < POSITION_FIGHTING) {
            StandUp(ch);
            return (TRUE);
        }
        SET_BIT(ch->player.class, CLASS_MAGIC_USER);

        if (GetMaxLevel(ch) >= 25 && !ch->desc) {
            if (Summoner(ch, cmd, arg, mob, type)) {
                return (TRUE);
            } else if (!IS_SET(real_roomp(ch->in_room)->room_flags, NO_SUM) && 
                NumCharmedFollowersInRoom(ch) < 4 && 
                IS_SET(ch->hatefield, HATE_CHAR)) {
                act("$n utters the words 'Here boy!'", 1, ch, 0, 0, TO_ROOM);
                cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                command_interpreter(ch, "order followers guard on");
                return (TRUE);
            }
        }

#ifdef PREP_SPELLS

        if (!ch->desc) {
            /* 
             * make sure it is a mob not a pc
             *
             * low level prep spells here 
             */
            if (!affected_by_spell(ch, SPELL_SHIELD)) {
                act("$n utters the words 'dragon'", 1, ch, 0, 0, TO_ROOM);
                cast_shield(GetMaxLevel(ch), ch, GET_NAME(ch),
                            SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_STRENGTH)) {
                act("$n utters the words 'giant'", 1, ch, 0, 0, TO_ROOM);
                cast_strength(GetMaxLevel(ch), ch, GET_NAME(ch),
                              SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!IS_EVIL(ch) && 
                !affected_by_spell(ch, SPELL_PROTECT_FROM_EVIL)) {
                act("$n utters the words 'anti-evil'", 1, ch, 0, 0, TO_ROOM);
                cast_protection_from_evil(GetMaxLevel(ch), ch, "",
                                          SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if ((IS_AFFECTED(ch, AFF_FIRESHIELD) || 
                 IS_AFFECTED(ch, AFF_SANCTUARY)) && 
                !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) && 
                !IS_AFFECTED(ch, AFF_DARKNESS)) {
                act("$n utters the words 'darkness'", 1, ch, 0, 0, TO_ROOM);
                cast_globe_darkness(GetMaxLevel(ch), ch, GET_NAME(ch),
                                    SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            if (!affected_by_spell(ch, SPELL_ARMOR)) {
                act("$n utters the words 'dragon'", 1, ch, 0, 0, TO_ROOM);
                cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (TRUE);
            }

            /*
             * high level prep spells here 
             */
            if (GetMaxLevel(ch) >= 25) {
                if (!affected_by_spell(ch, SPELL_STONE_SKIN)) {
                    act("$n utters the words 'stone'", 1, ch, 0, 0, TO_ROOM);
                    cast_stone_skin(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_GLOBE_MINOR_INV)) {
                    act("$n utters the words 'haven'", 1, ch, 0, 0, TO_ROOM);
                    cast_globe_minor_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (!affected_by_spell(ch, SPELL_GLOBE_MAJOR_INV)) {
                    act("$n utters the words 'super haven'", 1, ch, 0, 0,
                        TO_ROOM);
                    cast_globe_major_inv(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }

                if (GetMaxLevel(ch) >= 40 &&
                    !affected_by_spell(ch, SPELL_BLADE_BARRIER) &&
                    !affected_by_spell(ch, SPELL_FIRESHIELD) &&
                    !affected_by_spell(ch, SPELL_CHILLSHIELD) && 
                    !number(0, 2)) {
                    act("$n utters the words 'crackle'", 1, ch, 0, 0, TO_ROOM);
                    cast_fireshield(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
                    return (TRUE);
                }
            }
            return (FALSE);
        }
#endif
    }

    if (!ch->specials.fighting) {
        return (FALSE);
    }
    if (!IS_PC(ch) && GET_POS(ch) > POSITION_STUNNED && 
        GET_POS(ch) < POSITION_FIGHTING) {
        if (GET_HIT(ch) > GET_HIT(ch->specials.fighting) / 2) {
            StandUp(ch);
        } else {
            StandUp(ch);
            do_flee(ch, NULL, 0);
        }
        return (TRUE);
    }

    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    /*
     * Find a dude to to evil things upon ! 
     */
    vict = FindVictim(ch);

    if (!vict) {
        vict = ch->specials.fighting;
    }
    if (!vict) {
        return (FALSE);
    }
    lspell = number(0, GetMaxLevel(ch));
    
    if (!IS_PC(ch)) {
        lspell += GetMaxLevel(ch) / 5;
    }

    lspell = MIN(GetMaxLevel(ch), lspell);

    if (lspell < 1) {
        lspell = 1;
    }
    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
        act("$n utters the words 'Let me see the light!'", TRUE, ch, 0, 0,
            TO_ROOM);
        cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return TRUE;
    }

    if (IS_AFFECTED(ch, AFF_BLIND)) {
        return (FALSE);
    }
    if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 10 && 
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        act("$n utters the words 'Use MagicAway Instant Magic Remover'", 1,
            ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
    }

    if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 10 && 
        GetMaxLevel(ch) > GetMaxLevel(vict)) {
        act("$n utters the words 'Use MagicAway Instant Magic Remover'", 1,
            ch, 0, 0, TO_ROOM);
        cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 28 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n checks $s watch.", TRUE, ch, 0, 0, TO_ROOM);
        act("$n utters the words 'Oh my, would you just LOOK at the time!'", 
            1, ch, 0, 0, TO_ROOM);
        vict = FindMobDiffZoneSameRace(ch);
        if (vict) {
            spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
            return (TRUE);
        }
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    if (!IS_PC(ch) && GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && lspell > 15 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {
        act("$n utters the words 'Woah! I'm outta here!'", 1, ch, 0, 0,
            TO_ROOM);
        cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
        return (FALSE);
    }

    if (GET_HIT(ch) > GET_MAX_HIT(ch) / 2 && 
        !IS_SET(ch->specials.act, ACT_AGGRESSIVE) && 
        GetMaxLevel(vict) < GetMaxLevel(ch) && number(0, 1)) {
        if (lspell > 8 && lspell < 50 && !number(0, 6)) {
            act("$n utters the words 'Icky Sticky!'", 1, ch, 0, 0, TO_ROOM);
            cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 6)) {
            act("$n utters the words 'You wimp'", 1, ch, 0, 0, TO_ROOM);
            cast_weakness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 5 && lspell < 10 && !number(0, 7)) {
            act("$n utters the words 'Bippety boppity Boom'", 1, ch, 0, 0,
                TO_ROOM);
            cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return TRUE;
        }

        if (lspell > 12 && lspell < 20 && !number(0, 7)) {
            act("$n utters the words '&#%^^@%*#'", 1, ch, 0, 0, TO_ROOM);
            cast_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 10 && lspell < 20 && !number(0, 5)) {
            act("$n utters the words 'yabba dabba do'", 1, ch, 0, 0, TO_ROOM);
            cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return TRUE;
        }

        if (lspell > 8 && lspell < 40 && !number(0, 5) && 
            vict->specials.fighting != ch) {
            act("$n utters the words 'You are getting sleepy'", 1, ch, 0,
                0, TO_ROOM);
            cast_charm_monster(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);

            if (IS_AFFECTED(vict, AFF_CHARM)) {
                if (!vict->specials.fighting) {
                    sprintf(buf, "order %s kill %s", GET_NAME(vict),
                            GET_NAME(ch->specials.fighting));
                } else {
                    sprintf(buf, "order %s remove all", GET_NAME(vict));
                }
                command_interpreter(ch, buf);
            }
        }
    } else {
        switch (lspell) {
        case 1:
        case 2:
            act("$n utters the words 'bang! bang! pow!'", 1, ch, 0, 0, TO_ROOM);
            cast_magic_missile(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
            break;
        case 3:
        case 4:
        case 5:
            act("$n utters the words 'ZZZZzzzzzzTTTT'", 1, ch, 0, 0, TO_ROOM);
            cast_shocking_grasp(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                vict, 0);
            break;
        case 6:
        case 7:
        case 8:
            if (ch->attackers <= 2) {
                act("$n utters the words 'Icky Sticky!'", 1, ch, 0, 0, TO_ROOM);
                cast_web(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Fwoosh!'", 1, ch, 0, 0, TO_ROOM);
                cast_burning_hands(GetMaxLevel(ch), ch, "",
                                   SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 9:
        case 10:
            act("$n utters the words 'SPOOGE!'", 1, ch, 0, 0, TO_ROOM);
            cast_acid_blast(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 11:
        case 12:
        case 13:
            if (ch->attackers <= 2) {
                act("$n utters the words 'KAZAP!'", 1, ch, 0, 0, TO_ROOM);
                cast_lightning_bolt(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Ice Ice Baby!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_ice_storm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
            }
            break;
        case 14:
        case 15:
            act("$n utters the words 'Ciao!'", 1, ch, 0, 0, TO_ROOM);
            cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 16:
        case 17:
        case 18:
        case 19:
            act("$n utters the words 'maple syrup'", 1, ch, 0, 0, TO_ROOM);
            cast_slow(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 20:
        case 21:
        case 22:
            if (IS_EVIL(ch)) {
                act("$n utters the words 'slllrrrrrrpppp'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_energy_drain(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
            if (ch->attackers <= 2) {
                act("$n utters the words 'Look! A rainbow!'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_colour_spray(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Get the sensation!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_cone_of_cold(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
            act("$n utters the words 'Hasta la vista, Baby'", 1, ch, 0, 0,
                TO_ROOM);
            cast_fireball(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 36:
        case 37:
            act("$n utters the words 'KAZAP KAZAP KAZAP!'", 1, ch, 0, 0,
                TO_ROOM);
            cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            break;
        case 38:
            act("$n utters the words 'duhhh'", 1, ch, 0, 0, TO_ROOM);
            cast_feeblemind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 39:
            act("$n utters the words 'STOP'", 1, ch, 0, 0, TO_ROOM);
            cast_paralyze(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            break;
        case 40:
        case 41:
            if (ch->attackers <= 2) {
                act("$n utters the words 'frag'", 1, ch, 0, 0, TO_ROOM);
                cast_meteor_swarm(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Whew, whata smell!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_incendiary_cloud(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        default:
            if (ch->attackers <= 2) {
                act("$n utters the words 'ZZAAPP!'", 1, ch, 0, 0, TO_ROOM);
                cast_disintegrate(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
            } else {
                act("$n utters the words 'Whew, whata smell!'", 1, ch, 0,
                    0, TO_ROOM);
                cast_incendiary_cloud(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, vict, 0);
            }
            break;
        }
    }
    return TRUE;
}

int monk(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        MonkMove(ch);
    }
    return (FALSE);
}

int PrydainGuard(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    return (GenericCityguardHateUndead(ch, cmd, arg, mob, PRYDAIN));
}

int real_rabbit(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *i;

    if (cmd || !AWAKE(ch) || ch->specials.fighting) {
        return FALSE;
    }
    for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (IS_NPC(i) && mob_index[i->nr].vnum == 6005 && !number(0, 3)) {
            command_interpreter(ch, "emote sees the damn fox and runs like "
                                    "hell.");
            do_flee(ch, NULL, 0);
            return TRUE;
        }
    }

    if (!number(0, 5)) {
        switch (number(1, 2)) {
        case 1:
            command_interpreter(ch, "emote nibbles on some grass.");
            break;
        case 2:
            command_interpreter(ch, "emote bounces lightly to another patch of"
                                    " grass.");
            break;
        }
        return TRUE;
    }
    return FALSE;
}

int real_fox(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *i;
    struct obj_data *j,
                   *k,
                   *next;
    Keywords_t     *key;

    if (cmd || !AWAKE(ch) || ch->specials.fighting) {
        return FALSE;
    }
    if (ch->generic) {
        ch->generic--;
        return TRUE;
    }

    key = StringToKeywords( "corpse rabbit", NULL );

    for (j = real_roomp(ch->in_room)->contents; j; j = j->next_content) {
        if (IS_CORPSE(i) && KeywordsMatch(key, &j->keywords)) {
            command_interpreter(ch, "emote gorges on the corpse of a rabbit.");
            for (k = j->contains; k; k = next) {
                next = k->next_content;
                objectTakeFromObject(k);
                objectPutInRoom(k, ch->in_room);
            }
            objectExtract(j);
            ch->generic = 10;
            FreeKeywords(key, TRUE);
            return (TRUE);
        }
    }

    FreeKeywords(key, TRUE);

    for (i = real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
        if (IS_NPC(i) && mob_index[i->nr].vnum == 6001 && !number(0, 3)) {
            command_interpreter(ch, "emote yips and starts to make dinner.");
            hit(ch, i, TYPE_UNDEFINED);
            return TRUE;
        }
    }

    return FALSE;
}


int RepairGuy(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];
    int             cost,
                    ave;
    struct char_data *vict;
    struct obj_data *obj;
    struct obj_data *new;

    /* 
     * special procedure for this mob/obj 
     */
    int             (*rep_guy) ();

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    rep_guy = RepairGuy;

    if (IS_NPC(ch)) {
        if (cmd == 72) {
            arg = get_argument(arg, &obj_name);
            if (!obj_name || !(obj = get_obj_in_list_vis(ch, obj_name,
                                                         ch->carrying))) {
                return (FALSE);
            }

            arg = get_argument(arg, &vict_name);
            if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) ||
                !IS_NPC(vict)) {
                return (FALSE);
            }

            if (mob_index[vict->nr].func == rep_guy) {
                send_to_char("Nah, you really wouldn't want to do that.", ch);
                return (TRUE);
            }
        } else {
            return (FALSE);
        }
    }

    if (cmd == 72) {
        /* 
         * give 
         *
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name || !(obj = get_obj_in_list_vis(ch, obj_name,
                                                     ch->carrying))){
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * the target is the repairman, or an NPC 
         */
        if (!IS_NPC(vict)) {
            return (FALSE);
        }
        if (mob_index[vict->nr].func == rep_guy) {
            /*
             * we have the repair guy, and we can give him the stuff 
             */
            act("You give $p to $N.", TRUE, ch, obj, vict, TO_CHAR);
            act("$n gives $p to $N.", TRUE, ch, obj, vict, TO_ROOM);
        } else {
            return (FALSE);
        }

        act("$N looks at $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N looks at $p.", TRUE, ch, obj, vict, TO_ROOM);

        /*
         * make all the correct tests to make sure that everything is
         * kosher 
         */

        if (ITEM_TYPE(obj) == ITEM_TYPE_ARMOR && obj->value[1] > 0) {
            if (obj->value[1] > obj->value[0]) {
                cost = obj->cost;
                cost /= obj->value[1];
                cost *= (obj->value[1] - obj->value[0]);

                if (GetMaxLevel(vict) > 25) {
                    /* 
                     * super repair guy 
                     */
                    cost *= 2;
                }

                if (cost > GET_GOLD(ch)) {
                    if (check_soundproof(ch)) {
                        act("$N shakes $S head.\n\r",
                            TRUE, ch, 0, vict, TO_ROOM);
                        act("$N shakes $S head.\n\r",
                            TRUE, ch, 0, vict, TO_CHAR);
                    } else {
                        act("$N says 'I'm sorry, you don't have enough money.'",
                            TRUE, ch, 0, vict, TO_ROOM);
                        act("$N says 'I'm sorry, you don't have enough money.'",
                            TRUE, ch, 0, vict, TO_CHAR);
                    }
                } else {
                    GET_GOLD(ch) -= cost;

                    sprintf(buf, "You give $N %d coins.", cost);
                    act(buf, TRUE, ch, 0, vict, TO_CHAR);
                    act("$n gives some money to $N.", TRUE, ch, obj, vict,
                        TO_ROOM);

                    /*
                     * fix the armor 
                     */
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
                    act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);
                    if (GetMaxLevel(vict) > 25) {
                        obj->value[0] = obj->value[1];
                    } else {
                        ave = MAX(obj->value[0],
                                  (obj->value[0] + obj->value[1]) / 2);
                        obj->value[0] = ave;
                        obj->value[1] = ave;
                    }

                    if (check_soundproof(ch)) {
                        act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
                        act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
                    } else {
                        act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
                        act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
                    }
                }
            } else if (check_soundproof(ch)) {
                act("$N shrugs.", TRUE, ch, 0, vict, TO_ROOM);
                act("$N shrugs.", TRUE, ch, 0, vict, TO_CHAR);
            } else {
                act("$N says 'Your armor looks fine to me.'",
                    TRUE, ch, 0, vict, TO_ROOM);
                act("$N says 'Your armor looks fine to me.'",
                    TRUE, ch, 0, vict, TO_CHAR);
            }
        } else if (GetMaxLevel(vict) < 25 || 
                   ITEM_TYPE(obj) != ITEM_TYPE_WEAPON) {
            if (check_soundproof(ch)) {
                act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
                act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
            } else if (ITEM_TYPE(obj) != ITEM_TYPE_ARMOR) {
                act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_ROOM);
                act("$N says 'That isn't armor.'", TRUE, ch, 0, vict, TO_CHAR);
            } else {
                act("$N says 'I can't fix that...'", TRUE, ch, 0,
                    vict, TO_CHAR);
                act("$N says 'I can't fix that...'", TRUE, ch, 0,
                    vict, TO_ROOM);
            }
        } else {
            /*
             * weapon repair.  expensive! 
             */
            cost = obj->cost;
            new = objectRead(obj->item_number, REAL);
            if (obj->value[2]) {
                cost /= obj->value[2];
            }

            cost *= (new->value[2] - obj->value[2]);

            if (cost > GET_GOLD(ch)) {
                if (check_soundproof(ch)) {
                    act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N shakes $S head.\n\r", TRUE, ch, 0, vict, TO_CHAR);
                } else {
                    act("$N says 'I'm sorry, you don't have enough money.'",
                        TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'I'm sorry, you don't have enough money.'",
                        TRUE, ch, 0, vict, TO_CHAR);
                    objectExtract(new);
                }
            } else {
                GET_GOLD(ch) -= cost;

                sprintf(buf, "You give $N %d coins.", cost);
                act(buf, TRUE, ch, 0, vict, TO_CHAR);
                act("$n gives some money to $N.", TRUE, ch, obj, vict, TO_ROOM);

                /*
                 * fix the weapon 
                 */
                act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_ROOM);
                act("$N fiddles with $p.", TRUE, ch, obj, vict, TO_CHAR);

                obj->value[2] = new->value[2];
                objectExtract(new);

                if (check_soundproof(ch)) {
                    act("$N smiles broadly.", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N smiles broadly.", TRUE, ch, 0, vict, TO_CHAR);
                } else {
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_ROOM);
                    act("$N says 'All fixed.'", TRUE, ch, 0, vict, TO_CHAR);
                }
            }
        }

        act("$N gives you $p.", TRUE, ch, obj, vict, TO_CHAR);
        act("$N gives $p to $n.", TRUE, ch, obj, vict, TO_ROOM);
        return (TRUE);
    }

    if (cmd) {
        return FALSE;
    }
    return (fighter(ch, cmd, arg, mob, type));
}

int DruidChallenger(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    struct room_data *rp;
    int             level;
    struct char_data *vict;
    int             i;
    char           *name;

    if ((cmd) || (!AWAKE(ch))) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);
    if ((!rp) || (check_soundproof(ch))) {
        return (FALSE);
    }
    GreetPeople(ch);

    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_SLEEPING) {
        StandUp(ch);
        return (TRUE);
    }

    if (number(0, 101) > GetMaxLevel(ch) + 40) {
        return (TRUE);
    }
    if (!ch->specials.fighting) {
        level = number(1, GetMaxLevel(ch));
        if (GET_HIT(ch) < GET_MAX_HIT(ch)) {
            DruidHeal(ch, level);
            return (TRUE);
        }

        if (!ch->equipment[WIELD]) {
            if (GetMaxLevel(ch) > 4) {
                act("$n utters the words 'gimme a light'", 1, ch, 0, 0,
                    TO_ROOM);
                cast_flame_blade(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                                 ch, 0);
            }
            return (TRUE);
        }

        if (!affected_by_spell(ch, SPELL_BARKSKIN) && number(0, 1)) {
            act("$n utters the words 'woof woof'", 1, ch, 0, 0, TO_ROOM);
            cast_barkskin(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (TRUE);
        }

        if (ch->in_room == 501 && (vict = FindAHatee(ch)) == NULL) {
            vict = FindVictim(ch);
            if (vict) {
                name = strdup(GET_NAME(vict));
                do_hit(ch, name, 0);
                if( name ) {
                    free( name );
                }
            }
            return (FALSE);
        }
    } else {
        if ((vict = FindAHatee(ch)) == NULL) {
            vict = FindVictim(ch);
        }
        if (!vict) {
            vict = ch->specials.fighting;
        }
        if (!vict) {
            return (FALSE);
        }
        level = number(1, GetMaxLevel(ch));

        if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 2) && number(0, 1)) {
            DruidHeal(ch, level);
            return (TRUE);
        }

        if (GetMaxLevel(ch) >= 20 && !IS_AFFECTED(vict, AFF_SILENCE) &&
            !number(0, 4)) {
            act("$n utters the words 'ssshhhh'", 1, ch, 0, 0, TO_ROOM);
            cast_silence(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (FALSE);
        }

        if ((IS_AFFECTED(vict, AFF_FIRESHIELD) || 
            IS_AFFECTED(vict, AFF_SANCTUARY)) && 
            GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'use instaway instant magic remover'", 1,
                ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "",
                              SPELL_TYPE_SPELL, vict, 0);
            return (TRUE);
        }

        if (IsUndead(vict) && !number(0, 2)) {
            act("$n utters the words 'see the light!'", 1, ch, 0, 0, TO_ROOM);
            cast_sunray(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (FALSE);
        }

        if (vict->equipment[WIELD] && level > 19) {
            act("$n utters the words 'frogemoth'", 1, ch, 0, 0, TO_ROOM);
            cast_warp_weapon(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                             vict, 0);
            return (FALSE);
        }

        if (level > 23) {
            for (i = 0; i < MAX_WEAR; i++) {
                if (vict->equipment[i]) {
                    act("$n utters the words 'barbecue?'", 1, ch, 0, 0,
                        TO_ROOM);
                    cast_warp_weapon(GetMaxLevel(ch), ch, "",
                                     SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }
            }
        }

        if (IS_SET(rp->room_flags, NO_SUM) || IS_SET(rp->room_flags, TUNNEL) ||
            IS_SET(rp->room_flags, PRIVATE)) {
            DruidAttackSpells(ch, vict, level);
        } else {
            if (rp->sector_type == SECT_FOREST) {
                if (level > 16 && !number(0, 5)) {
                    act("$n utters the words 'briar'", 1, ch, 0, 0, TO_ROOM);
                    cast_entangle(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }

                if (level >= 8 && !number(0, 3)) {
                    act("$n utters the words 'snap!'", 1, ch, 0, 0, TO_ROOM);
                    cast_snare(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict,
                               0);
                    return (FALSE);
                }

                if (level > 30 && ch->mult_att < 6 && !number(0, 8)) {
                    DruidTree(ch);
                    return (FALSE);
                }

                if (ch->mult_att < 2 && level > 10 && !number(0, 8)) {
                    DruidMob(ch);
                    return (FALSE);
                }

                if (level > 30 && !number(0, 4)) {
                    act("$n utters the words 'Where is my SERVANT!'",
                        FALSE, ch, 0, 0, TO_ROOM);
                    cast_fire_servant(GetMaxLevel(ch), ch, "",
                                      SPELL_TYPE_SPELL, 0, 0);
                    command_interpreter(ch, "order followers guard on");
                    return (FALSE);
                } 
                
                if (level > 10 && !number(0, 5)) {
                    act("$n whistles", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_1(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 16 && !number(0, 5)) {
                    act("$n whistles loudly", FALSE, ch, 0, 0, TO_ROOM);
                    cast_animal_summon_2(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 24 && !number(0, 5)) {
                    act("$n whistles extremely loudly", FALSE, ch, 0,
                        0, TO_ROOM);
                    cast_animal_summon_3(GetMaxLevel(ch), ch, "",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }
            } else if (!IS_SET(rp->room_flags, INDOORS)) {
                if (level > 8 && !number(0, 3)) {
                    act("$n utters the words 'let it rain'", FALSE, ch,
                        0, 0, TO_ROOM);
                    cast_control_weather(GetMaxLevel(ch), ch, "worse",
                                         SPELL_TYPE_SPELL, 0, 0);
                    return (FALSE);
                }

                if (level > 15 && !number(0, 2)) {
                    act("$n utters the words 'here lightning'", FALSE,
                        ch, 0, 0, TO_ROOM);
                    cast_call_lightning(GetMaxLevel(ch), ch, "",
                                        SPELL_TYPE_SPELL, vict, 0);
                    return (FALSE);
                }
            }
            DruidAttackSpells(ch, vict, level);
        }
    }
    return(FALSE);
}

int MonkChallenger(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict;
    char                *name;

    if ((cmd) || (!AWAKE(ch))) {
        return (FALSE);
    }
    GreetPeople(ch);

    if (!ch->specials.fighting && ch->in_room == 551 &&
        (vict = FindAHatee(ch)) == NULL) {
        vict = FindVictim(ch);
        if (vict) {
            name = strdup(GET_NAME(vict));
            do_hit(ch, name, 0);
            if( name ) {
                free( name );
            }
        }
        return (FALSE);
    }
    MonkMove(ch);
    return (1);
}


void GreetPeople(struct char_data *ch)
{
    struct char_data *tch;

    if (!IS_SET(ch->specials.act, ACT_GREET)) {
        for (tch = real_roomp(ch->in_room)->people; tch;
             tch = tch->next_in_room) {
            if (!IS_NPC(tch) && !number(0, 8) && 
                GetMaxLevel(tch) > GetMaxLevel(ch) && CAN_SEE(ch, tch)) {
                Submit(ch, tch);
                SayHello(ch, tch);
                SET_BIT(ch->specials.act, ACT_GREET);
                break;
            }
        }
    } else if (!number(0, 100)) {
        REMOVE_BIT(ch->specials.act, ACT_GREET);
    }
}

void SayHello(struct char_data *ch, struct char_data *t)
{
    char            buf[200];
    char            buf2[80];

    switch (number(1, 10)) {
    case 1:
        command_interpreter(ch, "say Greetings, adventurer");
        break;
    case 2:
        if (t->player.sex == SEX_FEMALE) {
            command_interpreter(ch, "say Good day, milady");
        } else {
            command_interpreter(ch, "say Good day, lord");
        }
        break;
    case 3:
        if (t->player.sex == SEX_FEMALE) {
            command_interpreter(ch, "say Pleasant Journey, Mistress");
        } else {
            command_interpreter(ch, "say Pleasant Journey, Master");
        }
        break;
    case 4:
        if (t->player.sex == SEX_FEMALE) {
            sprintf(buf, "say Make way!  Make way for the lady %s!",
                    GET_NAME(t));
        } else {
            sprintf(buf, "say Make way!  Make way for the lord %s!",
                    GET_NAME(t));
        }
        command_interpreter(ch, buf);
        break;
    case 5:
        command_interpreter(ch, "say May the prophet smile upon you");
        break;
    case 6:
        command_interpreter(ch, "say It is a pleasure to see you again.");
        break;
    case 7:
        command_interpreter(ch, "say You are always welcome here, great one");
        break;
    case 8:
        command_interpreter(ch, "say My lord bids you greetings");
        break;
    case 9:
        if (time_info.hours > 6 && time_info.hours < 12) {
            sprintf(buf, "say Good morning, %s", GET_NAME(t));
        } else if (time_info.hours >= 12 && time_info.hours < 20) {
            sprintf(buf, "say Good afternoon, %s", GET_NAME(t));
        } else if (time_info.hours >= 20 && time_info.hours <= 24) {
            sprintf(buf, "say Good evening, %s", GET_NAME(t));
        } else {
            sprintf(buf, "say Up for a midnight stroll, %s?\n", GET_NAME(t));
        }
        command_interpreter(ch, buf);
        break;
    case 10:
        if (time_info.hours < 6) {
            strcpy(buf2, "evening");
        } else if (time_info.hours < 12) {
            strcpy(buf2, "morning");
        } else if (time_info.hours < 20) {
            strcpy(buf2, "afternoon");
        } else {
            strcpy(buf2, "evening");
        }

        switch (weather_info.sky) {
        case SKY_CLOUDLESS:
            sprintf(buf, "say Lovely weather we're having this %s, isn't it, "
                         "%s.", buf2, GET_NAME(t));
        case SKY_CLOUDY:
            sprintf(buf, "say Nice %s to go for a walk, %s.", buf2, 
                    GET_NAME(t));
            break;
        case SKY_RAINING:
            sprintf(buf, "say I hope %s's rain clears up.. don't you %s?",
                    buf2, GET_NAME(t));
            break;
        case SKY_LIGHTNING:
            sprintf(buf, "say How can you be out on such a miserable %s, %s!",
                    buf2, GET_NAME(t));
            break;
        default:
            sprintf(buf, "say Such a pleasant %s, don't you think?", buf2);
            break;
        }
        command_interpreter(ch, buf);
        break;
    }
}

void Submit(struct char_data *ch, struct char_data *t)
{
    char            buf[200];

    switch (number(1, 5)) {
    case 1:
        sprintf(buf, "bow %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 2:
        sprintf(buf, "smile %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 3:
        sprintf(buf, "wink %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    case 4:
        sprintf(buf, "wave %s", GET_NAME(t));
        command_interpreter(ch, buf);
        break;
    default:
        act("$n nods $s head at you", 0, ch, 0, t, TO_VICT);
        act("$n nods $s head at $N", 0, ch, 0, t, TO_NOTVICT);
        break;
    }
}

int Summoner(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *targ = 0;
    struct char_list *i;
    char            buf[255];
    extern char     EasySummon;
    char           *name;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (TRUE);
    }
    /*
     * wait till at 75% of hitpoints. 
     */
    if (GET_HIT(ch) > ((GET_MAX_HIT(ch) * 3) / 4)) {
        /*
         * check for hatreds 
         */
        if (IS_SET(ch->hatefield, HATE_CHAR) && ch->hates.clist) {
            for (i = ch->hates.clist; i; i = i->next) {
                if (i->op_ch) {
                    /* 
                     * if there is a char_ptr 
                     */
                    targ = i->op_ch;
                    if (IS_PC(targ)) {
                        sprintf(buf, "You hate %s\n\r", targ->player.name);
                        send_to_char(buf, ch);
                        break;
                    }
                }
            }
        }

        if (targ) {
            name = strdup(targ->player.name);
            act("$n utters the words 'Your ass is mine!'", 1, ch, 0, 0,
                TO_ROOM);
            if (EasySummon == 1) {
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_summon(GetMaxLevel(ch), ch, targ, 0);
                } else if (GET_MAX_HIT(targ) <= GET_HIT(ch)) {
                    ch->skills[SKILL_SUMMON].learned = 100;
                    sprintf( buf, "summon %s", name );
                    command_interpreter(ch, buf);
                    ch->points.mana = 100;
                } else {
                    ch->skills[SKILL_PORTAL].learned = 100;
                    sprintf( buf, "portal %s", name );
                    command_interpreter(ch, buf);
                    ch->points.mana = 100;
                }
            } else {
                if (GetMaxLevel(ch) > 32 && number(0, 3)) {
                    command_interpreter(ch, "say Curses!  Foiled again!\n\r");
                    if( name ) {
                        free( name );
                    }
                    return (0);
                }

                /*
                 * Easy Summon was turned off and they were > 32nd level 
                 * so we portal to them! 
                 */
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_portal(GetMaxLevel(ch), ch, targ, 0);
                    command_interpreter(ch, "enter portal");
                } else {
                    /* 
                     * its a psi summoner, do his stuff
                     *
                     * with easy_summon turned off must portal, so.. 
                     */
                    if (!ch->skills[SKILL_PORTAL].learned) {
                        ch->skills[SKILL_PORTAL].learned = 100;
                    }
                    sprintf(buf, "portal %s", name);
                    command_interpreter(ch, targ->player.name);
                    ch->points.mana = 100;
                }
            }

            if (targ->in_room == ch->in_room) {
                if (NumCharmedFollowersInRoom(ch) > 0) {
                    sprintf(buf, "order followers kill %s", GET_NAME(targ));
                    command_interpreter(ch, buf);
                }
                act("$n says, 'And now my young $N... You will DIE!", 0,
                    ch, 0, targ, TO_ROOM);
                if (!IS_SET(ch->player.class, CLASS_PSI)) {
                    spell_dispel_magic(GetMaxLevel(ch), ch, targ, 0);
                } else {
                    sprintf(buf, "blast %s", name);
                    command_interpreter(ch, buf);
                }
            }

            if( name ) {
                free( name );
            }
            return (FALSE);
        } else {
            return (FALSE);
        }
    } else {
        return (FALSE);
    }
}

int DruidAttackSpells(struct char_data *ch, struct char_data *vict,
                      int level)
{
    switch (level) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
        act("$n utters the words 'yow!'", 1, ch, 0, 0, TO_ROOM);
        cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
        if (!IS_SET(vict->M_immune, AFF_POISON) &&
            !IS_AFFECTED(vict, AFF_POISON)) {
            act("$n utters the words 'yuk'", 1, ch, 0, 0, TO_ROOM);
            cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'ouch'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                               vict, 0);
        }
        return (FALSE);
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
        act("$n utters the words 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
        cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
        if (!IS_SET(vict->M_immune, IMM_FIRE)) {
            act("$n utters the words 'fwoosh'", 1, ch, 0, 0, TO_ROOM);
            cast_firestorm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        } else {
            act("$n utters the words 'OUCH!'", 1, ch, 0, 0, TO_ROOM);
            cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
        }
        return (FALSE);
        break;
    default:
        act("$n utters the words 'kazappapapapa'", 1, ch, 0, 0, TO_ROOM);
        cast_chain_lightn(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
        return (FALSE);
        break;
    }
}

int GenericCityguardHateUndead(struct char_data *ch, int cmd, char *arg,
                               struct char_data *mob, int type)
{
    struct char_data *tch,
                   *evil;
    int             max_evil;
    char            buf[MAX_STRING_LENGTH];

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (!check_soundproof(ch)) {
            if (number(0, 100) == 0) {
                command_interpreter(ch, "shout To me, my fellows! I am in "
                                        "need of thy aid!");
            } else {
                act("$n shouts 'To me, my fellows! I need thy aid!'",
                    TRUE, ch, 0, 0, TO_ROOM);
            }

            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, type);
            }
            return (TRUE);
        }
    }

    max_evil = 0;
    evil = 0;

    if (check_peaceful(ch, "")) {
        return FALSE;
    }

    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if ((IS_NPC(tch) && (IsUndead(tch)) && CAN_SEE(ch, tch)) || 
            (IsGoodSide(ch) && IsBadSide(tch) && CAN_SEE(ch, tch)) || 
            (IsGoodSide(tch) && IsBadSide(ch) && CAN_SEE(ch, tch))) {

            max_evil = -1000;
            evil = tch;
            if (!check_soundproof(ch)) {
                act("$n screams 'EVIL!!! BANZAI!!'", FALSE, ch, 0, 0, TO_ROOM);
            }
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        }

        if (!IS_PC(tch) && tch->specials.fighting &&
            GET_ALIGNMENT(tch) < max_evil && 
            (!IS_PC(tch) || !IS_PC(tch->specials.fighting))) {
            max_evil = GET_ALIGNMENT(tch);
            evil = tch;
        }
    }

    if (evil && (GET_ALIGNMENT(evil->specials.fighting) >= 0)) {
        if (IS_MURDER(evil) || 
            GET_HIT(evil->specials.fighting) > GET_HIT(evil) ||
            evil->specials.fighting->attackers > 3) {
            if (!check_soundproof(ch)) {
                act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!!'",
                    FALSE, ch, 0, 0, TO_ROOM);
            }
            hit(ch, evil, TYPE_UNDEFINED);
            return (TRUE);
        } else if (!IS_MURDER(evil->specials.fighting)) {
            if (!check_soundproof(ch)) {
                act("$n yells 'There's no need to fear! $n is here!'",
                    FALSE, ch, 0, 0, TO_ROOM);
            }

            if (!ch->skills) {
                SpaceForSkills(ch);
            }
            if (!ch->skills[SKILL_RESCUE].learned) {
                ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch) * 3 + 30;
            }
            sprintf( buf, "rescue %s", GET_NAME(evil->specials.fighting) );
            command_interpreter(ch, buf );
        }
    }

    GreetPeople(ch);

    return (FALSE);
}

int GenericCityguard(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    struct char_data *tch,
                   *evil;
    int             max_evil;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        fighter(ch, cmd, arg, mob, type);

        if (!check_soundproof(ch)) {
            if (number(0, 120) == 0) {
                command_interpreter(ch, "shout To me, my fellows! I am in "
                                        "need of thy aid!");
            } else {
                act("$n shouts 'To me, my fellows! I need thy aid!'",
                    TRUE, ch, 0, 0, TO_ROOM);
            }

            if (ch->specials.fighting) {
                CallForGuard(ch, ch->specials.fighting, 3, type);
            }
            return (TRUE);
        }
    }

    max_evil = 1000;
    evil = 0;

    if (check_peaceful(ch, "")) {
        return FALSE;
    }
    for (tch = real_roomp(ch->in_room)->people; tch;
         tch = tch->next_in_room) {
        if (tch->specials.fighting && GET_ALIGNMENT(tch) < max_evil &&
            (IS_NPC(tch) || IS_NPC(tch->specials.fighting))) {
            max_evil = GET_ALIGNMENT(tch);
            evil = tch;
        }
    }

    if (evil && GET_ALIGNMENT(evil->specials.fighting) >= 0) {
        if (!check_soundproof(ch)) {
            act("$n screams 'PROTECT THE INNOCENT! BANZAI!!! CHARGE!!! SPOON!'",
                FALSE, ch, 0, 0, TO_ROOM);
        }
        hit(ch, evil, TYPE_UNDEFINED);
        return (TRUE);
    }

    GreetPeople(ch);

    return (FALSE);
}

struct breath_victim {
    struct char_data *ch;
    int             yesno;      /* 1 0 */
    struct breath_victim *next;
};

struct breath_victim *choose_victims(struct char_data *ch,
                                     struct char_data *first_victim)
{
    /*
     * this is goofy, dopey extraordinaire 
     */
    struct char_data *cons;
    struct breath_victim *head = NULL,
                   *temp = NULL;

    for (cons = real_roomp(ch->in_room)->people; cons;
         cons = cons->next_in_room) {
        temp = (struct breath_victim *) malloc(sizeof(struct breath_victim));
        temp->ch = cons;
        temp->next = head;
        head = temp;
        if (first_victim == cons) {
            temp->yesno = 1;
        } else if (ch == cons) {
            temp->yesno = 0;
        } else if (in_group(first_victim, cons) ||
                   cons == first_victim->master ||
                   cons->master == first_victim) {
            /*
             * group members will get hit 2/5 times 
             */
           temp->yesno = (dice(1, 5) < 3);
        } else if (cons->specials.fighting == ch) {
            /*
             * people fighting the dragon get hit 4/5 times 
             */
            temp->yesno = (!dice(1, 5));
        } else {
            /* 
             * bystanders get his 3/5 times 
             */
            temp->yesno = (dice(1, 5) < 4);
        }
    }
    return head;
}

void free_victims(struct breath_victim *head)
{
    struct breath_victim *temp;

    while (head) {
        temp = head->next;
        if (head) {
            free(head);
        }
        head = temp;
    }
}

void breath_weapon(struct char_data *ch, struct char_data *target,
                   int mana_cost, void (*func) ())
{
    struct breath_victim *hitlist,
                   *scan;
    struct char_data *tmp;
    int             victim;

    act("$n rears back and inhales", 1, ch, 0, ch->specials.fighting, TO_ROOM);
    victim = 0;

    for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room) {
        if (tmp != ch && !IS_IMMORTAL(tmp)) {
            victim = 1;
            cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, tmp, 0);
        }
    }

    hitlist = choose_victims(ch, target);

    if (func != NULL && victim && hitlist) {
        act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

        for (scan = hitlist; scan; scan = scan->next) {
            if (scan->yesno && !IS_IMMORTAL(scan->ch) && 
                scan->ch->in_room == ch->in_room) {
                func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
            }
        }
        GET_MANA(ch) -= mana_cost;
    } else {
        act("$n Breathes...coughs and sputters...", 1, ch, 0, 
            ch->specials.fighting, TO_ROOM);
        do_flee(ch, NULL, 0);
    }

    free_victims(hitlist);
}

void use_breath_weapon(struct char_data *ch, struct char_data *target,
                       int cost, void (*func) ())
{
    if (GET_MANA(ch) >= 0) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 2 && GET_MANA(ch) >= -cost) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_HIT(ch) < GET_MAX_HIT(ch) / 4 && GET_MANA(ch) >= -2 * cost) {
        breath_weapon(ch, target, cost, func);
    } else if (GET_MANA(ch) <= -3 * cost) {
        /* 
         * sputter 
         */
        breath_weapon(ch, target, 0, NULL);
    }
}

void DruidHeal(struct char_data *ch, int level)
{
    if (level > 13) {
        act("$n utters the words 'Woah! I feel GOOD! Heh.'.",
            1, ch, 0, 0, TO_ROOM);
        cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    } else if (level > 8) {
        act("$n utters the words 'I feel much better now!'.",
            1, ch, 0, 0, TO_ROOM);
        cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    } else {
        act("$n utters the words 'I feel good!'.", 1, ch, 0, 0, TO_ROOM);
        cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    }
}

void DruidTree(struct char_data *ch)
{
    act("$n utters the words 'harumph!'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n takes on the form and shape of a huge tree!", FALSE, ch, 0, 0,
        TO_ROOM);
    GET_RACE(ch) = RACE_TREE;
    ch->points.max_hit = GetMaxLevel(ch) * 10;
    ch->points.hit += GetMaxLevel(ch) * 5;

    if (ch->player.long_descr) {
        free(ch->player.long_descr);
    }
    if (ch->player.short_descr) {
        free(ch->player.short_descr);
    }
    ch->player.short_descr = (char *) strdup("The druid-tree");
    ch->player.long_descr = (char *) strdup("A huge tree stands here");
    GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
    ch->mult_att = 6;
    ch->specials.damsizedice = 6;
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
}

void DruidMob(struct char_data *ch)
{
    act("$n utters the words 'lagomorph'", FALSE, ch, 0, 0, TO_ROOM);
    act("$n takes on the form and shape of a huge lion", FALSE, ch, 0, 0,
        TO_ROOM);
    GET_RACE(ch) = RACE_PREDATOR;
    ch->points.max_hit *= 2;
    ch->points.hit += GET_HIT(ch) / 2;

    if (ch->player.long_descr) {
        free(ch->player.long_descr);
    }
    if (ch->player.short_descr) {
        free(ch->player.short_descr);
    }
    ch->player.short_descr = strdup("The druid-lion");
    ch->player.long_descr = strdup("A huge lion stands here, his tail twitches"
                                   " menacingly");
    GET_HIT(ch) = MIN(GET_MAX_HIT(ch), GET_HIT(ch));
    ch->mult_att = 3;
    ch->specials.damnodice = 3;
    ch->specials.damsizedice = 4;
    REMOVE_BIT(ch->specials.act, ACT_SPEC);
}

int AcidBreather(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0010A stream of hot acid bursts forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_acid_breath(GetMaxLevel(ch), ch,
                                  ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int acid_monster(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n spits a stream of acid at $N!", FALSE, ch, 0,
            ch->specials.fighting, TO_NOTVICT);
        act("$n spits a stream of acid at you!", FALSE, ch, 0,
            ch->specials.fighting, TO_VICT);
        spell_acid_breath(GetMaxLevel(ch), ch, ch->specials.fighting, 0);
        return (TRUE);
    }
    return (FALSE);
}

int range_estimate(struct char_data *ch, struct obj_data *o, int type)
{
    int             r,
                    sz,
                    w = o->weight;

    /*
     * Type 0 = fireweapon 1 = thrown 
     */
    if (w > 100) {
        sz = 3;
    } else if (w > 25) {
        sz = 2;
    } else if (w > 5) {
        sz = 1;
    } else {
        sz = 0;
    }
    r = ((GET_STR(ch) + (GET_ADD(ch) / 30) - 3) / 8) + 2;
    r /= sz + 1;

    if (!type) {
        r += ch->equipment[WIELD]->value[2];
    }
    r = r * 2 / 3;
    return r;
}

int pick_archer_target(struct char_data *ch, int maxr,
                       struct char_data **targ, int *rn, int *dr)
{
    int             i,
                    r,
                    rm;
    struct char_data *ptarg;

    if (maxr > 5) {
        maxr = 5;
    }
    for (i = 0; i < 6; i++) {
        rm = ch->in_room;
        r = 0;

        while (r <= maxr) {
            if (clearpath(ch, rm, i)) {
                r++;

                rm = real_roomp(rm)->dir_option[i]->to_room;
                for (ptarg = real_roomp(rm)->people; ptarg;
                     ptarg = ptarg->next_in_room) {
                    /*
                     * find PC's to kill if we are aggressive 
                     */
                    if (IS_PC(ptarg) && CAN_SEE(ch, ptarg) &&
                        (IS_SET(ch->specials.act, ACT_AGGRESSIVE) || 
                         IS_SET(ch->specials.act, ACT_META_AGG))) {
                        *targ = ptarg;
                        *rn = r;
                        *dr = i;
                        return (TRUE);
                    } else if (CAN_SEE(ch, ptarg) && 
                               IS_SET(ch->specials.act, ACT_HUNTING) && 
                               ((ch->specials.hunting && 
                                 ch->specials.hunting == ptarg) || 
                                (ch->specials.charging && 
                                 ch->specials.charging == ptarg))) {
                        /*
                         * find people we are hunting 
                         */
                        *targ = ptarg;
                        *rn = r;
                        *dr = i;
                        return (TRUE);
                    } else if (CAN_SEE(ch, ptarg) && Hates(ch, ptarg)) {
                        /*
                         * find hated peoples 
                         */
                        *targ = ptarg;
                        *rn = r;
                        *dr = i;
                        return (TRUE);
                    }
                }
            } else {
                r = maxr + 1;
            }
        }
    }
    return FALSE;
}

int archer_sub(struct char_data *ch)
{
    struct obj_data *bow = NULL,
                   *missile = NULL,
                   *thrown = NULL,
                   *spid = NULL,
                   *obj_object = NULL,
                   *next_obj = NULL;
    int             r,
                    a,
                    b,
                    found;
    char            buf[MAX_STRING_LENGTH];
    struct char_data *td;
    char           *temp;

    if (ch->equipment[WIELD] && 
        ch->equipment[WIELD]->type_flag == ITEM_TYPE_FIREWEAPON) {
        bow = ch->equipment[WIELD];
        if (ch->equipment[LOADED_WEAPON]) {
            missile = ch->equipment[LOADED_WEAPON];
        } else {
            /*
             * Search inventory for a missile 
             */
            for (spid = ch->carrying; spid; spid = spid->next_content) {
                if (spid->type_flag == ITEM_TYPE_MISSILE &&
                    spid->value[3] == bow->value[2]) {
                    missile = spid;
                    continue;
                } 

                /*
                 * see if they are carrying a quiver full of arrows,
                 * if so get an arrow 
                 */
                if (ITEM_TYPE(spid) == ITEM_TYPE_CONTAINER) {
                    found = FALSE;
                    for (obj_object = spid->contains;
                         obj_object && !found; obj_object = next_obj) {
                        next_obj = obj_object->next_content;
                        if (obj_object->type_flag == ITEM_TYPE_MISSILE && 
                            obj_object->value[3] == bow->value[2]) {
                            /*
                             * gets arrow out of quiver, next round
                             * they will load it 
                             */
                            get(ch, obj_object, spid);
                            found = TRUE;
                        }
                    }
                }
            }

            /*
             * If you found a missile, load it and return 
             */
            if (missile) {
                temp = KeywordsToString( &missile->keywords, NULL );
                do_weapon_load(ch, temp, 0);
                free( temp );
                return TRUE;
            }
        }

        if (missile) {
            /*
             * Pick a target and fire 
             */
            r = range_estimate(ch, missile, 0);
            if (pick_archer_target(ch, r, &td, &a, &b)) {
                sprintf( buf, "fire %s", td->player.name );
                command_interpreter(ch, buf);
                return( TRUE );
            } 

            return( FALSE );
        }
    }

    /*
     * Try and find a missile weapon and wield it 
     */
    if (!bow) {
        for (spid = ch->carrying; spid; spid = spid->next_content) {
            if (spid->type_flag == ITEM_TYPE_FIREWEAPON) {
                bow = spid;
            }

            if (ITEM_TYPE(spid) == ITEM_TYPE_CONTAINER) {
                found = FALSE;
                for (obj_object = spid->contains;
                     obj_object && !found; obj_object = next_obj) {
                    next_obj = obj_object->next_content;
                    if (obj_object->type_flag == ITEM_TYPE_FIREWEAPON) {
                        /*
                         * gets bow out of container 
                         */
                        get(ch, obj_object, spid);
                        found = TRUE;
                        bow = obj_object;
                    }
                }
            }
        }

        if (bow) {
            if (ch->equipment[WIELD]) {
                temp = KeywordsToString( &ch->equipment[WIELD]->keywords, 
                                         NULL );
                sprintf(buf, "remove %s", temp);
                free( temp );
                command_interpreter(ch, buf);
                return( TRUE );
            }

            temp = KeywordsToString( &bow->keywords, NULL );
            do_wield(ch, temp, 0);
            free(temp);
            return( TRUE );
        }
    }

    /*
     * No missile weapon or no ammo.  Try a thrown weapon 
     */
    for (spid = ch->carrying; spid; spid = spid->next_content) {
        if (IS_OBJ_STAT(spid, wear_flags, ITEM_THROW)) {
            thrown = spid;
        }
    }

    if (!thrown) {
        /*
         * Just can't do nothing. 
         */
        return( FALSE );
    }

    /*
     * Got a throw weapon, throw it. 
     */
    r = range_estimate(ch, thrown, 1);
    if (pick_archer_target(ch, r, &td, &a, &b)) {
        temp = KeywordsToString( &thrown->keywords, NULL );
        sprintf(buf, "throw %s %s", temp, td->player.name);
        free( temp );
        command_interpreter(ch, buf);
        return( TRUE );
    } 

    return( FALSE );
}

int archer_hth(struct char_data *ch)
{
    struct  obj_data    *spid,
                        *hth = NULL;
    char    buf[MAX_STRING_LENGTH];
    char   *temp;

    /*
     * What to do if you are an archer and find yourself in HTH combat 
     * I. If you are wielding a bow ditch it 
     */
    if (ch->equipment[WIELD] && 
        ch->equipment[WIELD]->type_flag == ITEM_TYPE_FIREWEAPON) {
        temp = KeywordsToString( &ch->equipment[WIELD]->keywords, NULL );
        sprintf(buf, "remove %s", temp);
        free( temp );
        command_interpreter(ch, buf);
        return( TRUE );
    } 
    
    if (ch->equipment[WIELD]) {
        return( FALSE );
    }
    for (spid = ch->carrying; spid; spid = spid->next_content) {
        if (IS_WEAPON(spid)) {
            hth = spid;
        }
    }

    if (hth) {
        temp = KeywordsToString( &hth->keywords, NULL );
        do_wield(ch, temp, 14);
        free( temp );
        return( TRUE );
    }
    return( FALSE );
}

int archer(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }

    if (GET_POS(ch) == POSITION_FIGHTING) {
        return (archer_hth(ch));
    } else {
        return (archer_sub(ch));
    }

    /* should never get here */
    return (archer_sub(ch));
}

int athos(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd) {
        return 0;
    }
    switch (number(0, 20)) {
    case 0:
        act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 1:
        act("$n grimaces.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 2:
        act("$n asks you, 'Have you seen the lady, pale and fair, with a heart"
            " of stone?'", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say That monster will be the death of us "
                                "all.");
        break;
    case 3:
        command_interpreter(ch, "say God save the King!");
        break;
    case 4:
        command_interpreter(ch, "say All for one and .. one for...");
        act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 5:
        act("$n looks up with a philosophical air.", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say Women - God's eternal punishment on man.");
        break;
    case 6:
        act("$n downs his glass and leans heavily on the oaken table.",
            FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say You know, we would best band together and"
                                " wrestle the monstrous woman from her lair "
                                "and home!");
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}

int baby_bear(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    struct char_data *t;
    struct room_data *rp;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        command_interpreter(ch, "scream");
        rp = real_roomp(ch->in_room);
        if (!rp) {
            return (FALSE);
        }
        for (t = rp->people; t; t = t->next_in_room) {
            if (GET_POS(t) == POSITION_SLEEPING && number(0, 1)) {
                do_wake(t, NULL, 0);
            }
        }
    }
    return (FALSE);
}

int banshee(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *tmp;
    int             victim;

    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (ch->specials.fighting && 
        ch->specials.fighting->in_room == ch->in_room) {
        if (IS_DARK(ch->in_room)) {
            for (tmp = real_roomp(ch->in_room)->people; tmp;
                 tmp = tmp->next_in_room) {
                if (tmp != ch && !IS_IMMORTAL(tmp)) {
                    victim = 1;
                    act("$n wails loudly draining the life from your body!", 
                        TRUE, ch, 0, 0, TO_ROOM);
                    /*
                     * do wail damage stuff here  - BUSTED!
                     */
                }
            }
        } else if (number(0, 1)) {
            return (magic_user(mob, cmd, arg, mob, type));
        } else {
            tmp = ch->specials.fighting;
            if (GetMaxLevel(tmp) <= 1) {
                Log("Banshee tried to drain char below level 1");
            } else {
                act("$n bites $N who looks drained!", FALSE, ch, 0, tmp,
                    TO_ROOM);
                act("$n bites you hard!", FALSE, ch, 0, tmp, TO_CHAR);
                drop_level(tmp, BestClassIND(tmp), FALSE);
            }
            set_title(tmp);
            do_save(tmp, NULL, 0);
        }
    }
    return (FALSE);
}

int Barbarian(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    return (fighter(ch, cmd, arg, mob, type));
}

int Beholder(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    int             action = 0;

    if (cmd || !AWAKE(mob)) {
        return (FALSE);
    }
    if (mob->specials.fighting) {
        if (GET_POS(mob) == POSITION_SITTING) {
            do_stand(mob, NULL, 0);
            return (TRUE);
        }

        action = number(1, 100);

        if (action >= 90) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_fear(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                      mob->specials.fighting, 0);
            return (TRUE);
        } else if (action >= 80) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_slow(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                      mob->specials.fighting, 0);
            return (TRUE);
        } else if (action >= 75) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_disintegrate(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                              mob->specials.fighting, 0);
            return (TRUE);
        } else if (action >= 50) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_dispel_magic(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                              mob->specials.fighting, 0);
            return (TRUE);
        } else if (action >= 30) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_charm_monster(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                               mob->specials.fighting, 0);
            return (TRUE);
        } else if (action >= 10) {
            act("$n turns one of $m eye stalks at $N!", FALSE, mob, 0,
                mob->specials.fighting, TO_NOTVICT);
            act("$n turns a evil looking eye stalk at you!", FALSE, mob, 0,
                mob->specials.fighting, TO_VICT);
            cast_harm(GetMaxLevel(mob), mob, "", SPELL_TYPE_SPELL,
                      mob->specials.fighting, 0);
            return (TRUE);
        }
    }
    return (magic_user(ch, cmd, arg, mob, type));
}

int blinder(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if ((cmd) || (GET_POS(ch) != POSITION_FIGHTING)) {
        return FALSE;
    }
    if (ch->specials.fighting && 
        ch->specials.fighting->in_room == ch->in_room && 
        number(0, 100) + GET_LEVEL(ch, BestClassIND(ch) >= 50)) {
        act("$n whispers, 'So, $N! You wouldst share my affliction!", 1,
            ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("$n whispers, 'So, $N! You wouldst share my affliction!", 1,
            ch, 0, ch->specials.fighting, TO_VICT);
        act("$n's frayed cloak blows as he points at $N.", 1, ch, 0,
            ch->specials.fighting, TO_NOTVICT);
        act("$n's frayed cloak blows as he aims a bony finger at you.", 1,
            ch, 0, ch->specials.fighting, TO_VICT);
        act("A flash of pale fire explodes in $N's face!", 1, ch, 0,
            ch->specials.fighting, TO_NOTVICT);
        act("A flash of pale fire explodes in your face!", 1, ch, 0,
            ch->specials.fighting, TO_VICT);
        cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                       ch->specials.fighting, 0);

        return TRUE;
    }
    return FALSE;
}

/*
 * Braxis the Swamp Dragon 
 */
int braxis_swamp_dragon(struct char_data *ch, int cmd, char *arg,
                        struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                   *temp,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0;
    int             has_marbles = 0;
    struct obj_data *i;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * TALK TO ME!!!
     */
    if (cmd == 531) {
        /* 
         * Talk 
         */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }
        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != BRAXIS) {
            return (FALSE);
        }

        for (i = vict->carrying; i && !has_marbles; i = i->next_content) {
            if (i->item_number == MARBLES) {
                has_marbles = 1;
            }
        }

        /*
         * Quest Text
         */
        if (has_marbles == 0) {
            act("$n says, 'Kind warriors, I seem to have misplaced my "
                "marbles.'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'How I so long to play with them again. If you can'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'return my marbles, I will tell you of a great land'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'of riches and powerful items.'", FALSE, vict, 0,
                0, TO_ROOM);
            act("$n shivers in anticipation.", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'Ignatius will fall to my master marble skills if I'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'can ever find them.'", FALSE, vict, 0, 0, TO_ROOM);
            return (TRUE);
        } else {
            act("$n says, 'Please leave me alone while I practice my game.'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'Ignatius will fall to my wrath.'", FALSE, vict,
                0, 0, TO_ROOM);
            return (TRUE);
        }
    } else if (cmd == 72) {
        arg = get_argument(arg, &obj_name);
        if (!obj_name || 
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) ||
            IS_PC(vict)) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != BRAXIS) {
            return (FALSE);
        }

        /*
         * If object is not Marbles
         */
        if (!IS_IMMORTAL(ch)) {
            if (obj->item_number != MARBLES) {
                sprintf(buf, "tell %s That is not the item I seek.", 
                        GET_NAME(ch));
                command_interpreter(vict, buf);
                return (TRUE);
            }
            test = 1;
            temp = strdup("bag-marbles braxis-swamp-dragon");
            do_give(ch, temp, 0);
            if( temp ) {
                free( temp );
            }
        } else {
            sprintf(buf, "give %s %s", obj_name, vict_name);
            command_interpreter(ch, buf);
            if (obj->item_number == MARBLES) {
                test = 1;
            } else {    
                return (TRUE);
            }
        }
    } else {
        return (FALSE);
    }

    if (test == 1) {
        /* It is the Braxis, and the marbles */
        act("$n says, 'There is a mighty power in the sceptre that rests in'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'the Great King's throne room. However, this power has'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'been split into three separate pieces that must be "
            "united.'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'You must reunite the head of the sceptre with its "
            "severed'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'eyes.  Once you do so you will be transported to a "
            "wondrous'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'world of monsters and danger.  You must be brave as "
            "well as'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'strong to survive, but if you do, the rewards will be "
            "great.'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n hands over a couple of items.", FALSE, vict, 0, 0, TO_ROOM);

        obj = objectRead(EYE_DRAGON, VIRTUAL);
        objectGiveToChar(obj, ch);

        obj = objectRead(GATEKEEPER_KEY, VIRTUAL);
        objectGiveToChar(obj, ch);

        return (TRUE);
    }

    return (FALSE);
}

int butcher(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if (cmd) {
        return 0;
    }
    switch (number(0, 40)) {
    case 0:
        command_interpreter(ch, "say I need a Union.");
        act("$n glares angrily.", FALSE, ch, 0, 0, TO_ROOM);
        act("$n rummages about for an axe.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 1:
        act("$n gnaws on a toothpick.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 2:
        act("$n runs a finger along the edge of a giant meat cleaver.",
            FALSE, ch, 0, 0, TO_ROOM);
        act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    case 3:
        command_interpreter(ch, "say Pork for sale!");
        break;
    case 4:
        act("$n whispers to you, 'I've got some great damage eq in the back "
            "room. Wanna see?'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n throws back his head and cackles with insane glee!", FALSE,
            ch, 0, 0, TO_ROOM);
        break;
    case 5:
        act("$n yawns.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 6:
        act("$n throws an arm around the headless body of an ogre and asks to "
            "have his picture taken.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 7:
        act("$n listlessly grabs a cleaver and hurls it into the wall behind "
            "your head.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 8:
        act("$n juggles some fingers.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 9:
        act("$n eyes your limbs.", FALSE, ch, 0, 0, TO_ROOM);
        act("$n chuckles.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 10:
        command_interpreter(ch, "say Hi, Alice.");
        break;
    case 11:
        command_interpreter(ch, "say Everyone looks like food to me these "
                                "days.");
        act("$n sighs loudly.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 12:
        act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,
            TO_ROOM);
        act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 13:
        command_interpreter(ch, "say The worms crawl in, the worms crawl "
                                "out..");
        break;
    case 14:
        act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n whistles a tune.", FALSE, ch, 0, 0, TO_ROOM);
        act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 15:
        command_interpreter(ch, "say Get Lurch to bring me over a case and "
                                "I'll sport you a year's supply of grilled "
                                "ogre.");
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}

int CorsairPush(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *targ,
                   *target;
    int             location;

    if (cmd && cmd != 156) {
        return (FALSE);
    }
    if (AWAKE(ch) && (targ = FindAnAttacker(ch)) != '\0') {
        act("$N pushes you off the ship.", TRUE, targ, 0, ch, TO_CHAR);
        act("$N gives $n a good push off the ship.", TRUE, targ, 0, ch,
            TO_ROOM);
        if (!(target = get_char_vis_world(ch, "ship", NULL))) {
            send_to_char("Where did that darn ship go??.\n\r", ch);
            return(FALSE);
        }

        /* 
         * Find the room that the corsair ship is in 
         */
        location = target->in_room;
        char_from_room(targ);
        char_to_room(targ, location);

        act("$n suddenly falls from above.", TRUE, targ, 0, 0, TO_ROOM);
        act("You fall helplessly downward off the ship.", FALSE, ch, 0,
            0, TO_CHAR);
    }
    return( TRUE );
}

int cronus_pool(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    char           *buf;
    struct char_data *portal;

    if (cmd != 7) {
        /* 
         * enter 
         */
        return (FALSE);
    }

    arg = get_argument(arg, &buf);
    if (buf) {
        if (!strcasecmp("pool", buf) || !strcasecmp("color", buf) ||
            !strcasecmp("color pool", buf)) {
            if ((portal = get_char_room("pool", ch->in_room))) {
                send_to_char("\n\r", ch);
                send_to_char("You attempt to enter the pool, and it "
                             "gives.\n\r", ch);
                send_to_char("You press on further and the pool surrounds you, "
                             "like some soft membrane.\n\r", ch);
                send_to_char("There is a slight wrenching sensation, and then "
                             "the color disappears.\n\r", ch);
                send_to_char("\n\r", ch);
                act("$n enters the pool and disappears!", FALSE, ch, 0, 0,
                    TO_ROOM);
                char_from_room(ch);
                char_to_room(ch, BAHAMUT_HOME);
                act("$n appears in a dazzling explosion of light!", FALSE,
                    ch, 0, 0, TO_ROOM);
                do_look(ch, NULL, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

int DarkBreather(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0008A cloud of oppressive darkness surges forth from $n's "
            "mouth!", FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_dark_breath(GetMaxLevel(ch), ch,
                                  ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int death_knight(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (number(0, 1)) {
        return (fighter(mob, cmd, arg, mob, type));
    } else {
        return (magic_user(mob, cmd, arg, mob, type));
    }
}

int DehydBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0005A searing cone of dehydration billows forth from $n's"
            " mouth!", FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_dehydration_breath(GetMaxLevel(ch), ch,
                                         ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

/*
 * This procs allows a mob to dispel and incernary cloud someone after
 * they miss a bash By Greg Hovey Feb 25, 2001 
 */

int DispellerIncMob(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    struct char_data *targ;

    if (cmd && cmd != 156) {
        return (FALSE);
    }
    if (cmd == 156) {
        send_to_char("You're much too afraid to steal anything!\n\r", ch);
        return (TRUE);
    }

    if (AWAKE(ch) && (targ = FindAnAttacker(ch)) != '\0') {
        act("$n slowly raises $s hands.", TRUE, ch, 0, 0, TO_ROOM);
        if (!saves_spell(targ, SAVING_PARA)) {
            act("Suddenly $n utters some strange words!",
                TRUE, ch, 0, targ, TO_ROOM);
            cast_dispel_magic(60, ch, "", SPELL_TYPE_SPELL, targ, 0);
            cast_incendiary_cloud(60, ch, "", SPELL_TYPE_SPELL, targ, 0);
        }
    }
    return( TRUE );
}

/*
 * Elamin 
 */
int elamin(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    char           *obj_name,
                   *vict_name,
                   *temp,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0;
    int             has_pen = 0,
                    has_key = 0;
    struct obj_data *i;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * TALK TO ME!!!
     */

    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != ELAMIN) {
            return (FALSE);
        }

        for (i = vict->carrying; i && !has_pen; i = i->next_content) {
            if (i->item_number == PEN_MIGHT) {
                has_pen = 1;
            }
        }

        /*
         * Quest Text
         */
        if (has_pen == 0) {
            act("$n says, 'Hello there.  How did you manage to get so far "
                "into'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'the keep?  Nevermind, it is of no importance.  So'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'you are looking for the infamous realm of dragons "
                "eh?'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'Very well.  I'll help you since that bugger of a "
                "king'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'locked me up here.  I will give you the sceptre "
                "head'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'if you do me a favor.  This pain in the neck "
                "jester'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'took a very powerful magical item from me a short "
                "time'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'back.  If you will go to the Jester and obtain my "
                "pen'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'and bring it back to me, the sceptre will be "
                "yours.'", FALSE, vict, 0, 0, TO_ROOM);

            for (i = vict->carrying; i && !has_key; i = i->next_content) {
                if (i->item_number == JESTER_KEY) {
                    has_key = 1;
                }
            }

            if (has_key == 1) {
                sprintf(buf, "give skeleton-key %s", GET_NAME(ch));
                command_interpreter(vict, buf);
            }
            return (TRUE);
        } else {
            act("$n says, 'Please depart at your convenience.  I have work to"
                " do'", FALSE, vict, 0, 0, TO_ROOM);
            return (TRUE);
        }
    } else if (cmd == 72) {
        arg = get_argument(arg, &obj_name);
        if (!obj_name || 
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict)) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != ELAMIN) {
            return (FALSE);
        }

        if (!IS_IMMORTAL(ch)) {
            if (obj->item_number != PEN_MIGHT) {
                sprintf(buf, "tell %s That is not the item I seek.", 
                        GET_NAME(ch));
                command_interpreter(vict, buf);
                return (TRUE);
            }
            test = 1;
            temp = strdup("pen-might elamin");
            do_give(ch, temp, 0);
            if( temp ) {
                free( temp );
            }
        } else {
            sprintf(buf, "give %s %s", obj_name, vict_name);
            command_interpreter(ch, buf);
            if (obj->item_number == PEN_MIGHT) {
                test = 1;
            } else {
                return (TRUE);
            }
        }
    } else {
        return (FALSE);
    }

    if (test == 1) {
        act("$n says, 'Thank you so much.  Here is the sceptre.  Please be'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'careful.  I wouldn't want anything bad to happen to'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'you in your journeys.'", FALSE, vict, 0, 0, TO_ROOM);
        act("$n hands over the dragon sceptre.", FALSE, vict, 0, 0, TO_ROOM);

        obj = objectRead(DRAGON_SCEPTRE_ONE, VIRTUAL);
        objectGiveToChar(obj, ch);
        return (TRUE);
    }
    return (FALSE);
}

int fighter_mage(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    if (number(1, 100) > 49) {
        return (fighter(ch, cmd, arg, mob, type));
    } else {
        return (magic_user(ch, cmd, arg, mob, type));
    }
}

int FireBreather(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0009A massive cone of fire shoots forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);
        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_fire_breath(GetMaxLevel(ch), ch, tar_char, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int FrostBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0014A cone of blistering frost shoots forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_frost_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}


int GasBreather(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0002A cloud of poisonous gas billows forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_gas_breath(GetMaxLevel(ch), ch,
                                 ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int goblin_sentry(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *sentrymob;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            fighter(ch, cmd, arg, mob, type);
        }
    } else {
        sentrymob = FindMobInRoomWithFunction(ch->in_room, goblin_sentry);
        if (cmd >= 1 && cmd <= 6) {
            if (cmd == 3) {
                return (FALSE); /* can always go south */
            }
            /*
             * everything else gets ya attacked 
             */
            if (ch->desc && !IS_IMMORTAL(ch) && number(0, 1) && 
                GET_RACE(ch) != RACE_GOBLIN && CAN_SEE(sentrymob, ch)) {
                act("$N blocks your path and attacks!", FALSE, ch, 0,
                    sentrymob, TO_CHAR);
                act("$N growls at $n and attacks!", TRUE, ch, 0,
                    sentrymob, TO_ROOM);
                hit(sentrymob, ch, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

int goblin_chuirgeon(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    char           *vict_name;
    struct char_data *vict;
    int             test = 0,
                    tmp_con = 0;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != GOBLIN_CHUIRGEON) {
            return (FALSE);
        }
        if (IS_IMMORTAL(ch) && GetMaxLevel(ch) < MAX_IMMORT) {
            send_to_char("Get real.  You shouldn't be playing around with "
                         "him.\n\r", ch);
            return (TRUE);
        }

        act("$n says, 'Lo.  Yoo be lookin for abit o doctrin?  I offa my'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'ade fo a wee tiny gold pyle.  Yoo be stronga, I'",
            FALSE, vict, 0, 0, TO_ROOM);
        act("$n says, 'swayr.  Jus type 'buy' and I get right ta work.'",
            FALSE, vict, 0, 0, TO_ROOM);

        return (TRUE);
    } else if (cmd == 56) {
        /* Buy */
        if (IS_IMMORTAL(ch) && GetMaxLevel(ch) < MAX_IMMORT) {
            send_to_char("Get real.  You shouldn't be playing around with "
                         "him.\n\r", ch);
            return (TRUE);
        }

        test = GET_RCON(ch) + 1;
        test = test * 1000000;

        if (GET_GOLD(ch) < test) {
            act("$n says, 'Wat I look lyk?  Freebee?  Yoo need mo money.'",
                FALSE, mob, 0, 0, TO_ROOM);
        } else if (GET_EXP(ch) < 100000000) {
            act("$n says, 'Yoo need mo expuriense befo I can help yoo.'",
                FALSE, mob, 0, 0, TO_ROOM);
        } else if (GET_RCON(ch) > 17) {
            act("$n says, 'Yoo too strong fo mee to help.'", FALSE, mob, 0,
                0, TO_ROOM);
        } else {
            /* 
             * finally fixed this, used to use GET_CON, which isn't a hard 
             * value  -Lennya 
             */
            tmp_con = GET_RCON(ch);
            act("$n says, 'Vewy good.  I begin now.'", FALSE, mob, 0, 0,
                TO_ROOM);
            act("$n pushes you to the floor.", FALSE, mob, 0, 0, TO_ROOM);
            act("$n walks over to one of the cages and pulls a small bunny "
                "out.", FALSE, mob, 0, 0, TO_ROOM);
            act("$n slowly returns, gutting the bunny then quickly slicing "
                "your stomach open.", FALSE, mob, 0, 0, TO_ROOM);
            WAIT_STATE(ch, PULSE_VIOLENCE * 12);
            GET_POS(ch) = POSITION_STUNNED;
            GET_MANA(ch) = 1;
            GET_MOVE(ch) = 1;
            GET_HIT(ch) = 1;
            GET_RCON(ch) = (GET_RCON(ch) + 1);
            GET_GOLD(ch) -= test;
            GET_EXP(ch) -= ((GET_RCON(ch)) * 5000000);
            do_save(ch, NULL, 0);

            Log("Player %s just bought a CON point, %d to %d. Payed "
                "%d xps, %d gold.", GET_NAME(ch), tmp_con, GET_RCON(ch),
                GET_RCON(ch) * 5000000, test);
        }
        return (TRUE);
    } else {
        return (FALSE);
    }
    return (FALSE);
}

int hangman(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    if (cmd) {
        return 0;
    }
    switch (number(0, 15)) {
    case 0:
        act("$n whirls his noose like a lasso and it lands neatly around your "
            "neck.", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say You're next, you ugly rogue!");
        command_interpreter(ch, "say Just kidding.");
        act("$n pats you on your head.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 1:
        command_interpreter(ch, "say I was conceived in Exile and have been "
                                "integrated into society!");
        command_interpreter(ch, "say Muahaha!");
        break;
    case 2:
        command_interpreter(ch, "say Anyone have a butterknife I can borrow?");
        break;
    case 3:
        act("$n suddenly pulls a lever.", FALSE, ch, 0, 0, TO_ROOM);
        act("With the flash of light on metal a giant guillotine comes "
            "crashing down!", FALSE, ch, 0, 0, TO_ROOM);
        act("A head drops to the ground from the platform.", FALSE, ch, 0,
            0, TO_ROOM);
        act("$n looks up and shouts wildly.", FALSE, ch, 0, 0, TO_ROOM);
        act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 4:
        act("$n whistles a local tune.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}

int idiot(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    if (cmd) {
        return FALSE;
    }
    switch (number(0, 40)) {
    case 0:
        command_interpreter(ch, "say even if idiot = god");
        command_interpreter(ch, "say and Stu = idiot");
        command_interpreter(ch, "say Stu could still not = god.");
        act("$n smiles.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 1:
        act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,
            TO_ROOM);
        break;
    case 2:
        act("$n doesn't think you could stand up to him in a duel.", FALSE,
            ch, 0, 0, TO_ROOM);
        break;
    case 3:
        command_interpreter(ch, "say Rome really was built in a day.");
        act("$n snickers.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 4:
        act("$n flips over and walks around on his hands.", FALSE, ch, 0,
            0, TO_ROOM);
        break;
    case 5:
        act("$n cartwheels around the room.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 6:
        command_interpreter(ch, "say How many ogres does it take to screw in a"
                                " light bulb?");
        act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,
            TO_ROOM);
        break;
    case 7:
        command_interpreter(ch, "say Uh huh. Uh huh huh.");
        break;
    case 8:
        act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
        act("$n whistles quietly.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 9:
        act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 10:
        act("$n has a battle of wits with himself and comes out unharmed.",
            FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 11:
        command_interpreter(ch, "say All this and I am just a number.");
        act("$n cries on your shoulder.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 12:
        command_interpreter(ch, "say A certain hunchback I know dresses very "
                                "similar to you, very similar...");
        break;
    default:
        return FALSE;
    }
    return TRUE;
}

/*
 * Leaves of Silver (Xamael 05/02/02) 
 */
int Jessep(struct char_data *ch, int cmd, char *arg, struct char_data *mob)
{
    char           *obj_name,
                   *tgt_name,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *tgt;
    struct obj_data *obj;
    char           *temp;

    if (cmd == 72) {
        /* 
         * 72 = give command 
         */
        arg = get_argument(arg, &obj_name);
        /*
         * if object not in pc's inventory 
         */
        if (!obj_name) {
            send_to_char("You do not seem to have anything like that.\n\r", ch);
            return (FALSE);
        }

        /*
         * if object in inventory but not visible to pc 
         */
        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            send_to_char("You do not seem to have anything like that.\n\r", ch);
            return (TRUE);
        }

        /*
         * if receiver doesn't exist 
         */
        arg = get_argument(arg, &tgt_name);
        if (!tgt_name) {
            return (FALSE);
        }

        /*
         * if receiver exists but isn't visible to pc 
         */
        if (!(tgt = get_char_room_vis(ch, tgt_name))) {
            return (FALSE);
        }

        /*
         * if reciever is fighting 
         */
        if (tgt->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * if receiver is another pc 
         */
        if (IS_PC(tgt)) {
            return (FALSE);
        }
        /*
         * if receiver isn't Jessep 
         */
        if (mob_index[tgt->nr].vnum != 47951) {
            return (FALSE);
        }
        /*
         * if object IS NOT the head of Matron Singh 
         */
        if (obj != get_obj_in_list_vis(ch, "head of Matron Singh",
                                       ch->carrying)) {
            sprintf(buf, "give %s %s", obj_name, tgt_name);
            command_interpreter(ch, buf);
            sprintf(buf, "Why, thank you very much, that will make a nice "
                         "addition to my collection of rarities.");
            do_say(tgt, buf, 19);
            return (TRUE);
        } 
        
        /*
         * if object IS the head of Matron Singh 
         */
        sprintf(buf, "give %s %s", obj_name, tgt_name);
        command_interpreter(ch, buf);

        /*
         * if Jessep has the Laurel of Leaves, give it to the pc 
         */
        if (tgt->equipment[WEAR_HEAD]) {
            sprintf(buf, "Ah, she's dead, is she? Serves her right. "
                         "Though she's shown me that I'm not fit to lead "
                         "my people. Perhaps you would be a better "
                         "choice.");
            do_say(tgt, buf, 19);
            temp = KeywordsToString( &tgt->equipment[WEAR_HEAD]->keywords, u
                                     NULL );
            sprintf(buf, "remove %s", temp);
            free( temp );
            command_interpreter(tgt, buf);

            sprintf(buf, "give laurel-leaves-myrrhal %s", GET_NAME(ch));
            command_interpreter(tgt, buf);

            sprintf(buf, "I wish you well.");
            do_say(tgt, buf, 19);
            return (TRUE);
        } 
        
        /*
         * if he doesn't, then give no prize and keep the head anyway. 
         * that bastard! 
         */
        sprintf(buf, "Hrm, you killed her, eh? Well, I don't lead "
                     "this village anymore. Go find the current "
                     "leader, and leave me alone.");
        do_say(tgt, buf, 19);
        return (TRUE);
    }
    return (FALSE);
}

int lich_church(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    if (type == EVENT_DEATH) {
        /*
         * add spec procs here for when he dies 
         */

        return (FALSE);
    }

    if (number(0, 1)) {
        return (magic_user(ch, cmd, arg, mob, type));
    } else {
        return (shadow(ch, cmd, arg, mob, type));
    }
}

int LightBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0011A beam of bright yellow light shoots forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_light_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int LightningBreather(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0012A bolt of lightning streaks forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_lightning_breath(GetMaxLevel(ch), ch,
                                       ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int lizardman_shaman(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }

        switch (number(1, 4)) {
        case 1:
            return (cleric(mob, cmd, arg, mob, type));
            break;
        case 2:
            return (fighter(mob, cmd, arg, mob, type));
            break;
        case 3:
            command_interpreter(ch, "emote leers cruelly and says, 'You will "
                                    "die now!'");
            break;
        case 4:
            command_interpreter(ch, "emote hisses in anger, 'Leave our home "
                                    "now or die!'");
            break;
        }
    } else if (number(1, 4) == 4) {
        switch (number(1, 3)) {
        case 1:
            command_interpreter(ch, "emote waves his hands about uttering "
                                    "some slurred words.");
            break;
        case 2:
            command_interpreter(ch, "emote looks upwards and calls to some "
                                    "unseen god.");
            break;
        case 3:
            command_interpreter(ch, "emote grabs his chest and screams 'Death"
                                    " to all that oppose you!'");
            break;
        }
    }
    return( TRUE );
}

#define COMFY_ROBE 51840
int lust_sinner(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *i,
                   *next;
    struct affected_type af;
    struct obj_data *obj,
                   *obj2;
    int             r_num = 0;
    char            buf[128];

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_STANDING && GET_POS(ch) > POSITION_STUNNED) {
        StandUp(ch);
        return (TRUE);
    }

    if ((check_soundproof(ch)) || 
        (check_nomagic(ch, 0, 0)) || 
        (ch->specials.fighting) || 
        (ch->followers)) {
        return (FALSE);
    }
    /*
     * ch not fighting, let's look for a victim 
     */
    if (ch->in_room > -1) {
        /*
         * there's victims, let's see if we can harrass one 
         */
        for (i = real_roomp(ch->in_room)->people; i; i = next) {
            next = i->next_in_room;
            if (!IS_NPC(i) && !IS_LINKDEAD(i) && !IS_IMMORTAL(i)) {
                /* 
                 * victim connected mortal, wehee!  
                 */
                if (!IsImmune(i, IMM_CHARM)) {
                    if (!affected_by_spell(i, SPELL_CHARM_PERSON) && 
                        !affected_by_spell(i, SPELL_CHARM_MONSTER)) {
                        /*
                         * oh look, we can abuse this dude! 
                         */
                        if (IsResist(i, IMM_CHARM)) {
                            /* 
                             * resistant gets a save 
                             */
                            if (!saves_spell(i, SAVING_PARA)) {
                                act("$n winks slyly at you, and you instantly "
                                    "realize you should follow $m.", FALSE, ch,
                                    0, i, TO_VICT);
                                act("$n winks slyly at $N.", TRUE, ch, 0,
                                    i, TO_NOTVICT);
                                if (i->master) {
                                    stop_follower(i);
                                }
                                add_follower(i, ch);

                                af.type = SPELL_CHARM_PERSON;
                                af.duration = 24;
                                af.modifier = 0;
                                af.location = 0;
                                af.bitvector = AFF_CHARM;
                                affect_to_char(i, &af);

                                /*
                                 * Hmm, let's give victim a cute robe 
                                 */
                                act("$n hands you a comfortable looking robe.",
                                    FALSE, ch, 0, i, TO_VICT);
                                act("$n hands $N a nice comfortable robe.",
                                    FALSE, ch, 0, i, TO_NOTVICT);

                                obj = objectRead(COMFY_ROBE, VIRTUAL);
                                objectGiveToChar(obj, i);

                                /*
                                 * and let's make him remove his
                                 * bodywear.. 
                                 */
                                if (i->equipment[WEAR_BODY]) {
                                    obj2 = i->equipment[WEAR_BODY];
                                    if ((obj2 = unequip_char(i, WEAR_BODY))) {
                                        objectGiveToChar(obj2, i);
                                        act("Smiling happily, you stop wearing"
                                            " your current body armor.", FALSE,
                                            ch, 0, i, TO_VICT);
                                        act("Smiling happily, $n stops using $s"
                                            " body armor.", FALSE, i, 0, 0, 
                                            TO_NOTVICT);
                                    }
                                }

                                /*
                                 * and replace it by something more comfy 
                                 */
                                sprintf(buf, "wear comfortable-robe");
                                command_interpreter(i, buf);
                                return (TRUE);
                            } else {
                                /*
                                 * made his save, give him some notice 
                                 */
                                act("$n winks slyly at you, but you're not so "
                                    "easily fooled.", FALSE, ch, 0, i, TO_VICT);
                            }
                        } else {
                            /*
                             * Yum, not resistant, easy meat! 
                             */
                            act("$n winks slyly at you, and you instantly "
                                "realize you should follow $m.", FALSE, ch, 0,
                                i, TO_VICT);
                            act("$n winks slyly at $N.", TRUE, ch, 0, i,
                                TO_NOTVICT);

                            if (i->master) {
                                stop_follower(i);
                            }
                            add_follower(i, ch);

                            af.type = SPELL_CHARM_PERSON;
                            af.duration = 24;
                            af.modifier = 0;
                            af.location = 0;
                            af.bitvector = AFF_CHARM;
                            affect_to_char(i, &af);

                            /*
                             * Hmm, let's give victim a cute robe 
                             */
                            act("$n hands you a comfortable looking robe.",
                                FALSE, ch, 0, i, TO_VICT);
                            act("$n hands $N a nice comfortable robe.",
                                FALSE, ch, 0, i, TO_NOTVICT);

                            obj = objectRead(COMFY_ROBE, VIRTUAL);
                            objectGiveToChar(obj, i);

                            /*
                             * and let's make him remove his bodywear.. 
                             */
                            if (i->equipment[WEAR_BODY]) {
                                obj2 = i->equipment[WEAR_BODY];
                                if ((obj2 = unequip_char(i, WEAR_BODY))) {
                                    objectGiveToChar(obj2, i);
                                    act("Smiling happily, you stop wearing "
                                        "your current body armor.", FALSE, ch,
                                        0, i, TO_VICT);
                                    act("Smiling happily, $n stops using $s "
                                        "body armor.", FALSE, i, 0, 0, 
                                        TO_NOTVICT);
                                }
                            }
                            /*
                             * and replace it by something more comfy 
                             */
                            sprintf(buf, "wear comfortable-robe");
                            command_interpreter(i, buf);
                            return (TRUE);
                        }
                    }
                } else {
                    /*
                     * victim imm:charm, bummer, can't bug this one 
                     */
                    act("$n tries to lure you away, but you're immune to $s "
                        "charms.", FALSE, ch, 0, i, TO_VICT);
                }
            }
        }
    }
    return( FALSE );
}

int mad_cyrus(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        return (fighter(mob, cmd, arg, mob, type));
    }
    return (FALSE);
}

int mad_gertruda(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        return (fighter(mob, cmd, arg, mob, type));
    }
    return (FALSE);
}

int medusa(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *tar;
    int             i;

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    for (i = 0; i < 8; i++) {
        if ((tar = FindAHatee(ch)) == NULL) {
            tar = FindVictim(ch);
        }
        if (tar && (tar->in_room == ch->in_room)) {
            if (HitOrMiss(ch, tar, CalcThaco(ch))) {
                act("$n glares at $N with an evil eye!", 1, ch, 0, tar,
                    TO_NOTVICT);
                act("$n glares at you with and evil eye!", 1, ch, 0, tar,
                    TO_VICT);
                if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
                    cast_paralyze(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, tar, 0);
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

int mime_jerry(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *tmp_ch;
    struct obj_data *target_obj;
    char            buf[80];
    int             inroom,
                    r_num = FALSE;

    if (IS_PC(ch)) {
        return (0);
    }
    inroom = ch->in_room;

    if (type == EVENT_DEATH) {
        sprintf(buf, "nadia thunder mountain");
        if ((tmp_ch = get_char_vis_world(ch, buf, 0))) {
            target_obj = objectRead(NADIA_KEY, VIRTUAL);
            objectGiveToChar(target_obj, tmp_ch);

            target_obj = objectRead(NADIA_PILL, VIRTUAL);
            objectGiveToChar(target_obj, tmp_ch);
        }
    }

    switch (number(0, 60)) {
    case 0:
        command_interpreter(ch, "emote turns himself into a box.");
        break;
    case 1:
        command_interpreter(ch, "emote starts to climb an invisible rope.");
        break;
    case 2:
        command_interpreter(ch, "emote is stuck inside a box that you can't "
                                "see. Weird!");
        break;
    case 3:
        command_interpreter(ch, "emote is crawling around on the floor like a "
                                "dog.");
        break;
    case 4:
        command_interpreter(ch, "emote is lying on the floor playing dead.");
        break;
    case 5:
        command_interpreter(ch, "emote pretends like he's riding a big "
                                "stallion.");
        break;
    case 6:
        command_interpreter(ch, "emote is stuck under an enormous rock....or "
                                "so he thinks.");
        break;
    case 7:
        command_interpreter(ch, "emote mimics your every move.");
        break;
    case 8:
        command_interpreter(ch, "emote begins to eat some imaginary cheese.");
        break;
    case 9:
        command_interpreter(ch, "emote sniffs your behind.");
        break;
    case 10:
        command_interpreter(ch, "emote would like a kiss from you.");
        break;
    case 11:
        command_interpreter(ch, "emote flexes his muscles and pretends like "
                                "he's the great Lord Ignatius.");
        break;
    case 12:
        command_interpreter(ch, "emote sticks his nose in your crotch and "
                                "won't go away.");
        break;
    case 13:
        command_interpreter(ch, "emote rolls up in a tiny ball for you to "
                                "kick him.");
        break;
    case 14:
        command_interpreter(ch, "emote takes a black magic marker and begins "
                                "to give you a cute mustache.");
        break;
    case 15:
        command_interpreter(ch, "emote worships Abagor as his patron saint of "
                                "stupidity.");
        break;
    case 16:
        command_interpreter(ch, "emote wishes he could be just like you.");
        break;
    case 17:
        command_interpreter(ch, "emote pees his pants every time that meanie "
                                "Tsaron shows up.");
        break;
    case 18:
        command_interpreter(ch, "emote begins falling and falling and falling "
                                "and falling and....");
        break;
    case 19:
        command_interpreter(ch, "emote is head over heels in love....no, he's "
                                "just head over heels.");
        break;
    case 20:
        command_interpreter(ch, "emote moshes wildly with you and tries to "
                                "steal your wallet.");
        break;
    case 21:
        command_interpreter(ch, "emote begs for a couple of cents with those "
                                "poor puppy dog eyes.");
        break;
    case 22:
        command_interpreter(ch, "emote is sitting with no chair...Amazing!");
        break;
    case 23:
        command_interpreter(ch, "emote shows you his wallet and the pictures "
                                "of all his kids.");
        break;
    case 24:
        command_interpreter(ch, "emote would like it if you would give him a "
                                "massage.");
        break;
    case 25:
        command_interpreter(ch, "emote would shave his butt and learn to walk "
                                "backwards if he looked like you!");
        break;
    case 26:
        command_interpreter(ch, "emote is gradually getting squished tighter "
                                "and tighter into a magical force field!");
        break;
    case 27:
        command_interpreter(ch, "emote is climbing a ladder, but going "
                                "nowhere.");
        break;
    case 28:
        command_interpreter(ch, "emote writhes on the floor doing strange "
                                "undulating motions.");
        break;
    case 29:
        command_interpreter(ch, "emote begins to do laps around the room on "
                                "his hands.");
        break;
    case 30:
        command_interpreter(ch, "emote sits down at his computer and starts "
                                "to MUD.");
        break;
    default:
        return (FALSE);
    }
    return (TRUE);
}

/*
 * Nadia 
 */
int nadia(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
          int type)
{
    char           *vict_name,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    int             has_key = 0;
    int             has_pill = 0;
    int             keyrnum = 0;
    int             pillrnum = 0;
    struct obj_data *i;

    if (!AWAKE(ch)) {
        return (FALSE);
    }

    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != NADIA) {
            return (FALSE);
        }

        for (i = vict->carrying; i && !has_key; i = i->next_content) {
            if (i->item_number == NADIA_KEY) {
                has_key = 1;
            }
        }

        /*
         * Quest Text
         */
        if (has_key == 0) {
            for (i = vict->carrying; i && !has_pill; i = i->next_content) {
                if (i->item_number == NADIA_PILL) {
                    has_pill = 1;
                }
            }

            if (has_pill == 0) {
                act("$n says, 'I don't know anything about a sceptre or any "
                    "dragons, but I'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'do know who might be able to help you.  Elamin,"
                    " a great magician'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'used to work for the king is imprisoned in the "
                    "keep tower.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'If you do me a favor and kill that dastardly "
                    "Mime Jerry, I will'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'give you the key to Elamin's prison.  That fool"
                    " king will learn'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'to pay the consequences when he doesn't come to"
                    " see me for weeks!'", FALSE, vict, 0, 0, TO_ROOM);

                if (vict->equipment[HOLD]) {
                    temp = KeywordsToString( &vict->equipment[HOLD]->keywords,
                                             NULL );
                    sprintf(buf, "remove %s", temp);
                    free( temp );
                    command_interpreter(vict, buf);
                    sprintf(buf, "give green-key %s", GET_NAME(ch));
                    command_interpreter(vict, buf);
                    act("$n says, 'Here is the key to the west wing.  You'll "
                        "need it.'", FALSE, vict, 0, 0, TO_ROOM);
                    return (TRUE);
                }
            }
        } else if (has_key == 1) {
            act("$n says, 'Here is the key.  I will tell you no more.  Go, "
                "before I change my mind.'", FALSE, vict, 0, 0, TO_ROOM);
            sprintf(buf, "give key-elamin %s", GET_NAME(ch));
            command_interpreter(vict, buf);
            return (TRUE);
        }
    }
    return (FALSE);
}


int pridemirror(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    char           *buf;
    struct char_data *portal;

    if (cmd != 15) {
        /* look */
        return (FALSE);
    }

    arg = get_argument(arg, &buf);

    if (buf && !strcasecmp("mirror", buf) && 
        (portal = get_char_room("mirror", ch->in_room))) {

        send_to_char("\n\r", ch);
        send_to_char("Ah yes, this is how you look best, after all. Who needs"
                     " clothes\n\r", ch);
        send_to_char("with a divine body like yours, anyway? It is time to "
                     "show that\n\r", ch);
        send_to_char("Guardian of Pride who looks best around here.\n\r", ch);
        send_to_char("\n\r", ch);

        act("$n looks into the Mirror of Pride, smiles, and steps inside it.",
            FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, 51835);
        act("$n steps out of one of the mirroring surfaces in the room, "
            "smiling.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}

/**
 * @bug (GH)2001 Still under Construction...
 */
int QPSalesman(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct obj_data *obj;
    char            buf[256];
    int             x = 0,
                    temp = 0;
    char            mobname[128];
    static int      questitems[14][3] = {
        {871, CLASS_CLERIC, 1},
        {872, CLASS_MAGIC_USER, 1},
        {873, CLASS_DRUID, 1},
        {874, CLASS_WARRIOR, 1},
        {875, CLASS_RANGER, 1},
        {876, CLASS_CLERIC, 1},
        {877, CLASS_MAGIC_USER, 1},
        {878, CLASS_DRUID, 1},
        {879, CLASS_WARRIOR, 1},
        {-1, -1, -1}
    };
    Keywords_t     *key;

    sprintf(mobname, "%s", GET_NAME(mob));

    if (cmd == 59) { 
        /* 
         * list
         */
        oldSendOutput(ch, "Quest Point Items:\n\r$c0011%-25s  %-20s\n\r",
                  "Name", "QPs");
        oldSendOutput(ch, "%-25s  %-20s\n\r", "----", "---");

        while (questitems[x][0] != -1) {
            /* 
             * loop through all the items in the shop..  lets print all of 
             * them.
             */
            obj = objectRead(questitems[x][0], VIRTUAL);
            if (obj) {
                oldSendOutput(ch, "$c0012%-25s  $c0015%-20d$c0007\n\r",
                          obj->short_description, questitems[x][2]);
            }
            x++;
        }
        oldSendOutput(ch, "\n\r Available commands 'List', 'ID <ITEM>', 'Buy "
                      "<ITEM>'\n\r");
        return (TRUE);
    } 
    
    if (cmd == 26) {
        /* 
         * lets ID that mofo 
         */
        key = StringToKeywords( arg, NULL );
        while (questitems[x][0] != -1) {
            /* 
             * loop through all the items in the shop..  
             * lets print all of them.
             */
            obj = objectRead(questitems[x][0], VIRTUAL);
            if (obj) {
                if (KeywordsMatch(key, &obj->keywords)) {
                    FreeKeywords(key, TRUE);
                    spell_identify(50, ch, 0, obj);
                    return (TRUE);
                }
            }
            x++;
        }
    }
    
    if (cmd == 56) {
        /* 
         * buy? 
         */
        temp = 13;              
        /*
         * lets search for item..
         */
        while (questitems[x][0] != -1) {
            /* 
             * loop through all the items in the shop.. see 
             * if someone wants to buy one.
             */
            if ((obj = objectRead(questitems[x][0], VIRTUAL)) &&
                KeywordsMatch(key, &obj->keywords)) {
                temp = questitems[x][2];

                if (temp <= ch->player.q_points) {
                    ch->player.q_points = ch->player.q_points - temp;
                    oldSendOutput(ch, "$c0013[$c0015%s$c0013] tells you 'I hope "
                                  "you enjoy.'\n\r", mobname);
                    oldSendOutput(ch, "%s gives you %s\n\r", mobname,
                              obj->short_description);
                    objectGiveToChar(obj, ch);
                    sprintf(buf, "bought %s", obj->short_description);
                    qlog(ch, buf);
                } else {
                    oldSendOutput(ch, "$c0013[$c0015%s$c0013] tells you 'You don't"
                                  " have enought QPoints for that item'\n\r",
                              mobname);
                }
                FreeKeywords(key, TRUE);
                return( TRUE );
            }
            x++;
        }
        oldSendOutput(ch, "$c0013[$c0015%s$c0013] tells you 'I don't have that "
                      "item.'\n\r", mobname);
        FreeKeywords(key, TRUE);
        return (TRUE);
    }

    FreeKeywords(key, TRUE);
    return( FALSE );
}

int QuestMobProc(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];

    struct char_data *vict;
    struct obj_data *obj;
    struct obj_data *obj2;
    int             universal;
    int             xps;
    float           align;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (cmd == 72) {
        /* give
         *
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name) {
            return (FALSE);
        }
        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            return (FALSE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name) {
            return (FALSE);
        }

        if (!(vict = get_char_room_vis(ch, vict_name))) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (IS_PC(vict)) {
            return (FALSE);
        }
        universal = mob_index[vict->nr].vnum;

        if (obj->item_number != universal) {
            if (!strcmp(vict->specials.quest_no, "")) {
                if (IS_GOOD(vict)) {
                    sprintf(buf, "tell %s Oh.. You must be mistaken, I'm not "
                                 "looking for that object.", GET_NAME(ch));
                } else {
                    sprintf(buf, "tell %s Blah.. Quit wasting my time with "
                                 "this garbage.", GET_NAME(ch));
                }
                command_interpreter(vict, buf);
            } else {
                send_to_room(vict->specials.quest_no, ch->in_room);
            }
            return (TRUE);
        } else {
            if ((obj2 = objectRead(universal + 1, VIRTUAL))) {
                act("You give $p to $N.", FALSE, ch, obj, vict, TO_CHAR);
                act("$n gives $p to $N.", FALSE, ch, obj, vict, TO_ROOM);

                if (!strcmp(vict->specials.quest_yes, "")) {
                    if (IS_GOOD(vict)) {
                        do_say(vict, "Thank you very much.  Here is something"
                                     " for your troubles.", 0);
                    } else {
                        do_say(vict, "Thanks.. Here ya go.. Now get lost will "
                                     "ya.", 0);
                    }
                } else {
                    send_to_room(vict->specials.quest_yes, ch->in_room);
                }

                act("$N gives you $p.", FALSE, ch, obj2, vict, TO_CHAR);
                act("$N gives $p to $n.", FALSE, ch, obj2, vict, TO_ROOM);
                objectGiveToChar(obj2, ch);
                objectTakeFromChar(obj);
                objectExtract(obj);
                return (TRUE);
            } else {
                /* 
                 * there's no reward. give quester xps + alignshift 
                 */
                if (!strcmp(vict->specials.quest_yes, "")) {
                    if (IS_GOOD(vict)) {
                        do_say(vict, "Thank you very much!", 0);
                    } else {
                        do_say(vict, "Thanks mate.", 0);
                    }
                } else {
                    send_to_room(vict->specials.quest_yes, ch->in_room);
                }
                act("$n puts $p into $s pocket.", TRUE, vict, obj, 0, TO_ROOM);
                objectTakeFromChar(obj);
                objectExtract(obj);

                /* 
                 * gain xps if there's no obj to be had 
                 */
                xps = GetMaxLevel(vict) * 1000; 
                /* 
                 * align shifts too 
                 */
                align = (float) GET_ALIGNMENT(vict) / 20;
                GET_ALIGNMENT(ch) += (int) align;
                if (align > 0) {
                    send_to_char("Your alignment just shifted towards "
                                 "good.\n\r", ch);
                    if (GET_ALIGNMENT(ch) > 1000) {
                        GET_ALIGNMENT(ch) = 1000;
                    }
                } else if (align < 0) {
                    send_to_char("Your alignment just shifted towards "
                                 "evil.\n\r", ch);
                    if (GET_ALIGNMENT(ch) < -1000) {
                        GET_ALIGNMENT(ch) = -1000;
                    }
                }
                if (xps > 0) {
                    sprintf(buf, "$c000BYou receive $c000W%d $c000Bexperience "
                                 "for completing this quest.\n\r", xps);
                    send_to_char(buf, ch);
                }
                return (TRUE);
            }
        }
        return (FALSE);
    }
    return (FALSE);
}


char            questwinner[50];
int             questwon = 0;

int             questNumber2 = -1;
int             questNumber = -1;
/*
 * Questor GOD 
 */
int QuestorGOD(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];

    struct char_data *vict = NULL;
    struct obj_data *obj;
    struct obj_data *obj2;
    int             itemgranted = 0;
    static time_t   time_diff = 50;
    static time_t   last_time = 0;

    if (questNumber == -1) {
        questNumber = number(0, 49);
        questNumber2 = number(0, 3);
    }

    if (cmd == 15) {
        /*
         * look
         */
        if (arg && !(strcmp(lower(arg), "ball"))) {
            send_to_char("You look deep into the crystal ball..\n\r\n\r", ch);
            if ((obj = find_tqp(1))) {
                send_to_char("A slight glint, deep inside the crystal tells "
                             "you that there's booty to be had.\n\r", ch);
            } else {
                send_to_char(".. and see nothing at all.\n\r", ch);
            }
            return (TRUE);
        }
        return (FALSE);
    }

    /*
     * Ask what the quest is?
     */
    if (cmd == 86) {
        /* 
         * ask 
         */
        arg = get_argument(arg, &vict_name);
        if (!arg || !vict_name || !(vict = get_char_room_vis(ch, vict_name))) {
            return(FALSE);
        }

        if (!(strcasecmp(arg, "what is the quest?"))) {
            send_to_char("$c0006You ask him 'What is the quest?'\n\r", ch);
            do_say(vict, "Solve my quest and you shall be rewarded!", 0);
            do_say(vict, QuestList[questNumber2][questNumber].where, 0);
            return (TRUE);
        }
        return (FALSE);
    }

    if (!cmd) {
        if ((time_diff > 150)) {
            time_diff = 0;
            last_time = time(NULL);
            do_say(ch, "Solve my quest and you shall be rewarded!", 0);
            do_say(ch, QuestList[questNumber2][questNumber].where, 0);

            return (TRUE);
        } else {
            time_diff = time(NULL) - last_time;
        }
        return (FALSE);
    }

    if (!AWAKE(ch))
        return (FALSE);

    if (cmd == 72) {
        /* 
         * give 
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name) {
            send_to_char("Give what?\n\r", ch);
            return (FALSE);
        }

        if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            send_to_char("Give what?\n\r", ch);
            return (TRUE);
        }

        arg = get_argument(arg, &vict_name);
        if (!vict_name) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (!(vict = get_char_room_vis(ch, vict_name))) {
            send_to_char("To who?\n\r", ch);
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (IS_PC(vict)) {
            return (FALSE);
        }
        if (GetMaxLevel(ch) < 99) {
            if (obj->item_number != QuestList[questNumber2][questNumber].item) {
                sprintf(buf, "tell %s That is not the item I seek.", 
                        GET_NAME(ch));
                command_interpreter(vict, buf);
            } else {
                ch->specials.questwon = ch->specials.questwon + 1;
                switch (number(0, 20 + ch->specials.questwon)) {
                case 0:
                case 1:
                case 2:
                    do_say(vict, "Thanks-you!!  Just what i needed!! Here ya "
                                 "go", 0);
                    /* 
                     * grant Heal potion 
                     */
                    itemgranted = 9611;
                    break;
                case 3:
                case 4:
                    do_say(vict, "Thanks!! Little mana potion for your "
                                 "troubles.", 0);
                    /* 
                     * blistering red mana 
                     */
                    itemgranted = 15811;
                    break;
                case 5:
                case 6:
                    do_say(vict, "Thanks-you!!  Just what i needed!! Here "
                                 "ya go", 0);
                    /* 
                     * Purple potion 
                     */
                    itemgranted = 4050;
                    break;
                case 7:
                case 8:
                    do_say(vict, "Thanks Again! My skin is already rock hard.."
                                 " You take it", 0);
                    /* 
                     * Stone skin 
                     */
                    itemgranted = 3417;
                    break;
                case 9:
                case 10:
                    do_say(vict, "Thanks-you!!  Just what i needed!! Here ya "
                                 "go", 0);
                    /* 
                     * tiny copper ring 
                     */
                    itemgranted = 41277;
                    break;
                case 11:
                case 12:
                    do_say(vict, "Dankes!! I should be giving this to Heximal,"
                                 " he needs it bad.", 0);
                    /* 
                     * 1k coins 
                     */
                    itemgranted = 40124;
                    break;
                case 13:
                case 14:
                case 15:
                    do_say(vict, "Thanks-you!!  Just what i needed!! Here ya"
                                 " go", 0);
                    /* 
                     * 2.5k coins 
                     */
                    itemgranted = 40125;
                    break;
                case 16:
                case 17:
                    do_say(vict, "Thanks-you!!  Just what i needed!! Here ya "
                                 "go", 0);
                    /* 
                     * Special copper bracers 
                     */
                    itemgranted = 41273;
                    break;
                case 18:
                case 19:
                    do_say(vict, "Thanks again!  Just pocket change to me!! Go"
                                 " buy a beer will ya", 0);
                    /* 
                     * 5k coins 
                     */
                    itemgranted = 40126;
                    break;
                case 20:
                case 21:
                    do_say(vict, "Thanks Again! Just pocket change to me!! Go "
                                 "buy a beer will ya", 0);
                    /* 
                     * 10k coins 
                     */
                    itemgranted = 40127;
                    break;
                case 22:
                case 23:
                    do_say(vict, "Thanks!  Go buy your mom something nice.", 0);
                    /* 
                     * 20k coins 
                     */
                    itemgranted = 40128;
                    break;
                case 24:
                    do_say(vict, "Thanks....Ahh.. it was worth it!! Here ya go",
                           0);
                    /* 
                     * 30k coins 
                     */
                    itemgranted = 40129;
                    break;
                case 25:
                    do_say(vict, "Your good!!  Ya like to be different don't "
                                 "you!! Here ya go", 0);
                    /* 
                     * restring token 
                     */
                    itemgranted = 1003;
                    break;
                case 26:
                    do_say(vict, "Don't you think your special!! Here ya go",
                            0);
                    /* 
                     * questpoint token 
                     */
                    itemgranted = 1002;
                    break;
                default:
                    do_say(vict, "Thanks!!  Damn!! You are good!!", 0);
                    /* 
                     * sanc potion 
                     */
                    itemgranted = 1002;
                    break;
                }
                obj2 = objectRead(itemgranted, VIRTUAL);

                if (!obj2) {
                    Log("Error in questor mob.. Please report or make "
                        "the items.");
                    do_say(vict, "Blah.. Guess i don't have that.. Take this "
                                 "instead.", 0);
                    itemgranted = 4050;

                    obj2 = objectRead(itemgranted, VIRTUAL);
                    if (!obj2) {
                        return (FALSE);
                    }
                }

                if (itemgranted == 1002 || itemgranted == 1003 || 
                    itemgranted == 1004) {
                    ch->specials.questwon = 0;
                    sprintf(buf, "won a token.. Received item %s", 
                            obj2->short_description);
                    qlog(ch, buf);
                }
                objectGiveToChar(obj2, ch);

                act("$N gives you $p.", TRUE, ch, obj2, vict, TO_CHAR);
                act("$N gives $p to $n.", TRUE, ch, obj2, vict, TO_ROOM);

                do_say(vict, "Hey.. I got something else for you to get too.",
                       0);
                act("$N waves $s hands and makes something disappear.",
                    TRUE, ch, obj, vict, TO_ROOM);
                act("$N waves $s hands and makes something disappear.",
                    TRUE, ch, obj, vict, TO_CHAR);
                objectTakeFromChar(obj);
                objectExtract(obj);
                /*
                 * pick new quest 
                 */
                questNumber = number(0, 49);
                questNumber2 = number(0, 3);
                time_diff = 201;
            }
            return (TRUE);
        }
    }
    return (FALSE);
}

#define IRON_DIR_STOP  EAST
#define IRON_ROOM_STOP 30078

int raven_iron_golem(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    struct char_data *v;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (cmd == IRON_DIR_STOP && ch->in_room == IRON_ROOM_STOP) {
        return (CheckForBlockedMove(ch, cmd, arg, IRON_ROOM_STOP, 
                                    IRON_DIR_STOP, 0));
    }

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {
        if (number(0, 1)) {
            act("$n belows out a gout of green gas!", TRUE, ch, 0, 0, TO_ROOM);
            for (v = real_roomp(ch->in_room)->people; v;
                 v = v->next_in_room) {
                if (ch != v && !IS_IMMORTAL(v)) {
                    act("The green gas fills your lungs!.", FALSE, v, 0, 0,
                        TO_CHAR);
                    cast_sleep(GetMaxLevel(ch), v, "", SPELL_TYPE_SPELL, v, 0);
                }
            }
            return (TRUE);
        } else {
            return (fighter(mob, cmd, arg, mob, type));
        }
    }

    return (FALSE);
}

int ShardBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0015A cone of glowing shards bursts forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_shard_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int silktrader(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct room_data *rp;

    if (cmd) {
        return 0;
    }
    rp = real_roomp(ch->in_room);

    if (rp->sector_type == SECT_CITY) {
        switch (number(0, 30)) {
        case 0:
            act("$n eyes a passing woman.", FALSE, ch, 0, 0, TO_ROOM);
            command_interpreter(ch, "say Come, m'lady, and have a look at this"
                                    " precious silk!");
            break;
        case 1:
            act("$n says to you, 'Wouldn't you look lovely in this!'",
                FALSE, ch, 0, 0, TO_ROOM);
            act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,
                TO_ROOM);
            break;
        case 2:
            act("$n holds a pair of silk gloves up for you to inspect.",
                FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 3:
            act("$n cries out, 'Have at this fine silk from exotic corners of"
                " the world you will likely never see!", FALSE, ch, 0, 0, 
                TO_ROOM);
            act("$n smirks.", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 4:
            command_interpreter(ch, "say Step forward, my pretty locals!");
            break;
        case 5:
            act("$n shades his eyes with his hand.", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 6:
            command_interpreter(ch, "say Have you ever seen an ogre in a "
                                    "silken gown?");
            command_interpreter(ch, "say I didn't *think* so!");
            act("$n throws his head back and cackles with insane glee!",
                FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 7:
            act("$n hands you a glass of wine.", FALSE, ch, 0, 0, TO_ROOM);
            command_interpreter(ch, "say Come, have a seat and view my wares.");
            break;
        case 8:
            act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
            act("$n shakes his head sadly.", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 9:
            act("$n fiddles with some maps.", FALSE, ch, 0, 0, TO_ROOM);
            break;
        case 10:
            command_interpreter(ch, "say Here here! Beggars and nobles alike "
                                    "come forward and make your bids!");
            break;
        case 11:
            command_interpreter(ch, "say I am in this bourgeois hamlet for a "
                                    "limited time only!");
            act("$n swirls some wine in a glass.", FALSE, ch, 0, 0, TO_ROOM);
            break;
        default:
            return (FALSE);
            break;
        }
        return (TRUE);
    }
    
    switch (number(0, 20)) {
    case 0:
        command_interpreter(ch, "say Ah! Fellow travellers! Come have a look "
                                "at the finest silk this side of the infamous "
                                "Ched Razimtheth!");
        return (FALSE);
    case 1:
        act("$n looks at you.", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say You are feebly attired for the danger "
                                "that lies ahead.");
        command_interpreter(ch, "say Silk is the way to go.");
        act("$n smiles warmly.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    case 2:
        command_interpreter(ch, "say Worthy adventurers, hear my call!");
        return (FALSE);
    case 3:
        act("$n adjusts his cloak.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    case 4:
        act("$n says to you, 'Certain doom awaits you, therefore shall you die"
            " in silk.'", FALSE, ch, 0, 0, TO_ROOM);
        act("$n bows respectfully.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    case 5:
        command_interpreter(ch, "say Can you direct me to the nearest tavern?");
        return (FALSE);
    case 6:
        command_interpreter(ch, "say Heard the latest ogre joke?");
        act("$n snickers to himself.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    case 7:
        command_interpreter(ch, "say What ho, traveller! Rest your legs here "
                                "for a spell and peruse the latest in "
                                "fashion!");
        return (FALSE);
    case 8:
        command_interpreter(ch, "say Beware ye, traveller, lest ye come to "
                                "live in Exile!");
        act("$n grins evilly.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    case 9:
        act("$n touches your shoulder.", FALSE, ch, 0, 0, TO_ROOM);
        command_interpreter(ch, "say A word of advice. Beware of any ale "
                                "labeled 'mushroom' or 'pumpkin'.");
        act("$n shivers uncomfortably.", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    default:
        return (FALSE);
        break;
    }

    return (TRUE);
}

 /*
  * Lennyas proc to portal somewhere 
  */
int sinpool(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    char           *buf;
    struct char_data *portal;

    if (cmd != 7) {
        /* enter */
        return (FALSE);
    }

    arg = get_argument(arg, &buf);

    if (buf && (!strcasecmp("portal", buf) || !strcasecmp("gate", buf) ||
                 !strcasecmp("gate portal", buf)) && 
        (portal = get_char_room("portal", ch->in_room))) {

        send_to_char("\n\r", ch);
        send_to_char("Having seen enough of this path, you enter the "
                     "Gate of Sin.\n\r", ch);
        send_to_char("\n\r", ch);

        act("$n enters the Gate of Sin and disappears from your "
            "vicinity.", FALSE, ch, 0, 0, TO_ROOM);
        char_from_room(ch);
        char_to_room(ch, 51804);

        act("Proven a true sinner, $n materializes from one of the "
            "altar's seven sides.", FALSE, ch, 0, 0, TO_ROOM);
        do_look(ch, NULL, 0);
        return (TRUE);
    }
    return (FALSE);
}


int sin_spawner(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *mobtmp;

    if (type == EVENT_DEATH) {
        mobtmp = read_mobile(real_mobile(51803), REAL);
        char_to_room(mobtmp, 51817);
        act("$n gasps his last breath, but the supply of sinners seems no "
            "less.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    }

    if (cmd == 329) {
        /* berserk */
        if (ch->specials.fighting) {

            send_to_char("\n\r", ch);
            send_to_char("$c0012The Rage overtakes you quickly!$c0007\n\r", ch);
            send_to_char("\n\r", ch);

            act("$c0012$n seems to have found the rage, and is whisked away "
                "from the fight.$c0007", FALSE, ch, 0, 0, TO_ROOM);
            char_from_room(ch);
            char_to_room(ch, 51823);

            act("$n madly rages about, then calms down when there's no more "
                "quarries about.", FALSE, ch, 0, 0, TO_ROOM);
            do_look(ch, NULL, 0);

            send_to_char("You rage about, then realize there's noone to fight "
                         "and calm down.\n\r", ch);
            return (TRUE);
        } else {
            return (FALSE);
        }
    } else {
        return (FALSE);
    }

    if (!AWAKE(mob)) {
        return (FALSE);
    }
    return (magic_user(mob, cmd, arg, mob, type));
}


#define SLAV_DEAD_DONE_ROOM     3494    /* where to tel people when he is
                                         * killed */
#define SLAV_LIVE_ROOM          3496    /* where he SHOULD live */
int Slavalis(struct char_data *ch, int cmd, char *arg,
             struct char_data *mob, int type)
{
    struct char_data *v;

    if (cmd || !AWAKE(mob)) {
        return (FALSE);
    }
    if (type == EVENT_DEATH && ch->in_room == SLAV_LIVE_ROOM) {
        act("$n screams at you horridly 'I will return again!'", FALSE, ch,
            0, 0, TO_ROOM);
        /*
         * move all pc's to different room 
         */
        for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
            if (v != ch && !IS_IMMORTAL(v)) {
                act("You get a quezzy feeling as you fall into a swirling "
                    "mist.\nYou arrive back on your home plane!", FALSE, v, 0,
                    ch, TO_CHAR);
                act("$n fades away blinking in curiousity.", FALSE, v, 0,
                    ch, TO_ROOM);
                char_from_room(v);
                char_to_room(v, SLAV_DEAD_DONE_ROOM);
                do_look(v, NULL, 0);
            }
        }

        send_to_all("The chruch bells of Karsynia sound off in a chorus of "
                    "happiness!\n");

        return (TRUE);
    }

    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }

        switch (number(1, 2)) {
        case 1:
            return (Demon(ch, cmd, arg, mob, type));
            break;
        case 2:
            return (Psionist(ch, cmd, arg, mob, type));
            break;
        default:
            return (Demon(ch, cmd, arg, mob, type));
            break;
        }
    } else {
        return (magic_user(ch, cmd, arg, mob, type));
    }
}

int SleepBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0003A cloud of sleeping gas billows forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_sleep_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int snake_avt(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    byte            lspell = 0;
    struct char_data *vict;

    if (type == EVENT_DEATH && ch->in_room == RESCUE_ROOM) {
        mob = get_char_vis(ch, "zifnab");
        cast_portal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, mob, 0);
        mob = read_mobile(real_mobile(RESCUE_VIRGIN), REAL);
        char_to_room(mob, RESCUE_ROOM);

        act("After defeating the avatar of the snake god a mystical door "
            "appears.", FALSE, ch, 0, 0, TO_ROOM);
        act("You quickly release the prisoner strapped to the altar and "
            "decide it", FALSE, ch, 0, 0, TO_ROOM);
        act("would be best to quickly enter the portal and leave this horrid "
            "place.", FALSE, ch, 0, 0, TO_ROOM);
        act("The prisoner says 'Yes, enter the portal to safety, but first "
            "unlock", FALSE, ch, 0, 0, TO_ROOM);
        act("the altar and take the spoils due your party.'",
            FALSE, ch, 0, 0, TO_ROOM);
        act("You listen to her and decide to QUICKLY do so before the portal "
            "disappears.", FALSE, ch, 0, 0, TO_ROOM);
        act("\n", FALSE, ch, 0, 0, TO_ROOM);
        return (FALSE);
    }

    if ((!AWAKE(ch)) || (cmd)) {
        return (FALSE);
    }
    if (ch->specials.fighting &&
        ch->specials.fighting->in_room == ch->in_room) {

        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }

        vict = FindVictim(ch);
        if (!vict) {
            vict = ch->specials.fighting;
        }
        if (!vict) {
            return (FALSE);
        }
        lspell = number(0, GetMaxLevel(ch));
        if (!IS_PC(ch)) {
            lspell += GetMaxLevel(ch) / 5;
        }
        lspell = MIN(GetMaxLevel(ch), lspell);

        if (lspell < 1) {
            lspell = 1;
        }
        if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
            act("$n utters the words 'Let me see the light!'",
                TRUE, ch, 0, 0, TO_ROOM);
            cast_cure_blind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (FALSE);
        }

        if (GET_MOVE(ch) < 0) {
            act("$n pulls a vial of human blood out of thin air.  How "
                "refreshing.", 1, ch, 0, 0, TO_ROOM);
            cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (FALSE);
        }

        if (!IS_AFFECTED(ch, AFF_FIRESHIELD) && lspell > 39) {
            act("$n utters the words 'zimja'", 1, ch, 0, 0, TO_ROOM);
            cast_fireshield(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (FALSE);
        }

        if (!IS_AFFECTED(ch, AFF_SANCTUARY) && lspell > 25) {
            act("$n utters the words 'zija'", 1, ch, 0, 0, TO_ROOM);
            cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
            return (FALSE);
        }

        if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 25 && 
            GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'paf zija'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            return (FALSE);
        }

        if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 25 && 
            GetMaxLevel(ch) >= GetMaxLevel(vict)) {
            act("$n utters the words 'paf zimja'", 1, ch, 0, 0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            return (FALSE);
        }

        if (lspell > 40) {
            act("$n utters the words 'zerumba'", 1, ch, 0, 0, TO_ROOM);
            cast_feeblemind(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
            return (FALSE);
        }

        /*
         * well, spells failed or not needed, let's kick someone :) 
         */

        switch (number(0, 3)) {
        case 1:
            return (magic_user(mob, cmd, arg, mob, type));
            break;
        case 2:
            return (cleric(mob, cmd, arg, mob, type));
            break;
        case 3:
            return (snake(mob, cmd, arg, mob, type));
            break;
        default:
            return (fighter(mob, cmd, arg, mob, type));
            break;
        }
    }
    return (magic_user(mob, cmd, arg, mob, type));
}

int snake_guardians(struct char_data *ch, int cmd, char *arg,
                    struct char_data *mob, int type)
{
    struct char_data *snakemob;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd) {
        if (ch->specials.fighting) {
            snake(ch, cmd, arg, mob, type);
        }
    } else {
        snakemob = FindMobInRoomWithFunction(ch->in_room, snake_guardians);
        if (cmd >= 1 && cmd <= 6) {
            if (cmd == 3) {
                /* can always go south */
                return (FALSE);
            }

            /*
             * everything else gets ya attacked 
             */
            if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) && 
                !IS_IMMORTAL(ch)) {
                act("$N blocks your path and attacks!", FALSE, ch, 0,
                    snakemob, TO_CHAR);
                act("$N hisses at $n and attacks!", TRUE, ch, 0, snakemob,
                    TO_ROOM);
                hit(snakemob, ch, 0);
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

int SoundBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0013A sonic vibration booms forth from $n's mouth!", FALSE,
            ch, 0, 0, TO_ROOM);
        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_sound_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int starving_man(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    char           *obj_name,
                   *vict_name,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0;
    int             has_danish = 0;
    struct obj_data *i;
    char           *temp;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (!cmd && !number(0, 10)) {
        act("$n says, 'Pardon me sire, might you spare a poor man some food?'",
            FALSE, mob, 0, 0, TO_ROOM);
        return (TRUE);
    }

    /*
     * TALK TO ME!!!
     */

    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }
        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != STARVING_MAN) {
            return (FALSE);
        }
        for (i = vict->carrying; i; i = i->next_content) {
            if (has_danish == 0) {
                if (i->type_flag == ITEM_TYPE_FOOD) {
                    has_danish = 1;
                } else {
                    has_danish = 0;
                }
            }
        }

        /*
         * Quest Text
         */
        if (has_danish == 0) {
            act("$n says, 'Pardon me sire, might you spare a poor man some "
                "food?'", FALSE, vict, 0, 0, TO_ROOM);
            return (TRUE);
        } 
        
        act("$n says, 'I'm too busy eating to talk!  Go Away!'", FALSE,
            vict, 0, 0, TO_ROOM);
        return (TRUE);
    } 
    
    if (cmd == 72) {
        /* give */
        arg = get_argument(arg, &obj_name);

        if (!obj_name || 
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            return (FALSE);
        }
        arg = get_argument(arg, &vict_name);
        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) ||
            IS_PC(vict)) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * Mob is not Starving Man
         */
        if (mob_index[vict->nr].vnum != STARVING_MAN) {
            return (FALSE);
        }
        
        if (!IS_IMMORTAL(ch)) {
            if (obj->type_flag != ITEM_TYPE_FOOD) {
                sprintf(buf, "tell %s Thank you, but that is not what I "
                             "desire.", GET_NAME(ch));
                command_interpreter(vict, buf);
                return (TRUE);
            } 
            
            test = 1;
            sprintf(buf, "%s %s", obj_name, vict_name);
            do_give(ch, buf, 0);
        } else {
            sprintf(buf, "%s %s", obj_name, vict_name);
            do_give(ch, buf, 0);
            if (obj->type_flag == ITEM_TYPE_FOOD) {
                test = 1;
            }
        }
    } else {
        return (FALSE);
    }

    if (test == 1) {
        if (vict->equipment[WIELD]) {
            sprintf(buf, "tell %s Thank you mighty hero.  Take this as a "
                         "token of my appreciation.", GET_NAME(ch));
            command_interpreter(vict, buf);
            temp = KeywordsToString( &vict->equipment[WIELD]->keywords, NULL );
            sprintf(buf, "remove %s", temp);
            free( temp );

            command_interpreter(vict, buf);
            sprintf(buf, "give Staff %s", GET_NAME(ch));
            command_interpreter(vict, buf);
        } else {
            sprintf(buf, "tell %s You are indeed a generous person, but I have "
                         "nothing to offer you in return.", GET_NAME(ch));
            command_interpreter(vict, buf);
        }
    }
    return (TRUE);
}

#define STRAHD_ZOMBIE 30000

int strahd_zombie(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *mobtmp;
    char            buf[128];

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }

        switch (number(0, 4)) {
        case 1:
            return (shadow(ch, cmd, arg, mob, type));
        case 2:
            if (number(0, 1)) {
                mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE), REAL);
                char_to_room(mobtmp, ch->in_room);
                act("A body part falls from $n and forms into $N!",
                    FALSE, ch, 0, mobtmp, TO_ROOM);

                /*
                 * avoid same name fighting 
                 */
                if (GET_NAME(ch) != GET_NAME(ch->specials.fighting)) {
                    sprintf(buf, "kill %s", GET_NAME(ch->specials.fighting));
                    command_interpreter(mobtmp, buf);
                }
            }
            return (TRUE);
            break;
        case 3:
            return (wraith(ch, cmd, arg, mob, type));
            break;
        default:
            return (shadow(ch, cmd, arg, mob, type));
            break;
        }
    }
    return (FALSE);
}

#define SUN_SWORD_RAVENLOFT  30750
#define MAX_SUN_SWORDS       3  /* to limit the number of sun swords */
#define HOLY_ITEM_RAVENLOFT  30751
#define STRAHD_VAMPIRE       30113      /* mob number of strahd */
#define STRAHD_RELOCATE_ROOM 30008      /* room to put second strahd */
#define STRAHD2_ITEM1        31798
#define STRAHD2_ITEM2        31799

int strahd_vampire(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *mobtmp;
    static struct char_data *tmp;
    struct obj_data *target_obj;
    char            buf[80];
    int             i,
                    r_num,
                    virtual,
                    hasitem = FALSE;
    static int      location = -1;
    static int      vampnum = 0;

    int             sun_loc[] = { 30041, 30085, 30088, 30025 };
    int             holy_loc[] = { 30037, 30015, 30086 };

    /*
     * first time he dies load another strahd at second location 
     * and force it to hate the person that he was fighting 
     */

    if (type == EVENT_DEATH) {
        switch (vampnum) {
        case -1:
            vampnum = 0;
            break;
        case 0:
            vampnum++;
            mobtmp = read_mobile(real_mobile(STRAHD_VAMPIRE), REAL);
            char_to_room(mobtmp, STRAHD_RELOCATE_ROOM);
            AddHated(mobtmp, tmp);
            SetHunting(mobtmp, tmp);

            /*
             * new items for strahd number 2
             */
            target_obj = objectRead(STRAHD2_ITEM1, VIRTUAL);
            objectGiveToChar(target_obj, mobtmp);

            target_obj = objectRead(STRAHD2_ITEM2, VIRTUAL);
            objectGiveToChar(target_obj, mobtmp);

            act("$n falls to the ground and crumbles into dust, a faint green"
                " shadow leaves the corpse.", FALSE, ch, 0, 0, TO_ROOM);
            act("$n summons an army of his faithful followers!", FALSE,
                mobtmp, 0, 0, TO_ROOM);
            ch = mobtmp;

            for (i = 0; i < 7; i++) {
                mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE), REAL);
                char_to_room(mobtmp, ch->in_room);
                add_follower(mobtmp, ch);
                SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
                AddHated(mobtmp, tmp);
            }
            command_interpreter(ch, "order followers guard on");
            return (TRUE);
            break;

        default:
            act("$n falls to the ground and crumbles into dust, a faint sigh "
                "of relief seems to fill the castle.", FALSE, ch, 0, 0,
                TO_ROOM);
            vampnum = -1;
            return (TRUE);
            break;
        }
    } else {
        if (vampnum == -1) {      
            /* fix this if the vamp is loaded and we are not dead */
            vampnum = 0;
        }
    }

    if ((cmd) || (!AWAKE(mob))) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        tmp = ch->specials.fighting;

        switch (number(0, 5)) {
        case 1:
            return (magic_user(mob, cmd, arg, mob, type));
            break;
        case 2:
            return (magic_user(mob, cmd, arg, mob, type));
            break;
        case 3:
            /*
             * drain one level, check for sun or holy item here before 
             * doing! 
             */
            if (tmp->equipment[WIELD]) {
                virtual = MAX( 0, tmp->equipment[WIELD]->item_number );

                if (virtual == SUN_SWORD_RAVENLOFT) {
                    hasitem = TRUE;
                }
            }

            if (tmp->equipment[HOLD]) {
                virtual = MAX( 0, tmp->equipment[HOLD]->item_number );

                if (virtual == HOLY_ITEM_RAVENLOFT) {
                    hasitem = TRUE;
                }
            }

            if (hasitem) {
                act("$n tries to bite $N, but is repelled by some powerful "
                    "force!", FALSE, ch, 0, tmp, TO_ROOM);
                return (TRUE);
            }

            /*
             * wel'p they screwed up and didn't have either the sun
             * sword or the holy symbol, lets drain'em one level! 
             */

            if (GetMaxLevel(tmp) <= 1) {
                Log("Strahd tried to drain char below level 1");
            } else {
                /*
                 * fix this later...  
                 */
                act("$n bites $N who looks drained and weak!", FALSE,
                    ch, 0, tmp, TO_ROOM);
                drop_level(tmp, BestClassIND(tmp), FALSE);
                set_title(tmp);
                do_save(tmp, NULL, 0);
            }
            break;

            /*
             * summon up Strahd Zombie and order it to protect and kill 
             */
        default:
            tmp = ch->specials.fighting;
            mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE), REAL);
            char_to_room(mobtmp, ch->in_room);
            add_follower(mobtmp, ch);
            SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);

            act("$n summons one of his faithful followers, $N!", FALSE,
                ch, 0, mobtmp, TO_ROOM);
            command_interpreter(ch, "order followers guard on");
            sprintf(buf, "order followers kill %s", GET_NAME(tmp));
            command_interpreter(ch, buf);
            return (TRUE);
            break;
        }
    }

    if (!ch->specials.fighting) {
        /*
         * is the holy symbol in game? 
         */
        if (location == -1) {
            if ((target_obj = get_obj_vis_world(ch, "baabaabowie", NULL)) && 
                target_obj->in_room != NOWHERE) {
                /*
                 * items already loaded! 
                 */
                location = target_obj->in_room;
                return( FALSE );
            } else if (vampnum != 0) {
                /*
                 * do not load the items since this is the second vamp 
                 */
            } else {
                /*
                 * item was not found, lets load up the items and
                 * randomise the locations! 
                 */

                Log("Loading Ravenloft random location items.");
                /*
                 * place sun sword ... 
                 */
                target_obj = objectRead(SUN_SWORD_RAVENLOFT, VIRTUAL);
                objectPutInRoom(target_obj, sun_loc[number(0, 3)]);

                /*
                 * place holy symbol 
                 */
                target_obj = objectRead(HOLY_ITEM_RAVENLOFT, VIRTUAL);
                objectPutInRoom(target_obj, holy_loc[number(0, 2)]);

                return (FALSE);
            }
        }
    }

    return (magic_user(mob, cmd, arg, mob, type));
}

int stu(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
        int type)
{
    if (cmd) {
        return 0;
    }
    switch (number(0, 60)) {
    case 0:
        command_interpreter(ch, "say I'm so damn cool, I'm too cool to hang "
                                "out with myself!");
        break;
    case 1:
        command_interpreter(ch, "say I'm really the NICEST guy you ever MEET!");
        break;
    case 2:
        command_interpreter(ch, "say Follow me for exp, gold and lessons in "
                                "ADVANCED C!");
        break;
    case 3:
        command_interpreter(ch, "say Mind if I upload 200 megs of pregnant XXX"
                                " gifs with no descriptions to your bbs?");
        break;
    case 4:
        command_interpreter(ch, "say Sex? No way! I'd rather jog 20 miles!");
        break;
    case 5:
        command_interpreter(ch, "say I'll take you OUT!!   ...tomorrow");
        break;
    case 6:
        command_interpreter(ch, "say I invented Mud you know...");
        break;
    case 7:
        command_interpreter(ch, "say Can I have a cup of water?");
        break;
    case 8:
        command_interpreter(ch, "say I'll be jogging down ventnor ave in 10 "
                                "minutes if you want some!");
        break;
    case 9:
        command_interpreter(ch, "say Just let me pull a few strings and I'll "
                                "get ya a site, they love me! - doesn't "
                                "everyone?");
        break;
    case 10:
        command_interpreter(ch, "say Pssst! Someone tell Manwe to sport me "
                                "some levels.");
        act("$n nudges you with his elbow.", FALSE, ch, 0, 0, TO_ROOM);
        break;
    case 11:
        command_interpreter(ch, "say Edgar! Buddy! Let's group and hack some "
                                "ogres to tiny quivering bits!");
        break;
    case 12:
        act("$n tells you, 'Xenon has bad taste in women!'", FALSE, ch, 0,
            0, TO_ROOM);
        act("$n screams in terror!", FALSE, ch, 0, 0, TO_ROOM);
        do_flee(ch, NULL, 0);
        break;
    case 13:
        if (number(0, 32767) < 10) {
            act("$n whispers to you, 'Dude! If you say 'argle bargle' to the "
                "glowing fido he'll raise you a level!'", FALSE, ch, 0, 0, 
                TO_ROOM);
            act("$n flexes.", FALSE, ch, 0, 0, TO_ROOM);
        }
        break;
    default:
        return (FALSE);
        break;
    }
    return (TRUE);
}

int sund_earl(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    if (cmd) {
        return (FALSE);
    }
    switch (number(0, 20)) {
    case 0:
        command_interpreter(ch, "say Lovely weather today.");
        return (TRUE);
    case 1:
        act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,
            TO_ROOM);
        return (TRUE);
    case 2:
        command_interpreter(ch, "say Hot performance at the gallows tonight.");
        act("$n winks suggestively.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    case 3:
        command_interpreter(ch, "say Must remember to up the taxes at my "
                                "convenience.");
        return (TRUE);
    case 4:
        command_interpreter(ch, "say Sundhaven is impermeable to the enemy!");
        act("$n growls menacingly.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    case 5:
        command_interpreter(ch, "say Decadence is the credence of the "
                                "abominable.");
        return (TRUE);
    case 6:
        command_interpreter(ch, "say I look at you and get a wonderful sense "
                                "of impending doom.");
        act("$n chortles merrily.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    case 7:
        act("$n touches his goatee ponderously.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    case 8:
        command_interpreter(ch, "say It's Mexican Madness night at Maynards!");
        act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    default:
        break;
    }

    return (FALSE);
}

#define TimnusRoom 22602
#define TIMNUSNORTHLIMIT 30
#define TIMNUSWESTLIMIT 9

int timnus(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *vict;
    byte            lspell;

    if (cmd) {
        if (cmd == 1 && ch->in_room == TimnusRoom &&
            GetMaxLevel(ch) > TIMNUSNORTHLIMIT && !IS_IMMORTAL(ch)) {
            if (!check_soundproof(ch)) {
                act("$n tells you 'Thou art not pure enough of heart.'",
                    TRUE, mob, 0, ch, TO_VICT);
            }
            act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
            return (TRUE);
        } else if (cmd == 4 && ch->in_room == TimnusRoom && 
                   GetMaxLevel(ch) > TIMNUSWESTLIMIT && !IS_IMMORTAL(ch)) {
            if (!check_soundproof(ch)) {
                act("$n tells you 'Thou art not pure enough of heart.'",
                    TRUE, mob, 0, ch, TO_VICT);
            }
            act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
            return (TRUE);
        }
        return (FALSE);
    } else if (!IS_PC(ch) && ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        } else if (check_soundproof(ch) || check_nomagic(ch, 0, 0)) {
            MakeNiftyAttack(ch);
        } else {
            vict = FindVictim(ch);
            if (!vict) {
                vict = ch->specials.fighting;
            }
            if (!vict) {
                return (FALSE);
            }
            lspell = number(0, GetMaxLevel(ch));

            if (!IS_PC(ch)) {
                lspell += GetMaxLevel(ch) / 5;
            }
            lspell = MIN(GetMaxLevel(ch), lspell);

            if (lspell < 1) {
                lspell = 1;
            }
            if (IS_AFFECTED(ch, AFF_BLIND) && lspell > 15) {
                act("$n utters the words 'Let me see the light!'", TRUE, ch, 
                    0, 0, TO_ROOM);
                cast_cure_blind(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, ch, 0);
                return (FALSE);
            }

            if (GET_MOVE(ch) < 0) {
                act("$n pulls a glass of lemonade out of thin air.  How "
                    "refreshing.", 1, ch, 0, 0, TO_ROOM);
                cast_refresh(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
                return (FALSE);
            }

            if (!IS_AFFECTED(ch, AFF_FIRESHIELD) && lspell > 39) {
                act("$n utters the words 'FLAME ON!!!'", 1, ch, 0, 0, TO_ROOM);
                cast_fireshield(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, ch, 0);
                return (FALSE);
            }

            if (!IS_AFFECTED(ch, AFF_SANCTUARY) && lspell > 25) {
                act("$n utters the words 'Don't you just hate it when I do "
                    "this?'", 1, ch, 0, 0, TO_ROOM);
                cast_sanctuary(GetMaxLevel(ch), ch, "",
                               SPELL_TYPE_SPELL, ch, 0);
                return (FALSE);
            }

            if (IS_AFFECTED(vict, AFF_SANCTUARY) && lspell > 25 && 
                GetMaxLevel(ch) >= GetMaxLevel(vict)) {
                act("$n utters the words 'Do unto others as you'd have them do"
                    " unto you...'", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
                return (FALSE);
            }

            if (IS_AFFECTED(vict, AFF_FIRESHIELD) && lspell > 25 && 
                GetMaxLevel(ch) >= GetMaxLevel(vict)) {
                act("$n utters the words 'Do unto others as you'd have them do"
                    " unto you...'", 1, ch, 0, 0, TO_ROOM);
                cast_dispel_magic(GetMaxLevel(ch), ch, "",
                                  SPELL_TYPE_SPELL, vict, 0);
                return (FALSE);
            }

            if (lspell > 40) {
                act("$n utters the words 'Here's a penny, go buy a brain, and "
                    "give me the change'", 1, ch, 0, 0, TO_ROOM);
                cast_feeblemind(GetMaxLevel(ch), ch, "",
                                SPELL_TYPE_SPELL, vict, 0);
                return (FALSE);
            }

            /*
             * well, spells failed or not needed, let's kick
             * someone :) 
             */

            FighterMove(ch);
            return (FALSE);
        }
    }
    return( FALSE );
}

int tmk_guard(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob, int type)
{
    char           *vict_name;
    struct char_data *vict;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    /*
     * TALK TO ME!!!
     */

    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) || 
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }
        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum == TMK_GUARD_ONE || 
            mob_index[vict->nr].vnum == TMK_GUARD_TWO) {
            switch (number(1, 10)) {
            case 1:
                act("$n says, 'That Bahamut scares me to no end.  I know he's "
                    "supposed to be good'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'and all, but anything with that much power is "
                    "just too dangerous!'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'I've heard rumors that his skin is magically "
                    "empowered to make him'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'resistant to attacks from clubs and such "
                    "weapons.  Heh.  What I'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'wouldn't give to get some armor made of that "
                    "skin.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n grins slyly, but turns away quickly to prevent you "
                    "from seeing.", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 2:
                act("$n says, 'Have you heard the legend of Bahamut, The "
                    "Platinum Dragon?  According'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'to the stories, when Bahamut feels that he may "
                    "be in danger, he pauses'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'to pray to the great dragon lord.  And his "
                    "prayers are answered by an'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'enormous blazing light that virtually melts "
                    "away the skin.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 3:
                act("$n says, 'Hello!  Hmmm.  Dragons?  From what I understand,"
                    " there is a great'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'power in the keep that can get you to a "
                    "mysterious realm of dragons'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'This realm is a gigantic, shifting maze where "
                    "danger lurks around'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'every corner.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 4:
                act("$n says, 'Watch out for that fountain.  Wierd things "
                    "happen around that thing.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 5:
                act("$n says, 'I had this wierd dream last night.  Wonder if "
                    "it means anything.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'These colors kept flashing over and over again "
                    "in the same pattern:'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'White, Prismatic, Crystaline, Dark Gray, Blue, "
                    "Black, Gold, Platinum.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'I wonder what it means.'", FALSE, vict, 0,
                    0, TO_ROOM);
                act("$n frowns thoughtfully", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 6:
                act("$n says, 'Stories are spreading about some strange "
                    "creature in the surounding'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'mountains that can increase a person's stamina."
                    "  Wild stories of'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'animal sacrifice and the lord only knows what "
                    "else.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 7:
                act("$n says, 'A long time ago, the king's magician made a "
                    "great staff that, when held,'", FALSE, vict, 0, 0, 
                    TO_ROOM);
                act("$n says, 'could knock over even the largests of beasts.  "
                    "The king bestowed this'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'staff to Bahamut as a gift for his protection "
                    "of the keep in a great'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'battle against Tiamat.'", FALSE, vict, 0, 0,
                    TO_ROOM);
                break;
            case 8:
                act("$n rubs his chin in deep thought", FALSE, vict, 0, 0,
                    TO_ROOM);
                act("$n says, 'Tales of fantasy...  I seem to remember a story"
                    " from when I was a child.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'Crazy stuff really.  But there was this one that"
                    " really intriqued me.'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'Supposedly, there is a spring hidden somewhere "
                    "in the realm who's waters'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'can restore a person's age if they give some "
                    "ancient staff to a nymph'", FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'that guards the it.  Probably rediculous "
                    "fancy.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            case 9:
                act("$n says, 'Ever hear the legend of the Staff of the Ages?'",
                    FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'Some say that it's mysteries contain the'",
                    FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'only possibility for the defeat of Bahamut.'",
                    FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'Though, I bet thats crazy.  There's *nothing*'",
                    FALSE, vict, 0, 0, TO_ROOM);
                act("$n says, 'that can take out Bahamut, The Ancient "
                    "Platinum.'", FALSE, vict, 0, 0, TO_ROOM);
                break;
            default:
                act("$n says, 'Leave me alone.  I've got work to do!'",
                    FALSE, vict, 0, 0, TO_ROOM);
                break;
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int TreeThrowerMob(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *vict;

    /*
     **  Throws people in various directions, value:(N=0,E=1,S=2,W=3,U=4,D=5)
     **  for checking a direction, add 1 to value, for throwing use value.
     */

    if (!cmd) {
        if (AWAKE(ch) && ch->specials.fighting) {
            /*
             **  take this person and throw them
             */
            vict = ch->specials.fighting;
            switch (ch->in_room) {
            case 13912: 
                /* Forest of Rhowyn, Bridge Troll */
                /* throw chars to the east */
                ThrowChar(ch, vict, 1); 
                return (FALSE);
                break;
            case 6224:
                /* Rhyiana, Sentinel Tree */
                /* throw chars to the south */
                ThrowChar(ch, vict, 2); 
                return (FALSE);
                break;
            default:
                return (FALSE);
            }
        }
    } else {
        switch (ch->in_room) {
        case 13912:
            /* Forest of Rhowyn, Bridge Troll */
            if (cmd == 1) {
                /* north+1 */
                act("$n blocks your way!", FALSE, mob, 0, ch, TO_VICT);
                act("$n blocks $N's way.", FALSE, mob, 0, ch, TO_NOTVICT);
                command_interpreter(mob, "say In order to pass, you must "
                                         "first defeat me!");
                /* evil grin ;) */
                do_action(mob, NULL, 97);
                return (TRUE);
            }
            break;
        case 6224:
            /* Rhyiana, Sentinel Tree */
            if (cmd == 1 && IS_EVIL(ch)) {
                act("The trees around you rustle loudly.", FALSE, mob, 0,
                    ch, TO_VICT);
                act("Suddenly, branches reach out, blocking your way!",
                    FALSE, mob, 0, ch, TO_VICT);
                act("The branches of a huge tree move suddenly, blocking $N's "
                    "way!", FALSE, mob, 0, ch, TO_NOTVICT);
                act("A low rumbling voice whispers in your ear, 'The way north"
                    " is forbidden to those of evil.'", FALSE, mob, 0, ch, 
                    TO_VICT);
                act("The trees seem to be whispering in the wind.",
                    FALSE, mob, 0, ch, TO_NOTVICT);
                return (TRUE);
            }
            break;
        default:
            return (FALSE);
            break;
        }
    }
    return (FALSE);
}


#define ROBBER 51823
#define MUGGER 51822
#define NABBER 51821
#define FILCHER 51824

int trinketlooter(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    int             toRoom = 0;
    struct obj_data *trinket,
                   *obj;

    if ((cmd) || (mob->in_room != 51837)) {
        return (FALSE);
    }
    obj = get_obj_in_list_vis(ch, "stash", real_roomp(ch->in_room)->contents);

    if (!obj) {
        return (FALSE);
    }
    trinket = get_obj_in_list_vis(ch, "trinket", obj->contains);

    if (!trinket) {
        /*
         * hm no trinket? better bide our time then 
         */
        do_say(mob, "Oooh, no more pretty shinies? I'll be back!", 1);
        return (FALSE);
    }

    /*
     * yay, there's a trinket to be had 
     */
    send_to_room("The greedy sinner looks around the tent, and doesn't fail "
                 "to notice the chest standing\n\r", mob->in_room);
    send_to_room("in the center. It sneaks up to the chest, quickly grabs a "
                 "trinket, and cackles gleefully.\n\r", mob->in_room);
    send_to_room("Still laughing like a madman, it quaffs a misty potion and "
                 "fades out of existence.\n\r", mob->in_room);

    objectTakeFromObject(trinket);
    objectGiveToChar(trinket, mob);
    char_from_room(mob);

    if (mob_index[ch->nr].vnum == ROBBER) {
        toRoom = 51848;
    }
    if (mob_index[ch->nr].vnum == NABBER) {
        toRoom = 51874;
    }
    if (mob_index[ch->nr].vnum == MUGGER) {
        toRoom = 51858;
    }
    if (mob_index[ch->nr].vnum == FILCHER) {
        toRoom = 51868;
    }
    if (toRoom == 0) {
        Log("Screwup in finding the right room to transport to, trinketlooter");
        return (FALSE);
    }

    char_to_room(mob, toRoom);
    act("$n appears in the middle of the room, giggling.", FALSE, mob, 0,
        0, TO_ROOM);
    act("$n gets a shiny trinket from $s pocket and smiles gleefully.",
        FALSE, ch, 0, 0, TO_ROOM);
    command_interpreter(mob, "put trinket stash");
    return( TRUE );
}

int Tysha(struct char_data *ch, int cmd, char *arg, struct char_data *mob)
{
    char           *tgt_name,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *tgt;
    struct affected_type af;

    if (cmd == 533) {
        /* 533 = beckon */
        /*
         * if target doesn't exist 
         */
        arg = get_argument(arg, &tgt_name);
        if (!tgt_name) {
            return (FALSE);
        }
        /*
         * if target exists but isn't visible to pc 
         */
        if (!(tgt = get_char_room_vis(ch, tgt_name))) {
            return (FALSE);
        }
        /*
         * if target is fighting 
         */
        if (tgt->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * if target is another pc 
         */
        if (IS_PC(tgt)) {
            return (FALSE);
        }
        /*
         * if target isn't Tysha 
         */
        if (mob_index[tgt->nr].vnum != 47975) {
            return (FALSE);
        }
        /*
         * if Tysha is already following someone else 
         */
        if (tgt->master) {
            return (FALSE);
        } else {
            /*
             * if the target is Tysha 
             */
            sprintf(buf, "I wanna go home!");
            do_say(tgt, buf, 19);

            /*
             * make her follow the first pc that beckons her... 
             */
            add_follower(tgt, ch);
            af.type = SPELL_CHARM_PERSON;
            af.duration = 24 * 18;
            af.modifier = 0;
            af.location = 0;
            af.bitvector = AFF_CHARM;
        }
    }
    return (FALSE);
}

int Vaelhar(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob)
{
    struct char_data *hero = NULL,
                   *i,
                   *next;
    struct obj_data *obj;

    if ((ch->followers) || (cmd || !AWAKE(ch))) {
        return (FALSE);
    }
    /*
     * scan room for people 
     */
    if (ch->in_room > -1) {
        for (i = real_roomp(ch->in_room)->people; i; i = next) {
            next = i->next_in_room;
            if (mob_index[i->nr].vnum == 47975) {
                /* 
                 * if tysha is in the room 
                 */
                do_say(i, "Grandpa! Oh grandpa, I'm home!", 0);
                /*
                 * make tysha follow Vaelhar 
                 */
                if (i->master) {
                    hero = i->master;
                    stop_follower(i);
                }

                add_follower(i, ch);
                act("Tysha runs up to Vaelhar and jumps into his arms, tears "
                    "of joy running down her face.", FALSE, ch, 0, 0, TO_ROOM);
                act("The old man can't hold back his own tears, as he twirls "
                    "the little girl around and around.", FALSE, ch, 0, 0, 
                    TO_ROOM);
                act("After some time, they quiet down and Tysha tells the old "
                    "man all about her ordeals.", FALSE, ch, 0, 0, TO_ROOM);
                act("She doesn't fail to mention how bravely you rescued her.",
                    FALSE, ch, 0, 0, TO_ROOM);
                act("When she's done talking, the old man stands up and walks"
                    " over to a cabinet,", FALSE, ch, 0, 0, TO_ROOM);
                act("He takes out a softly glowing mace.", FALSE, ch, 0, 0,
                    TO_ROOM);
                do_say(ch, "Thank you, brave hero, for returning my "
                           "granddaughter to me.", 0);
                do_say(ch, "She is all I have left. I'm old and feeble "
                           "nowadays, so perhaps", 0);
                do_say(ch, "this mace will have more use in your hands. You "
                           "certainly have", 0);
                do_say(ch, "the spirit to make use of it. I wish you well.",
                       0);
                act("Vaelhar hands over Brynn'Ath, Bane of the Blighted.",
                    FALSE, ch, 0, 0, TO_ROOM);

                /*
                 * give tysha's former master brynn'ath 
                 */
                obj = objectRead(47914, VIRTUAL);
                if (!obj) {
                    Log("No obj found in Vaelhar's proc");
                    return (FALSE);
                }

                if (!hero) {
                    Log("No hero found in Vaelhar's proc");
                    return (FALSE);
                }
                objectGiveToChar(obj, hero);
            }
        }

        if (!ch->followers) {
            do_say(ch, "Please, please bring my granddaughter back to me.", 0);
            act("Vaelhar looks up at you imploringly.", FALSE, ch, 0, 0,
                TO_ROOM);
        }
    }
    return( TRUE );
}

int VaporBreather(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *tar_char;

    if (cmd) {
        return (FALSE);
    }
    if (ch->specials.fighting && number(0, 2)) {
        act("$n rears back and inhales", FALSE, ch, 0, 0, TO_ROOM);
        act("$c0006A cloud of scalding vapor surges forth from $n's mouth!",
            FALSE, ch, 0, 0, TO_ROOM);

        for (tar_char = real_roomp(ch->in_room)->people; tar_char;
             tar_char = tar_char->next_in_room) {
            if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
                spell_vapor_breath(GetMaxLevel(ch), ch,
                                   ch->specials.fighting, 0);
            }
        }
        return (TRUE);
    }
    return (FALSE);
}

int village_princess(struct char_data *ch, int cmd, char *arg,
                     struct char_data *mob, int type)
{
    if ((cmd) || (!AWAKE(ch))) {
        return (FALSE);
    }
    /*
     * Add more princess type things, holler for help etc... 
     */
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && 
            GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }

        if (number(0, 1)) {
            return (fighter(mob, cmd, arg, mob, type));
        } else {
            return (magic_user(mob, cmd, arg, mob, type));
        }
    }
    return( FALSE );
}

int virgin_sac(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    if (type == EVENT_DEATH && ch->in_room == RESCUE_ROOM) {
        mob = read_mobile(real_mobile(SNAKE_GOD), REAL);
        char_to_room(mob, RESCUE_ROOM);
        act("$n says 'You have completed the sacrifice, you shall be rewarded "
            "with death!'", FALSE, mob, 0, 0, TO_ROOM);
        return (TRUE);
    }

    if ((!AWAKE(ch)) || (cmd)) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        if (GET_POS(ch) < POSITION_FIGHTING && GET_POS(ch) > POSITION_STUNNED) {
            StandUp(ch);
        }
        return (TRUE);
    }

    return (FALSE);
}

int winger(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
           int type)
{
    struct char_data *vict;

    if ((cmd) || (check_soundproof(ch)) || (check_nomagic(ch, 0, 0)) || 
        (IS_AFFECTED(ch, AFF_PARALYSIS))) {
        return (FALSE);
    }
    if (ch->specials.fighting) {
        vict = FindVictim(ch);

        if (!vict) {
            vict = ch->specials.fighting;
        }
        if (!vict) {
            return (FALSE);
        }
        if (IS_AFFECTED(vict, AFF_FLYING)) {
            act("$n utters the words 'Instant Wing Remover!'.", 1, ch, 0,
                0, TO_ROOM);
            cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,
                              vict, 0);
            return (TRUE);
        } else {
            /*
             * As per request of Alfred, if we are fighting and they
             * aren't affected by fly, then eek, we flee! 
             */
            do_flee(ch, NULL, 0);
            return (TRUE);
        }
    }
    return (FALSE);
}

int zork(struct char_data *ch, int cmd, char *arg, struct char_data *mob,
         int type)
{
    struct char_data *temp_ch;
    char            argument[150];
    char           *vict_name,
                    buf[MAX_INPUT_LENGTH];
    struct char_data *vict;
    struct obj_data *obj;
    int             test = 0,
                    r_num = 0;

    if (!AWAKE(ch)) {
        return (FALSE);
    }
    if (cmd == 531) {
        /* Talk */
        arg = get_argument(arg, &vict_name);

        if (!vict_name || !(vict = get_char_room_vis(ch, vict_name)) ||
            IS_PC(vict) || vict == ch) {
            return (FALSE);
        }

        if (vict->specials.fighting) {
            send_to_char("Not while they are fighting!\n\r", ch);
            return (TRUE);
        }

        if (mob_index[vict->nr].vnum != ZORK) {
            return (FALSE);
        }

        if (get_obj_vis_world(ch, "black pill thunder mountain", NULL)) {
            test = 1;
        }

        if (get_obj_vis_world(ch, "blue pill thunder mountain", NULL)) {
            test = 1;
        }

        if (get_obj_vis_world(ch, "elamin pen might", NULL)) {
            test = 1;
        }

        if (test == 0) {
            act("$n says, 'So, Elamin has finally found a courier to aid him.'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'How quaint.  Well, I'll tell you what I'll do.  If'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'you can figure out a simple riddle, I will grant'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'poor Elamin's pen.  Eat the correct pill and the'",
                FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'pen will be yours.  Eat the wrong pill and you "
                "will'", FALSE, vict, 0, 0, TO_ROOM);
            act("$n says, 'face an instant death.'", FALSE, vict, 0, 0,
                TO_ROOM);

            obj = objectRead(BLACK_PILL, VIRTUAL);
            objectGiveToChar(obj, ch);

            obj = objectRead(BLUE_PILL, VIRTUAL);
            objectGiveToChar(obj, ch);

            act("$n hands over 2 pills: one black and one blue.", FALSE,
                vict, 0, 0, TO_ROOM);
            act("$n says, 'The riddle is this:'", FALSE, vict, 0, 0,
                TO_ROOM);
            act("$n says, 'When one stands on solid ground'", FALSE, vict,
                0, 0, TO_ROOM);
            act("$n says, 'They look up and cannot deny'", FALSE, vict, 0,
                0, TO_ROOM);
            act("$n says, 'That the law of nature's demand'", FALSE, vict,
                0, 0, TO_ROOM);
            act("$n says, 'Determines the shade of this cloudless sky'",
                FALSE, vict, 0, 0, TO_ROOM);
            return (TRUE);
        }

        if (test == 1) {
            act("$n says, 'I can only tell you the riddle:'", FALSE, vict,
                0, 0, TO_ROOM);
            act("$n says, 'When one stands on solid ground'", FALSE, vict,
                0, 0, TO_ROOM);
            act("$n says, 'They look up and cannot deny'", FALSE, vict, 0,
                0, TO_ROOM);
            act("$n says, 'That the law of nature's demand'", FALSE, vict,
                0, 0, TO_ROOM);
            act("$n says, 'Determines the shade of this cloudless sky'",
                FALSE, vict, 0, 0, TO_ROOM);
            return (TRUE);
        }
    }

    if (cmd) {
        return (FALSE);
    }

    temp_ch = FindAnyVictim(ch);
    if (temp_ch && IS_PC(temp_ch)) {
        switch (number(0, 30)) {
        case 0:
            strcpy(argument, "like Cheese!");
            break;
        case 1:
            strcpy(argument, "loves to rock and roll all night!");
            break;
        case 2:
            strcpy(argument, "smells what the rock is cookin'!");
            break;
        case 3:
            strcpy(argument, "wants to be just like Xenon!");
            break;
        case 4:
            strcpy(argument, "just got whupped by Zork, The Jester!");
            break;
        case 5:
            strcpy(argument, "has $c0009'LOSER' $c0007stamped across their "
                             "forehead.");
            break;
        case 6:
            strcpy(argument, "would like to be spanked!");
            break;
        case 7:
            strcpy(argument, "has a brown wallet!");
            break;
        case 8:
            strcpy(argument, "killed Kenny!");
            break;
        case 9:
            strcpy(argument, "the Stone Giant Fibber/Wannabee");
            break;
        case 10:
            strcpy(argument, "the $c0012Dark Clown $c0007Worshipper");
            break;
        case 11:
            strcpy(argument, "thinks Pentak is on shrumes");
            break;
        case 12:
            strcpy(argument, "StormLight, The Warder of the Titanic Broodkins");
            break;
        case 13:
            strcpy(argument, "CoolGuy, Leader of the Fonzies");
            break;
        case 14:
            strcpy(argument, "Presley, The King has entered the Mud");
            break;
        case 15:
            strcpy(argument, "has fallen off of the stupid tree and hit "
                             "every branch on the way down.");
            break;
        case 16:
            strcpy(argument, "is a toad!!");
            break;
        case 17:
            strcpy(argument, "likes sheep and other assorted farm animals");
            break;
        case 18:
            strcpy(argument, "suffers from stupiditis");
            break;
        case 19:
            strcpy(argument, "says, 'Do not listen to a word I say, I'm "
                             "incompetent'");
            break;
        case 20:
            strcpy(argument, "wants you now!  Need only apply!");
            break;
        case 21:
            strcpy(argument, "is having passionate thoughts about...everyone"
                             "...all at once...");
            break;
        case 22:
            strcpy(argument, "is a sissy!");
            break;
        case 23:
            strcpy(argument, "was with Zork, The Jester all last night.");
            break;
        case 24:
            strcpy(argument, "is dumb");
            break;
        case 25:
            strcpy(argument, "White and the Seven Dorks.");
            break;
        case 26:
            strcpy(argument, "is downloading naughty things");
            break;
        case 27:
            strcpy(argument, "is afraid of Puff, the Magic Dragon");
            break;
        case 28:
            strcpy(argument, "thinks that he could easily beat Pentak up");
            break;
        case 29:
            strcpy(argument, "Marley - Soul Rebel");
            break;
        case 30:
            strcpy(argument, "is telepathetic");
            break;
        default:
            return (FALSE);
        }

        if (temp_ch->player.title) {
            free(temp_ch->player.title);
        }
        sprintf(buf, "%s %s", GET_NAME(temp_ch), argument);
        temp_ch->player.title = strdup(buf);
        return (TRUE);
    }

    return (FALSE);
}

int creeping_death(struct char_data *ch, int cmd, char *arg,
                   struct char_data *mob, int type)
{
    struct char_data *t,
                   *next;
    struct room_data *rp;
    struct obj_data *co,
                   *o;

    if (cmd) {
        return (FALSE);
    }
    if (check_peaceful(ch, 0)) {
        act("$n dissipates, you breathe a sigh of relief.", FALSE, ch, 0,
            0, TO_ROOM);
        extract_char(ch);
        return (TRUE);
    }

    if (ch->specials.fighting && IS_SET(ch->specials.act, ACT_SPEC)) {
        /* 
         * kill 
         */
        t = ch->specials.fighting;
        if (t->in_room == ch->in_room) {
            act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
            act("You are engulfed by $n, and are quickly disassembled.",
                FALSE, ch, 0, t, TO_VICT);
            act("$N is quickly reduced to a bloody pile of bones by $n.",
                FALSE, ch, 0, t, TO_NOTVICT);
            GET_HIT(ch) -= GET_HIT(t);
            die(t, '\0');

            /*
             * find the corpse and destroy it 
             */
            rp = real_roomp(ch->in_room);
            if (!rp) {
                Log("invalid room in creeping death?! oddness!");
                return (FALSE);
            }

            for (co = rp->contents; co; co = co->next_content) {
                if (IS_CORPSE(co)) {
                    /* 
                     * assume 1st corpse is victim's 
                     */
                    while (co->contains) {
                        o = co->contains;
                        objectTakeFromObject(o);
                        objectPutInRoom(o, ch->in_room);
                    }

                    /* remove the corpse */
                    objectExtract(co);
                }
            }
        }

        if (GET_HIT(ch) < 0) {
            act("$n dissipates, you breathe a sigh of relief.", FALSE, ch,
                0, 0, TO_ROOM);
            extract_char(ch);
        }
        return (TRUE);
    }

    /*
     * the generic is the direction of travel 
     */
    if (number(0, 2) == 0) {
        /* 
         * move 
         */
        if (!ValidMove(ch, (ch->generic) - 1)) {
            if (number(0, 2) != 0) {
                /* 
                 * 66% chance it dies 
                 */
                act("$n dissipates, you breathe a sigh of relief.", FALSE,
                    ch, 0, 0, TO_ROOM);
                extract_char(ch);
            }
        } else {
            do_move(ch, NULL, ch->generic);
        }
        return (FALSE);
    } else {
        /*
         * make everyone with any brains flee 
         */
        for (t = real_roomp(ch->in_room)->people; t; t = next) {
            next = t->next_in_room;
            if (t != ch && !saves_spell(t, SAVING_PETRI)) {
                do_flee(t, NULL, 0);
            }
        }

        /*
         * find someone in the room to flay 
         */
        for (t = real_roomp(ch->in_room)->people; t; t = next) {
            if (!t) {
                Log("found no mobiles in creeping death?! oddness!");
                return (FALSE);
            }

            next = t->next_in_room;
            if (!IS_IMMORTAL(t) && t != ch && !number(0, 1) && 
                IS_SET(ch->specials.act, ACT_SPEC)) {
                act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
                act("You are engulfed by $n, and are quickly disassembled.",
                    FALSE, ch, 0, t, TO_VICT);
                act("$N is quickly reduced to a bloody pile of bones by $n.",
                    FALSE, ch, 0, t, TO_NOTVICT);
                GET_HIT(ch) -= GET_HIT(t);
                die(t, '\0');

                /*
                 * find the corpse and destroy it 
                 */
                rp = real_roomp(ch->in_room);
                if (!rp) {
                    Log("invalid room called in creeping death?! oddness!");
                    return (FALSE);
                }

                for (co = rp->contents; co; co = co->next_content) {
                    if (IS_CORPSE(co)) {
                        /* assume 1st corpse is victim's */
                        while (co->contains) {
                            o = co->contains;
                            objectTakeFromObject(o);
                            objectPutInRoom(o, ch->in_room);
                        }
                        /* 
                         * remove the corpse 
                         */
                        objectExtract(co);
                    }
                }

                if (GET_HIT(ch) < 0) {
                    act("$n dissipates, you breathe a sigh of relief.",
                        FALSE, ch, 0, 0, TO_ROOM);
                    extract_char(ch);
                    return (TRUE);
                }
                break;
            }
        }
    }
    return( FALSE );
}

int Deshima(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *vict,
                   *next,
                   *i;
    struct room_data *rp;
    char            buf[MAX_STRING_LENGTH + 30];

    /*
     * Deshima was meant to paralyze through immunity 
     */

    if (!ch) {
        return (FALSE);
    }
    if (ch->in_room < 0) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);

    if (!rp) {
        return (FALSE);
    }
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if ((GET_DEX(ch)) != 21) {
        /* 
         * Deshima wants to have a nice dex value - how about 21? 
         */
        GET_DEX(ch) = 21;
    }

    if (!ch->specials.fighting) {
        return (FALSE);
    } else {
        /* 
         * we're fighting 
         */
        vict = ch->specials.fighting;
        if (!IS_AFFECTED(vict, AFF_PARALYSIS)) {
            if (!IS_IMMORTAL(vict) && (ch != vict)) {
                /* 
                 * let's see if he got lucky (25%) 
                 */
                switch (number(0, 3)) {
                case 1: 
                    /* he got lucky */
                    act("$c0008Deshima's eyes glow a dark c$c000rr$c000Rims"
                        "$c000ro$c0008n as he gazes into $n's eyes.", FALSE, 
                        vict, 0, 0, TO_NOTVICT);
                    act("$c0008$n's muscles seem to contract, but he manages "
                        "to break away from Deshima's gaze in time.$c0007", 
                        FALSE, vict, 0, 0, TO_NOTVICT);

                    sprintf(buf, "$c0008Deshima's eyes glow a dark c$c000rr"
                                 "$c000Rims$c000ro$c0008n as he gazes into "
                                 "your soul.\n\r");
                    send_to_char(buf, vict);

                    sprintf(buf, "$c0008You feel your muscles turn to stone, "
                                 "but manage to break the eye contact in time."
                                 "$c0007\n\r");
                    send_to_char(buf, vict);
                    break;
                default:
                    /* 
                     * let's nail that bugger! 
                     */
                    act("$c0008Deshima's eyes glow a dark c$c000rr$c000Rims"
                        "$c000ro$c0008n as he gazes into $n's eyes.", FALSE,
                        vict, 0, 0, TO_NOTVICT);
                    act("$c0008$n's muscles turn all rigid, making him unable "
                        "to fight.$c0007", FALSE, vict, 0, 0, TO_NOTVICT);

                    sprintf(buf, "$c0008Deshima's eyes glow a dark c$c000rr"
                                 "$c000Rims$c000ro$c0008n as he gazes into "
                                 "your soul.\n\r");
                    send_to_char(buf, vict);

                    sprintf(buf, "$c0008You feel your muscles turn to stone, "
                                 "and lose the ability to fight back."
                                 "$c0007\n\r");
                    send_to_char(buf, vict);
                    SET_BIT(vict->specials.affected_by, AFF_PARALYSIS);
                    break;
                }
            }
        } else {
            /* 
             * he's paralyzed, let's look for a new victim 
             */
            for (i = rp->people; i; i = next) {
                next = i->next_in_room;
                if (!IS_IMMORTAL(i) && ch != i && 
                    !IS_AFFECTED(i, AFF_PARALYSIS)) {
                    /*
                     * We got a fresh target on our hands here, let's
                     * start picking on him 
                     */
                    sprintf(buf, "$c0008Having effectively disabled %s, "
                                 "Deshima turns his attention to %s.$c0007",
                            GET_NAME(vict), GET_NAME(i));
                    act(buf, FALSE, i, 0, 0, TO_ROOM);
                    sprintf(buf, "$c0008Having effectively disabled %s, "
                                 "Deshima turns his attention to you!"
                                 "$c0007\n\r", GET_NAME(vict));
                    send_to_char(buf, i);
                    stop_fighting(ch);
                    set_fighting(ch, i);
                    return (FALSE);
                }
            }
        }
    }
    return (FALSE);
}


int mermaid(struct char_data *ch, int cmd, char *arg,
            struct char_data *mob, int type)
{
    struct char_data *i,
                   *next;
    struct affected_type af;
    char            buf[128];

    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (GET_POS(ch) < POSITION_SITTING) {
        return (FALSE);
    }
    if (check_soundproof(ch)) {
        return (FALSE);
    }
    if (check_nomagic(ch, 0, 0)) {
        return (FALSE);
    }
    /*
     * if ch is fighting, don't fire 
     */
    if (ch->specials.fighting) {
        return (FALSE);
    }
    /*
     * if ch already has a follower, don't fire 
     */
    if (ch->followers) {
        return (FALSE);
    }
    /*
     * ch not fighting, let's look for a victim 
     */
    if (ch->in_room <= -1) {
        return( FALSE );
    }

    /*
     * there's victims, let's see if we can harrass one 
     */
    for (i = real_roomp(ch->in_room)->people; i; i = next) {
        next = i->next_in_room;
        if (GET_RACE(i) == RACE_HUMAN && GET_SEX(i) == SEX_MALE && !IS_NPC(i) &&
            !IS_LINKDEAD(i) && !IS_IMMORTAL(i) && 
            !affected_by_spell(i, SPELL_CHARM_PERSON)) {
            if (!IsImmune(i, IMM_CHARM)) {
                if (!saves_spell(i, SAVING_PARA)) {
                    /* didn't make his save, his ass is mine! */
                    act("$n sings a beautiful song, oh my.. Your heart is sold"
                        " to $m.", FALSE, ch, 0, i, TO_VICT);
                    act("$n hums a merry tune while looking $N in the eyes.",
                        TRUE, ch, 0, i, TO_NOTVICT);
                    act("$N grins like the moron he is. $n's charms enchanted "
                        "him.", TRUE, ch, 0, i, TO_NOTVICT);

                    if (i->master) {
                        stop_follower(i);
                    }
                    add_follower(i, ch);
                    af.type = SPELL_CHARM_PERSON;
                    af.duration = 24;
                    af.modifier = 0;
                    af.location = 0;
                    af.bitvector = AFF_CHARM;
                    affect_to_char(i, &af);
                    return (TRUE);
                } else {
                    /* 
                     * made his save, give him some notice 
                     */
                    act("$n sings a beautiful song, oh my.. You're almost "
                        "willing to follow $m.", FALSE, ch, 0, i, TO_VICT);
                    act("$n hums a merry tune while looking $N in the eyes.", 
                        TRUE, ch, 0, i, TO_NOTVICT);
                }
            } else {
                /* 
                 * victim imm:charm, make him pay! 
                 */
                act("$n sings a beautiful song. At the end, you applaud and "
                    "give $m some coins.", FALSE, ch, 0, i, TO_VICT);
                act("$n hums a merry tune while looking $N in the eyes.", TRUE,
                    ch, 0, i, TO_NOTVICT);
                act("When it's finished, $N gives $m a round of applause and "
                    "hands over some coins.", TRUE, ch, 0, i, TO_NOTVICT);
                sprintf(buf, "give 20 coins %s", GET_NAME(ch));
                command_interpreter(i, buf);
            }
        }
    }
    return( TRUE );
}


#define COLLECTIBLE_1 3998
#define COLLECTIBLE_2 47961
#define COLLECTIBLE_3 51838
#define COLLECTIBLE_4 51839
#define REWARD_GNOME  52870
int gnome_collector(struct char_data *ch, int cmd, char *arg,
                    struct room_data *rp, int type)
{
    char           *obj_name,
                   *vict_name,
                   *arg1;
    struct char_data *gnome,
                   *tmp_ch = NULL,
                   *j,
                   *receiver = NULL;
    struct obj_data *obj,
                   *i,
                   *reward;
    int             obj_num = 0,
                    x = 0;
    int             HasCollectibles[4] = { 0, 0, 0, 0 };
    int             winner = 0,
                    found = 0;

    if (!ch || !AWAKE(ch)) {
        return (FALSE);
    }

    if (ch->in_room) {
        rp = real_roomp(ch->in_room);
    } else {
        Log("weirdness in gnome_collector, char not in a room");
        return (FALSE);
    }

    if (!rp) {
        Log("weirdness in gnome_collector, char's room does not exist");
        return (FALSE);
    }

    /*
     * let's make sure gnome doesn't get killed or robbed 
     */
    if (!IS_SET(rp->room_flags, PEACEFUL)) {
        SET_BIT(rp->room_flags, PEACEFUL);
    }
    if (!(gnome = get_char_room("gnome female collector", ch->in_room))) {
        Log("gnome_collector proc is attached to a mob without "
            "proper name, in room %ld", ch->in_room);
        return (FALSE);
    }

    if (!gnome) {
        Log("weirdness in gnome_collector, gnome found but not assigned");
        return (FALSE);
    }

    if (!IS_NPC(gnome)) {
        Log("weirdness in gnome_collector, gnome is not a mob");
        return (FALSE);
    }

    /*
     * if there's no pcs present in room, I'm gonna get going, get some
     * work done mate! 
     */

    j = rp->people;
    found = 0;

    while (j && !found) {
        if (IS_PC(j)) {
            /*
             * first PC in room (should be only cuz it's tunnel 1)
             * receives prize, if winner 
             */
            receiver = j;
            found = 1;
        }
        j = j->next_in_room;
    }

    if (!found) {
        /* 
         * no pcs in room! what am I doin here?!  let's get lost 
         */
        act("$n steps into $s little door, and it seals shut behind $s.",
            FALSE, gnome, 0, 0, TO_ROOM);

        while (gnome->carrying) {
            objectExtract(gnome->carrying);
        }

        extract_char(gnome);
        return (TRUE);
    }

    if (!IS_SET(gnome->specials.act, ACT_SENTINEL)) {
        SET_BIT(gnome->specials.act, ACT_SENTINEL);
    }
    if ((i = gnome->carrying)) {
        for (; i; i = i->next_content) {
            if (i->item_number == COLLECTIBLE_1) {
                HasCollectibles[0] = 1;
            } else if (i->item_number == COLLECTIBLE_2) {
                HasCollectibles[1] = 1;
            } else if (i->item_number == COLLECTIBLE_3) {
                HasCollectibles[2] = 1;
            } else if (i->item_number == COLLECTIBLE_4) {
                HasCollectibles[3] = 1;
            }
        }
        winner = 1;

        for (x = 0; x < 4; x++) {
            if (HasCollectibles[x] == 0) {
                winner = 0;
            }
        }

        if (winner) {
            act("$n says, 'Woop, I got everything i need now! Thank you ever "
                "so much.", FALSE, gnome, 0, 0, TO_ROOM);
            if ((reward = objectRead(REWARD_GNOME, VIRTUAL))) {
                act("I would express my gratitude by presenting you with this "
                    "magical ring.", FALSE, gnome, 0, 0, TO_ROOM);
                act("I came across it in an ancient traveller's corpse, back "
                    "in the day when", FALSE, gnome, 0, 0, TO_ROOM);
                act("I still got around. I never quite figured out its use, "
                    "but I'm sure it's", FALSE, gnome, 0, 0, TO_ROOM);
                act("more than it seems. I hope you will make good use of "
                    "it.'\n\r", FALSE, gnome, 0, 0, TO_ROOM);
                act("$N gives $p to $n.\n\r", FALSE, receiver, reward,
                    gnome, TO_ROOM);
                act("$N gives $p to you.\n\r", FALSE, receiver, reward,
                    gnome, TO_CHAR);

                objectGiveToChar(reward, receiver);
            }

            act("$n says, 'Right, gotta get going now, I'm impatient to start "
                "my experiments.", FALSE, gnome, 0, 0, TO_ROOM);
            act("$n steps into $s little door, and it seals shut behind $s.",
                FALSE, gnome, 0, 0, TO_ROOM);

            /*
             * extract carried items if any 
             */
            while (gnome->carrying) {
                objectExtract(gnome->carrying);
            }
            extract_char(gnome);
            return (TRUE);
        }
    }

    /*
     * talk does nothing, she's silent 
     */
    if (cmd == 531) {
        arg = get_argument(arg, &arg1);
        if (arg1 && (tmp_ch = get_char_room_vis(ch, arg1)) && 
            tmp_ch == gnome ) {
            oldSendOutput(ch, "%s looks at you in a meaningful way, but stays "
                          "silent.\n\r", gnome->player.short_descr);
            return (TRUE);
        }
        return (FALSE);
    }

    /*
     * give 
     */
    if (cmd == 72) {
        /*
         * determine the correct obj 
         */
        arg = get_argument(arg, &obj_name);
        if (!obj_name || 
            !(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
            return (FALSE);
        }

        obj_num = obj->item_number;

        arg = get_argument(arg, &vict_name);

        if ((!vict_name || !(tmp_ch = get_char_room_vis(ch, vict_name))) && 
            tmp_ch != gnome) {
            return (FALSE);
        }

        /*
         * found an object, and the correct person to give it to 
         */
        if (gnome->specials.fighting) {
            send_to_char("Not while she is fighting!\n\r", ch);
            return (TRUE);
        }

        /*
         * correct object? 
         */
        if (obj_num != COLLECTIBLE_1 && obj_num != COLLECTIBLE_2 &&
            obj_num != COLLECTIBLE_3 && obj_num != COLLECTIBLE_4) {
            /*
             * nope 
             */
            oldSendOutput(ch, "%s doesn't seem to be interested in that sort of "
                          "junk.\n\r", gnome->player.short_descr);
            return (TRUE);
        }
        
        act("$n says, 'Woah, good stuff! I've been looking for this thing for "
            "ages.'", FALSE, gnome, 0, 0, TO_ROOM);
        act("You give $p to $N.", FALSE, ch, obj, gnome, TO_CHAR);
        act("$n gives $p to $N.", FALSE, ch, obj, gnome, TO_ROOM);
        objectTakeFromChar(obj);
        objectGiveToChar(obj, gnome);
        return (TRUE);
    }
    return (FALSE);
}

/*
 * Sentinel's Zone 
 */
int janaurius(struct char_data *ch, int cmd, char *arg,
              struct char_data *mob)
{

    struct char_data *i,
                   *target;

    if (!ch->specials.fighting && !ch->attackers) {
        ch->generic = 0;
    }
    if (cmd || !AWAKE(ch)) {
        return (FALSE);
    }
    if (ch->generic) {
        return (FALSE);
    }
    if ((target = ch->specials.fighting)) {
        ch->generic = 1;
        for (i = character_list; i; i = i->next) {
            if (mob_index[i->nr].vnum == 51166) {
                AddHated(i, target);
                SetHunting(i, target);
            }
        }
    }
    return( FALSE );
}



/*
 * Ashamael's Citystate of Tarantis 
 */
#define NIGHTWALKER 49600
#define SPAWNROOM 49799
#define DEST_ROOM 49460
#define REAVER_RM 49701
#define WAITROOM 49601

#define TARANTIS_PORTAL 49701
#define REAVER_PORTAL 49460

/*
 * Proc that makes a portal spawn at nighttime 8pm, and close at dawn 5pm. 
 */
int portal_regulator(struct char_data *ch, struct room_data *rp, int cmd)
{
    struct room_data *spawnroom;
    struct char_data *nightwalker;
    struct obj_data *obj;
    extern struct time_info_data time_info;
    int             check = 0;

    if (time_info.hours < 20 && time_info.hours > 5) {
        /* 
         * it should not be there 
         */
        rp = real_roomp(WAITROOM);
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == TARANTIS_PORTAL) {
                    send_to_room("$c0008The dark portal suddenly turns "
                                 "sideways, shrinks to a mere sliver, and "
                                 "disappears completely!\n\r", WAITROOM);
                    objectExtract(obj);
                }
            }
        }

        rp = real_roomp(REAVER_RM);
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == TARANTIS_PORTAL) {
                    send_to_room("$c0008The dark portal suddenly turns "
                                 "sideways, shrinks to a mere sliver, and "
                                 "disappears completely!\n\r", REAVER_RM);
                    objectExtract(obj);
                }
            }
        }

        rp = real_roomp(DEST_ROOM);
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == REAVER_PORTAL) {
                    send_to_room("$c0008The dark portal suddenly turns "
                                 "sideways, shrinks to a mere sliver, and "
                                 "disappears completely!\n\r", DEST_ROOM);
                    objectExtract(obj);
                }
            }
        }                       
        
        /* 
         * all portals are gone, now do the transfer mob thing 
         */
        if (time_info.hours == 9) {
            spawnroom = real_roomp(SPAWNROOM);
            if (!spawnroom) {
                Log("No nightwalker spawnroom found, blame Ash.");
                return (FALSE);
            }

            while (spawnroom->people) {
                nightwalker = spawnroom->people;
                char_from_room(nightwalker);
                char_to_room(nightwalker, WAITROOM);
            }
        }
    } else {
        /* 
         * portals should appear 
         */
        rp = real_roomp(WAITROOM);
        check = 0;
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == TARANTIS_PORTAL) {
                    check = 1;
                }
            }

            if (!check && (obj = objectRead(TARANTIS_PORTAL, VIRTUAL))) {
                send_to_room("$c0008A sliver of darkness suddenly appears."
                             " It widens, turns sideways, and becomes a "
                             "portal!\n\r", WAITROOM);
                objectPutInRoom(obj, WAITROOM);
            }
        }

        rp = real_roomp(REAVER_RM);
        check = 0;
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == TARANTIS_PORTAL) {
                    check = 1;
                }
            }

            if (!check && (obj = objectRead(TARANTIS_PORTAL, VIRTUAL))) {
                send_to_room("$c0008A sliver of darkness suddenly appears. It "
                             "widens, turns sideways, and becomes a "
                             "portal!\n\r", REAVER_RM);
                objectPutInRoom(obj, REAVER_RM);
            }
        }

        rp = real_roomp(DEST_ROOM);
        check = 0;
        if (rp) {
            for (obj = rp->contents; obj; obj = obj->next_content) {
                if (obj->item_number == REAVER_PORTAL) {
                    check = 1;
                }
            }

            if (!check && (obj = objectRead(REAVER_PORTAL, VIRTUAL))) {
                send_to_room("$c0008A sliver of darkness suddenly appears. It "
                             "widens, turns sideways, and becomes a "
                             "portal!\n\r", DEST_ROOM);
                objectPutInRoom(obj, DEST_ROOM);
            }
        }
    }
    return (FALSE);
}


#define ING_1   52870
#define ING_2   52878
#define ING_3   52879
#define ING_4   52880
#define ING_5   44180

#define SMITH_SHIELD 52877

int master_smith(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *yeelorn;
    struct room_data *rp = NULL;
    struct obj_data *obj,
                   *i,
                   *obj1,
                   *obj2,
                   *obj3,
                   *obj4,
                   *obj5;
    int             found = 0;
    int             found1 = 0;
    int             found2 = 0;
    int             found3 = 0;
    int             found4 = 0;
    int             found5 = 0;

    if (!ch || !cmd || IS_NPC(ch)) {
        return (FALSE);
    }

    if (ch->in_room) {
        rp = real_roomp(ch->in_room);
    }
    if (!rp) {
        return (FALSE);
    }
    if (!(yeelorn = FindMobInRoomWithFunction(ch->in_room, master_smith))) {
        Log("proc assigned to a mob that cannot be found?!");
        return (FALSE);
    }

    if (cmd == 72) { 
        /* 
         * give 
         */
        send_to_char("Giving away your stuff for free? Better rethink "
                     "this.\n\r", ch);
        return (TRUE);
    }

    if (cmd == 86) {
        /* 
         * ask 
         */
        send_to_char("Yeelorn says, 'Aye, yer curious if yev got summat that I"
                     " could use, eh? Right, lemme\n\r", ch);
        send_to_char("rummage through yer packs and see if there's aught I kin"
                     " use fer raw material.'\n\r", ch);
        send_to_char("\n\r", ch);
        send_to_char("Yeelorn searches through your bags, mumbling to himself"
                     " about people toting along\n\r", ch);
        send_to_char("all kinds of useless junk, and should they not put a "
                     "little more thought to what\n\r", ch);
        send_to_char("they're packing and what a tasty looking loaf of "
                     "pipeweed is that.'\n\r", ch);
        send_to_char("\n\r", ch);

        /*
         * check items 
         */
        for (i = object_list; i; i = i->next) {
            if (i->item_number == ING_1) {
                obj = i;

                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    found1 = 1;
                }
            }   

            if (i->item_number == ING_2) {
                obj = i;

                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    found2 = 1;
                }
            }

            if (i->item_number == ING_3) {
                obj = i;
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    found3 = 1;
                }
            }

            if (i->item_number == ING_4) {
                obj = i;
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    found4 = 1;
                }
            }

            if (i->item_number == ING_5) {
                obj = i;
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    found5 = 1;
                }
            }
        }

        if (!found1 && !found2 && !found3 && !found4 && !found5) {
            /* 
             * nothing found 
             */
            send_to_char("When he's finished, Yeelorn says, 'Hmm, no, ye gots "
                         "nothin of interest to me, matey.\n\r", ch);
            send_to_char("Come back when ya got some. Quit wasting me time "
                         "now, there's work to do.'\n\r", ch);
            return (TRUE);
        } 
        
        if (found1 && found2 && found3 && found4 && found5) {
            /* 
             * has everything, tell him the price 
             */
            send_to_char("When he's finished, Yeelorn exclaims, 'Woah mate, I"
                         " see ye'ns got everything I be needin\n\r", ch);
            send_to_char("to fix you up with a pretty shield. The shield of "
                         "thought, to start out with. Then we\n\r", ch);
            send_to_char("gots to reinforce it with a dragon bone and a shield"
                         " spell, enamel it with the silver\n\r", ch);
            send_to_char("from the sheet, and finally, we bring it to life "
                         "with the runes from the ring. Aye, that\n\r", ch);
            send_to_char("will be a nice piece of work. Course, I'll have to "
                         "charge ye a builder's fee. I'd think\n\r", ch);
            send_to_char("a million coins would do nicely. So. Whatcha think? "
                         "You wanna buy this service from me?'\n\r", ch);
            return (TRUE);
        }
        
        if (found1 || found2 || found3 || found4 || found5) {
            /* 
             * has at least one item 
             */
            send_to_char("When he's finished, Yeelorn says, 'Hmm, I see ye've "
                         "got yerself some pretty items.", ch);
            if (found1 == 1) {
                send_to_char("\n\rThis runed ring here, for instance. With the"
                             " aid of them runes, I may be able to embue\n\r",
                             ch);
                send_to_char("a creation of mine with some powerful magicks.  "
                             "Aye, sure looks promisin.", ch);
            }

            if (found2 == 1) {
                send_to_char("\n\rThis here dragon bone looks like the right "
                             "kind to reinforce a certain piece of armor\n\r",
                             ch);
                send_to_char("with.  Oh golly, I bet I could do somethin funky"
                             " with that.", ch);
            }

            if (found3 == 1) {
                send_to_char("\n\rWhat a pretty silver sheet ye've got there, "
                             "mate.  Could put that to some good use\n\r", ch);
                send_to_char("if I were to have to pretty up a little summat "
                             "or another.", ch);
            }

            if (found4 == 1) {
                send_to_char("\n\rHmmm, a scroll of shield.. I could use that "
                             "when hammering some crafty bit of armor.", ch);
            }

            if (found5 == 1) {
                send_to_char("\n\rA shield with a mind of its own, eh? That "
                             "looks int'resting. Bet I could sharpen up\n\r",
                             ch);
                send_to_char("that mind a wee bit. Aye, but I'd need some more"
                             " materials to do so..", ch);
            }

            /*
             * maybe they'll get a hint 
             */
            switch (number(1, 20)) {
            case 1:
                if (!found1) {
                    send_to_char("\n\r\n\rThe other day, I heard there's this"
                                 " tiny gnome who's nicked herself a useful "
                                 "ring.", ch);
                }
                break;
            case 2:
                if (!found2) {
                    send_to_char("\n\r\n\rYa know, dragonbone is ever a useful"
                                 " ingredient for crafty pieces of armor.", ch);
                }
                break;
            case 3:
                if (!found3) {
                    send_to_char("\n\r\n\rI just ran out of silver filigree "
                                 "too. Maybe ye kin find me a sheet or two?",
                                 ch);
                }
                break;
            case 4:
                if (!found4) {
                    send_to_char("\n\r\n\rAye, iffen ye want to imbue the metal"
                                 " with real power, ye'd need a scroll of "
                                 "spell too.", ch);
                }
                break;
            case 5:
                if (!found5) {
                    send_to_char("\n\r\n\rHeard there's a shield with a mind "
                                 "of its own. Wouldn't that be something to "
                                 "see, eh?", ch);
                }
                break;
            default:
                break;
            }
            send_to_char("'\n\r", ch);
        }
        return (TRUE);
    }

    if (cmd == 56) {
        /* 
         * buy 
         */
        obj1 = obj2 = obj3 = obj4 = obj5 = 0;
        for (i = object_list; i; i = i->next) {
            if (i->item_number == ING_1) {
                obj = i;

                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    obj1 = i;
                    found = 1;
                }
            }

            if (i->item_number == ING_2) {
                obj = i;
                
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }    
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    obj2 = i;
                    found = 1;
                }
            }
            
            if (i->item_number == ING_3) {
                obj = i;
            
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    obj3 = i;
                    found = 1;
                }
            }
            
            if (i->item_number == ING_4) {
                obj = i;
                
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    obj4 = i;
                    found = 1;
                }
            }
            
            if (i->item_number == ING_5) {
                obj = i;
            
                while (obj->in_obj) {
                    obj = obj->in_obj;
                }    
                if (obj->carried_by == ch || obj->equipped_by == ch) {
                    obj5 = i;
                    found = 1;
                }
            }
            
            if (found) {
                /* 
                 * transfer items to inventory for easy work later 
                 */
                if (i->equipped_by) {
                    obj = unequip_char(ch, i->eq_pos);
                    objectGiveToChar(obj, ch);
                } else if (!i->carried_by && i->in_obj) {
                    objectTakeFromObject(i);
                    objectGiveToChar(i, ch);
                } else {
                    Log("where is this item!?! bad spot in master_smith");
                    send_to_char("Ugh, something wrong with this proc, "
                                 "sorry.\n\r", ch);
                    return (TRUE);
                }
            }
            found = 0;
        }

        if (obj1 && obj2 && obj3 && obj4 && obj5) {
            /*
             * got all the items.. how about a million coins? 
             */
            if (GET_GOLD(ch) < 1000000) {
                send_to_char("Yeelorn says, 'Aye laddie, ye've got yerself "
                             "the items, but yer still some money short.'\n\r",
                             ch);
                return (TRUE);
            } 

            GET_GOLD(ch) -= 1000000;
            if (obj1->carried_by) {
                objectTakeFromChar(obj1);
                objectExtract(obj1);
            }
            if (obj2->carried_by) {
                objectTakeFromChar(obj2);
                objectExtract(obj2);
            }
            if (obj3->carried_by) {
                objectTakeFromChar(obj3);
                objectExtract(obj3);
            }
            if (obj4->carried_by) {
                objectTakeFromChar(obj4);
                objectExtract(obj4);
            }
            if (obj5->carried_by) {
                objectTakeFromChar(obj5);
                objectExtract(obj5);
            }

            if ((obj = objectRead(SMITH_SHIELD, VIRTUAL))) {
                objectGiveToChar(obj, ch);
                send_to_char("You give your items to Yeelorn, along with an "
                             "incredible heap of coins.\n\r", ch);
                send_to_char("Yeelorn pokes up his forge, and starts heating "
                             "the shield. Once it's red hot,\n\r", ch);
                send_to_char("he bends a few edges, bangs a few times, and the"
                             " dragonbone collar is firmly\n\r", ch);
                send_to_char("attached. He then proceeds with folding the "
                             "silver sheet over the shield, and\n\r", ch);
                send_to_char("heats it some more. More hammering melds the "
                             "silver with the shield and bone,\n\r", ch);
                send_to_char("making it look rather impressive. Then, Yeelorn "
                             "places it in a barrel ot brine,\n\r", ch);
                send_to_char("causing great clouds of noxious fumes to fill "
                             "the air. Next he orates the prayer\n\r", ch);
                send_to_char("from the scroll, which bursts into flame while "
                             "the spell is sucked into the\n\r", ch);
                send_to_char("shield. Once more he heats it up, and when it's "
                             "about to go to pieces from the\n\r", ch);
                send_to_char("heat, he takes it out and presses the runed ring"
                             " in the center of it. The shield\n\r", ch);
                send_to_char("seems to shudder. That must have been your "
                             "imagination. Shields are not alive.\n\r", ch);
                send_to_char("Or are they?\n\r", ch);
                send_to_char("Once the shield has cooled down, Yeelorn takes a"
                             " rag, and polishes it til it\n\r", ch);
                send_to_char("shines as bright as full moon. He giggles as he"
                             " hands it over.\n\r", ch);
                send_to_char("'Bout time yeh got on with yer adventuring' he "
                             "says, as he winks at you.\n\r", ch);
            } else {
                Log("could not load up prize for master_smith proc");
                send_to_char("Ugh, something wrong with this proc, "
                             "sorry..\n\r", ch);
            }

            return (TRUE);
        }
    }
    return (FALSE);
}

int nightwalker(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    struct char_data *freshmob;
    struct room_data *rp;
    struct obj_data *obj;
    char                buf[MAX_STRING_LENGTH];

    if (!ch) {
        return (FALSE);
    }
    if (!IS_NPC(ch)) {
        return (FALSE);
    }
    /*
     * if event death, do the die thing and load up a new mob at a
     * spawn_room 
     */
    if (type == EVENT_DEATH) {
        freshmob = read_mobile(real_mobile(NIGHTWALKER), REAL);
        char_to_room(freshmob, SPAWNROOM);
        act("$n crumbles to a pile of dust, but a tiny cloud seems to escape.",
            FALSE, ch, 0, 0, TO_ROOM);
        return (TRUE);
    }

    /*
     * else if light && not in spawn_room && not in a DARK room then burn
     * to a cinder, and load up a new one in the spawn room 
     */
    if (ch->in_room != SPAWNROOM && ch->in_room != WAITROOM && 
        !IS_SET(real_roomp(ch->in_room)->room_flags, DARK) && 
        time_info.hours > 6 && time_info.hours < 19) {
        act("A young ray of sunlight peeps over the horizon and strikes $n.",
            FALSE, ch, 0, 0, TO_ROOM);
        act("The power of the beam instantly reduces $n to a pile of dust!",
            FALSE, ch, 0, 0, TO_ROOM);
        GET_HIT(ch) = -1;
        die(ch, '\0');
        return (TRUE);
    }

    /*
     * Make them enter portal if they're in the right spot 
     */
    if (ch->in_room && ch->in_room == WAITROOM && (rp = real_roomp(WAITROOM))) {
        for (obj = rp->contents; obj; obj = obj->next_content) {
            if (obj->item_number == TARANTIS_PORTAL) {
                strcpy(buf, "portal");
                do_enter(ch, buf, 7);
                return (FALSE);
            }
        }
    }
    return (FALSE);
}


/*
 * start Talesian's procs 
 */

/* 
 * vnum assignments
 */
#define WAITTOGOHOME  98765
#define INPEACEROOM  98764
#define CORPSEOBJVNUM  37817
#define VENTOBJVNUM 37813
#define VENTROOMVNUM 37865
#define PHYLOBJVNUM 37812
#define MOBROOMVNUM 37859
#define HOMEZONE 115
#define ZOMBIEVNUM 37800
#define SKELETONVNUM 37801
/* 
 * vnums for the rooms where the 'specialty' mobs are stored
 */
#define DEMILICHSTORAGE  37860
#define DRACOLICHSTORAGE  37861
#define DEATHKNIGHTSTORAGE  37862
#define VAMPIRESTORAGE  37863
#define UNDEADDEMONSTORAGE 37864
#define GUARDIANMOBVNUM 37803   
/* 
 * MUST be same as guardianextraction proc mob
 */
#define GUARDIANHOMEROOMVNUM 37823
#define CLOSETROOMVNUM 37839
#define MISTROOMVNUM 37866
#define GENERICMOBVNUM 37816

/*
 * @Name:           sageactions 
 * @description:    The sage acts like a lich with phylactery.  
 *                  If the item (37812) is not where it should 
 *                  be, he chases it.  If it doesn't exist at all, 
 *                  he dies.  In combat he can summon help
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37802) 
 */
int sageactions(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{
    int             i = 0;
    int             j = 0;
    int             temp = 0;
    int             currroomnum = -1;
    char            buf[80];
    struct obj_data *curritem;
    struct obj_data *theitem = NULL;
    struct obj_data *ventobj;
    struct room_data *ventroom;
    struct obj_data *corpse;
    struct obj_data *tempobj;
    struct obj_data *nextobj;
    struct char_data *tempchar;
    struct room_data *currroom;
    int             k = 1;
    struct obj_data *remobj;
    struct obj_data *parentobj;
    int             GoodItemReal = -1;
    int             whatundead = 1;
    int             realcorpseid = -1;
    int             ventobjrnum = -1;

    if (cmd) {
        return (FALSE);
    }

    /* 
     * For the corpse that we toss around
     */
    GoodItemReal = PHYLOBJVNUM;
    realcorpseid = CORPSEOBJVNUM;
    ventroom = real_roomp(VENTROOMVNUM);
    ventobjrnum = VENTOBJVNUM;
    ventobj = get_obj_in_list_num(ventobjrnum, ventroom->contents);

    if (mob->specials.fighting || GET_POS(mob) < POSITION_STANDING) {
        if (!number(0, 3)) {
            /* 
             * 1 in 4 chance of summoning an undead 
             * helper - the special caster types load 
             * with zone in a dark-no-everything room, 
             * then get transferred in, so that they 
             * are spelled up
             */
            whatundead = number(1, 75);

            switch (whatundead) {
            case 1:
                /* 
                 * demi-lich v25000 r2069 in DEMILICHSTORAGE 
                 * if demi-lich in rnum
                 */
                if ((tempchar = get_char_room("demi lich", DEMILICHSTORAGE))) {
                    do_say(mob, "I call for you, your debt will be removed for"
                                " this service!", 0);
                    char_from_room(tempchar);
                    char_to_room(tempchar, mob->in_room);
                    act("$n suddenly appears, with cold hostility in the "
                        "remains of its eyes.", TRUE, tempchar, 0, 0, TO_ROOM);
                    AttackRandomChar(tempchar);
                }
                break;
            case 2:
                /*
                 * dracolich v5010 r942 in DRACOLICHSTORAGE 
                 * if dracolich in rnum
                 */
                if ((tempchar = get_char_room("dracolich lich",
                                              DRACOLICHSTORAGE))) {
                    do_say(mob, "I helped create you, come and fulfill your "
                                "debt!", 0);
                    char_from_room(tempchar);
                    char_to_room(tempchar, mob->in_room);
                    act("$n abruptly appears, filling the room with its bulk "
                        "and its malevolence.", TRUE, tempchar, 0, 0, TO_ROOM);
                    AttackRandomChar(tempchar);
                }
                /* 
                 * otherwise he has enough help!
                 */
                break;
            case 3:
                /* 
                 * The Wretched Vampire v2573 r2473 in VAMPIRESTORAGE 
                 * if The Wretched Vampire in rnum
                 */
                if ((tempchar = get_char_room("The Wretched Vampire",
                                              VAMPIRESTORAGE))) {
                    do_say(mob, "I provide you with sustenance, avail yourself"
                                " and aid me!", 0);
                    char_from_room(tempchar);
                    char_to_room(tempchar, mob->in_room);
                    act("A swirl of darkness coalesces, and the horrific figure"
                        " of $n becomes visible.", TRUE, tempchar, 0, 0, 
                        TO_ROOM);
                    AttackRandomChar(tempchar);
                }
                /* 
                 * otherwise he has enough help!
                 */
                break;
            case 4:
                /* 
                 * death-knight v16219 r1664 in DEATHKNIGHTSTORAGE
                 * if death knight in rnum
                 */
                if ((tempchar = get_char_room("death knight",
                                              DEATHKNIGHTSTORAGE))) {
                    do_say(mob, "I can aid you in your quest of death if you "
                                "come to my side now!", 0);
                    GET_RACE(tempchar) = RACE_UNDEAD;
                    char_from_room(tempchar);
                    char_to_room(tempchar, mob->in_room);
                    act("A dark portal appears, and the terrifying figure of "
                        "the $n steps through.", TRUE, tempchar, 0, 0, TO_ROOM);
                    AttackRandomChar(tempchar);
                }
                /* 
                 * else he has enough help!
                 */
                break;
            case 5:
                /* 
                 * Ghastly Undead Demon v23022 r2439 in UNDEADDEMONSTORAGE
                 * if ghastly-undead-demon in rnum
                 */
                if ((tempchar = get_char_room("Ghastly Undead Demon",
                                              UNDEADDEMONSTORAGE))) {
                    do_say(mob, "I will remove your torment, if you remove my "
                                "foes!", 0);
                    char_from_room(tempchar);
                    char_to_room(tempchar, mob->in_room);
                    act("From the ground, $n rises up, clawing his way towards "
                        "you.", TRUE, tempchar, 0, 0, TO_ROOM);
                    AttackRandomChar(tempchar);
                }
                /* 
                 * else he has enough help!
                 */
                break;
            default:
                /* 
                 * load a slightly random, but straight fighting undead
                 */
                act("$n makes an arcane gesture and shouts, 'Come servant, "
                    "come to my aid!'", TRUE, mob, 0, 0, TO_ROOM);
                switch (number(1, 50)) {
                case 1:
                    /* 
                     * Death v6999 r1118
                     */
                    CreateAMob(mob, 6999, 5, "");
                    break;
                case 2:
                    /* 
                     * master-centaur v13769 r1446
                     */
                    CreateAMob(mob, 13769, 5, "");
                    break;
                case 3:
                    /* 
                     * ringwraith v26900 r2576
                     */
                    CreateAMob(mob, 26900, 5, "");
                    break;
                case 4:
                    /* 
                     * ghost-captain v21140 r1902
                     */
                    CreateAMob(mob, 21140, 5, "");
                    break;
                case 5:
                    /* 
                     * soul-lost v30118 r2239
                     */
                    CreateAMob(mob, 30118, 5, "");
                    break;
                default:
                    /* 
                     * load either a zombie or a skeleton
                     */
                    if (number(0, 1)) {
                        /* 
                         * zombie v37800 r3104
                         */
                        CreateAMob(mob, ZOMBIEVNUM, 5, "");
                    } else {
                        /*
                         * skeleton v37801 r3105
                         */
                        CreateAMob(mob, SKELETONVNUM, 5, "");
                    }
                    break;
                }
                break;
            }
        }

        return (necromancer(mob, cmd, arg, mob, type));
    }

    for (curritem = object_list; curritem; curritem = curritem->next) {
        if (curritem->item_number == GoodItemReal) {
            theitem = curritem;
            i++;
        }
    }

    if (!number(0, 2)) {
        return (necromancer(mob, cmd, arg, mob, type));
    }

    if (i == 0) {
        /* 
         * Kill off char, and all that he carries
         */
        act("$n suddenly screams in agony and falls into a pile of dust.",
            FALSE, mob, 0, 0, TO_ROOM);

        for (j = 0; j < MAX_WEAR; j++) {
            if ((tempobj = mob->equipment[j])) {
                if (tempobj->contains) {
                    objectGiveToChar(unequip_char(mob, j), mob);
                } else {
                    MakeScrap(mob, NULL, tempobj);
                }
            }
        }

        while ((tempobj = mob->carrying)) {
            if (!tempobj->contains) {
                MakeScrap(mob, NULL, tempobj);
            } else {
                while ((nextobj = tempobj->contains)) {
                    objectTakeFromObject(nextobj);
                    objectGiveToChar(nextobj, mob);
                }
            }
        }

        GET_RACE(mob) = RACE_UNDEAD_ZOMBIE;
        damage(mob, mob, GET_MAX_HIT(mob) * 2, TYPE_SUFFERING);
        return (TRUE);
    }
    
    if (i == 1) {
        /* 
         * ok there's only one item found, now what is it on?
         * Check to see if it's in the correct obj, if not go hunt it
         */
        if (theitem->in_obj == ventobj) {
            /* 
             * In correct obj
             */
            if (mob->generic == WAITTOGOHOME) {
                currroom = real_roomp(mob->in_room);
                temp = 0;
                for (corpse = currroom->contents; corpse;
                     corpse = corpse->next_content) {
                    if (corpse->item_number == realcorpseid) {
                        temp = 1;
                        break;
                    }
                }

                if (temp) {
                    /* 
                     * remove the corpse, head back for home;
                     */
                    act("With a glance, $n dissolves the corpse, looking "
                        "stronger for it.", TRUE, mob, 0, 0, TO_ROOM);
                    objectExtract(corpse);
                }

                act("$n seems to biodegrade into nothingness, leaving only"
                    " the stench of decay.", TRUE, mob, 0, 0, TO_ROOM);
                char_from_room(mob);
                char_to_room(mob, MOBROOMVNUM);

                act("A smell of death and decay wafts by as $n emerges "
                    "from nothingness.", TRUE, mob, 0, 0, TO_ROOM);
                mob->generic = 0;
                return (TRUE);
            } 

            mob->generic = 0;
            return (FALSE);
        } 
        
        /* 
         * Not in the vent
         */
        if (theitem->in_room != -1) {
            /* 
             * In room, just go get it (but use messages, and 
             * ch->generic to have him take his time)
             */
            if (mob->generic != theitem->in_room) {
                /* 
                 * we will set ch->generic to in_room, and drop
                 * corpse there, after that we will go to corpse
                 * and get item, then return home
                 */
                mob->generic = theitem->in_room;
                for (tempobj = real_roomp(theitem->in_room)->contents;
                     tempobj; tempobj = tempobj->next) {
                    if (tempobj->item_number == realcorpseid) {
                        return (TRUE);
                    }
                }

                corpse = objectRead(CORPSEOBJVNUM, VIRTUAL);
                objectPutInRoom(corpse, theitem->in_room);
                currroomnum = mob->in_room;
                char_from_room(mob);
                char_to_room(mob, mob->generic);
                act("An old rotted corpse suddenly appears ten feet "
                    "above the ground and falls to the ground with a "
                    "sickening thud.", FALSE, mob, 0, 0, TO_ROOM);
                char_from_room(mob);
                char_to_room(mob, currroomnum);
            } else {
                /* 
                 * go to room, send item to storage, wait a pulse, 
                 * set ch->generic to go home again
                 */
                temp = 0;
                for (tempobj = real_roomp(theitem->in_room)->contents;
                     tempobj; tempobj = tempobj->next) {
                    if (tempobj->item_number == realcorpseid) {
                        temp = 1;
                    }
                }
                /* Even though the generic and in_room match, there is no corpse
                 * so we need to drop one, and that will be our action this time.
                 * If the corpse is there, then we will take the next step.
                 */
                if(!temp) {
                    corpse = objectRead(CORPSEOBJVNUM, VIRTUAL);
                    objectPutInRoom(corpse, theitem->in_room);
                    currroomnum = mob->in_room;
                    char_from_room(mob);
                    char_to_room(mob, mob->generic);
                    act("An old rotted corpse suddenly appears ten feet "
                        "above the ground and falls to the ground with a "
                        "sickening thud.", FALSE, mob, 0, 0, TO_ROOM);
                    char_from_room(mob);
                    char_to_room(mob, currroomnum);
                    return(TRUE);
                }
                if(theitem->in_room != mob->in_room) {
                act("$n waves $s hands, and a pair of rotted hands "
                    "reaches up through the ground and drags $m under.",
                    TRUE, mob, 0, 0, TO_ROOM);
                char_from_room(mob);
                char_to_room(mob, mob->generic);
                act("The rotting corpse suddenly sits up, it reaches "
                    "down into the ground like it was water, and pulls "
                    "up another being.", FALSE, mob, 0, 0, TO_ROOM);
                }

                if (theitem->in_room == mob->in_room) {
                    act("$n gestures and a bauble disappears from the "
                        "ground.", TRUE, mob, 0, 0, TO_ROOM);
                    objectTakeFromRoom(theitem);
                    objectPutInObject(theitem, ventobj);
                    mob->generic = WAITTOGOHOME;
                } else {
                    act("$n looks frustrated by something.", TRUE,
                        mob, 0, 0, TO_ROOM);
                    act("$n seems to biodegrade into nothingness, "
                        "leaving only the stench of decay.", TRUE, mob,
                        0, 0, TO_ROOM);
                    char_from_room(mob);
                    char_to_room(mob, MOBROOMVNUM);
                    
                    act("A smell of death and decay wafts by as $n "
                        "emerges from nothingness.", TRUE, mob, 0, 0,
                        TO_ROOM);
                    mob->generic = 0;
                }
            }
            return (TRUE);
        }
        
        /* 
         * object exists, but not in room, must be 
         * on person or in container in room
         */
        temp = 0;
        for (tempchar = character_list; tempchar;
             tempchar = tempchar->next) {
            if (HasObject(tempchar, PHYLOBJVNUM)) {
                temp = 1;
                break;
            }
        }

        if (temp) {
            /* 
             * a character has it let's go get him/her
             */
            if (real_roomp(tempchar->in_room)->zone == HOMEZONE) {
                /* 
                 * don't go hunting if char is still in zone
                 */
                if (mob->in_room == tempchar->in_room && 
                    mob->generic != INPEACEROOM) {
                    /* 
                     * what to do when you find him
                     */
                    if (!IS_SET(real_roomp(mob->in_room)->room_flags,
                                PEACEFUL)) {
                        act("$n glares at you, and launches to the "
                            "attack!", TRUE, mob, 0, tempchar, TO_VICT);
                        act("$n suddenly launches $mself at $N!",
                            TRUE, mob, 0, tempchar, TO_NOTVICT);
                        MobHit(mob, tempchar, 0);
                    } else {
                        sprintf(buf, "I will wait for you, %s.",
                                GET_NAME(tempchar));
                        do_say(mob, buf, 0);
                        act("$n becomes completely motionless.",
                            TRUE, mob, 0, 0, TO_ROOM);
                        mob->generic = INPEACEROOM;
                    }
                }
                return (FALSE);
            }
            if (mob->in_room == tempchar->in_room &&
                mob->generic != INPEACEROOM) {

                if (!IS_SET(real_roomp(mob->in_room)->room_flags,
                            PEACEFUL)) {
                    act("$n glares at you, and launches to the attack!",
                        TRUE, mob, 0, tempchar, TO_VICT);
                    act("$n suddenly launches $mself at $N!", TRUE, mob,
                        0, tempchar, TO_NOTVICT);
                    MobHit(mob, tempchar, 0);
                    mob->generic = WAITTOGOHOME;
                    return (FALSE);
                }
            }
            if(mob->in_room == tempchar->in_room &&
                mob->generic == INPEACEROOM) {

                if(!number(0,2)) {
                    act("$n glares at you!",
                        TRUE, mob, 0, tempchar, TO_VICT);
                    act("$n glares at $N!", TRUE, mob,
                        0, tempchar, TO_NOTVICT);
                }
                return(FALSE);
            }

            if (mob->generic != tempchar->in_room) {
                /* 
                 * mob has not dropped a corpse in the correct 
                 * room yet
                 */
                mob->generic = tempchar->in_room;
                for (tempobj = real_roomp(tempchar->in_room)->contents;
                     tempobj; tempobj = tempobj->next) {
                    if (tempobj->item_number == realcorpseid) {
                        return (TRUE);
                    }
                }
                corpse = objectRead(CORPSEOBJVNUM, VIRTUAL);
                objectPutInRoom(corpse, mob->generic);
                currroomnum = mob->in_room;
                char_from_room(mob);
                char_to_room(mob, tempchar->in_room);

                act("An old rotted corpse suddenly appears ten feet "
                    "above the ground and falls to the ground with a "
                    "sickening thud.", FALSE, mob, 0, 0, TO_ROOM);
                char_from_room(mob);
                char_to_room(mob, currroomnum);
                return (TRUE);
            } 
            
            if(mob->generic == tempchar->in_room) {
            /* 
             * mob dropped corpse in room, and char is still there 
             * so use corpse to transfer mob, then attack if not 
             * peace room
             */
                temp = 0;
                for (tempobj = real_roomp(tempchar->in_room)->contents;
                     tempobj; tempobj = tempobj->next) {
                    if (tempobj->item_number == realcorpseid) {
                        temp = 1;
                    }
                }
                if(!temp) {
                    mob->generic = -1;
                    return(FALSE);
                }
            act("$n waves $s hands, and a pair of rotted hands reaches "
                "up through the ground and drags $m under.", TRUE, mob,
                0, 0, TO_ROOM);
            char_from_room(mob);
            char_to_room(mob, mob->generic);

            act("The rotting corpse suddenly sits up, it reaches down "
                "into the ground like it was water, and pulls up "
                "another being.", FALSE, mob, 0, 0, TO_ROOM);
            act("$n points at you, that can't be good.",
                TRUE, mob, 0, tempchar, TO_VICT);
            act("$n slowly points at $N, which bodes ill for $N.",
                TRUE, mob, 0, tempchar, TO_NOTVICT);

            if (mob->in_room == tempchar->in_room &&
                mob->generic != INPEACEROOM) {
                if (!IS_SET(real_roomp(mob->in_room)->room_flags,
                            PEACEFUL)) {
                    act("$n glares at you, and launches to the attack!",
                        TRUE, mob, 0, tempchar, TO_VICT);
                    act("$n suddenly launches $mself at $N!", TRUE, mob,
                        0, tempchar, TO_NOTVICT);
                    MobHit(mob, tempchar, 0);
                    mob->generic = WAITTOGOHOME;
                    return (TRUE);
                } 

                sprintf(buf, "I will wait for you, %s.",
                        GET_NAME(tempchar));
                do_say(mob, buf, 0);
                act("$n becomes completely motionless.", TRUE, mob, 0,
                    0, TO_ROOM);
                mob->generic = INPEACEROOM;
                return (TRUE);
            }
            return (FALSE);
        }
        }
        
        /* 
         * in container, in room -> how about
         * moving outermost container to room,
         * then moving some items out of it, until 
         * the item is out
         */
        if (theitem->in_obj != NULL && theitem->in_obj != ventobj) {
            parentobj = theitem->in_obj;
            while (parentobj->in_obj != NULL) {
                parentobj = parentobj->in_obj;
            }
            
            /* 
             * Let's grab the parent object from the
             * floor, move it to the vent
             */
            currroomnum = mob->in_room;
            char_from_room(mob);
            char_to_room(mob, parentobj->in_room);

            act("A pair of skeletal hands reaches up from the earth and"
                " grabs $p, pulling it down into the ground.", TRUE,
                mob, parentobj, 0, TO_ROOM);
            char_from_room(mob);
            char_to_room(mob, currroomnum);
            objectTakeFromRoom(parentobj);
            objectPutInObject(parentobj, ventobj);

            while (theitem->in_obj != ventobj) {
                /* 
                 * it's in a container take out items until it's not
                 */
                parentobj = theitem->in_obj;
                while (parentobj->contains) {
                    tempobj = parentobj->contains;
                    objectTakeFromObject(tempobj);
                    objectPutInObject(tempobj, ventobj);
                }
            }
        }
        return (FALSE);
    } 
    
    if (i > 1) {
        /* 
         * I say randomly remove all but one, then hunt it
         */
        j = number(1, i);
        for (curritem = object_list; curritem;) {
            if (curritem->item_number == GoodItemReal) {
                remobj = curritem;
                curritem = curritem->next;
                if (k != j) {
                    objectExtract(remobj);
                }
                k++;
            } else {
                curritem = curritem->next;
            }
        }
    }

    return (necromancer(mob, cmd, arg, mob, type));
}



/*
 * @Name:           guardianextraction 
 * @description:    A mob function used by the guardians 
 *                  from guardianroom to clean themselves 
 *                  out of the room if there are no PC's 
 *                  there to fight, also resets the trap 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    room(37823) 
 */

int guardianextraction(struct char_data *ch, int cmd, char *arg,
                       struct char_data *mob, int type)
{
    struct char_data *tempchar;
    int             i = 0;
    struct room_data *rp;
    struct char_data *next_v;

    if(cmd) {
        return (FALSE);
    }

    if (mob->in_room != GUARDIANHOMEROOMVNUM) {
        return (FALSE);
    }

    rp = real_roomp(mob->in_room);

    for (tempchar = rp->people; tempchar;
         tempchar = tempchar->next_in_room) {
        if (IS_PC(tempchar) && !IS_IMMORTAL(tempchar)) {
            i++;
        }
    }

    if (i == 0) {
        for (tempchar = rp->people; tempchar; tempchar = next_v) {
            next_v = tempchar->next_in_room;
            if (IS_NPC(tempchar) && 
                !IS_SET(tempchar->specials.act, ACT_POLYSELF) &&
                mob_index[tempchar->nr].vnum == GUARDIANMOBVNUM) {
                extract_char(tempchar);
            }
        }
        rp->special = 0;
    }
    return (FALSE);
}


/*
 * @Name:           confusionmob 
 * @description:    This mob will attempt to make players flee 
 *                  using their 'best' method. Uses ability scores 
 *                  as saves rather than saves vs spells 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37811) 
 */

/*
 * Ideally, I'd like this to force characters to do semi-random 
 * commands instead of fleeing (maybe 3 out of 4 are random, 1 is 
 * user choice), but that would probably mean adding an affect
 */
int confusionmob(struct char_data *ch, int cmd, char *arg,
                 struct char_data *mob, int type)
{
    struct char_data *tempchar;
    int             makethemflee = 1;
    int             currroomnum = 0;

    if(cmd) {
        return(FALSE);
    }

    if ((tempchar = mob->specials.fighting)) {
        if (number(0, 4) > 0) {
            if (dice(3, 7) < GET_CHR(tempchar) &&
                dice(3, 7) < GET_INT(tempchar) &&
                dice(3, 7) < GET_WIS(tempchar)) {
                /* 
                 * three saves to avoid it fail any of the three, and it's 
                 * time to flee, this one is charisma
                 * to show what kind of confidence you have in yourself
                 * two saves to avoid it, this one intelligence to 
                 * understand what it is trying to do
                 * final save to avoid it, this one wisdom, to show 
                 * strength of will to avoid fleeing
                 * 3d7 = 12 average
                 */
                        
                /*
                 * they made 3 saves, I guess they can stay
                 */
                makethemflee = 0;
            }

            if (makethemflee) {
                currroomnum = tempchar->in_room;
                act("You gaze into $n's eyes and suddenly get frightened for no"
                    " reason!", FALSE, mob, 0, tempchar, TO_VICT);
                act("$n stares at $N and they suddenly look very scared!  "
                    "They've even stopped fighting back!", FALSE, mob, 0,
                    tempchar, TO_NOTVICT);

                /* 
                 * mages teleport
                 */
                stop_fighting(tempchar);
                if (HasClass(tempchar, CLASS_MAGIC_USER)) {
                    act("You panic and attempt to teleport to safety!",
                        FALSE, tempchar, 0, 0, TO_CHAR);
                    spell_teleport(GET_LEVEL(tempchar, MAGE_LEVEL_IND),
                                   tempchar, tempchar, 0);
                } else if (HasClass(tempchar, CLASS_CLERIC)) {
                    /* 
                     * clerics word and necromancers bind
                     */
                    act("You panic and attempt to recall to safety!",
                        FALSE, tempchar, 0, 0, TO_CHAR);
                    spell_word_of_recall(GET_LEVEL(tempchar, CLERIC_LEVEL_IND),
                                         tempchar, tempchar, 0);
                } else if (HasClass(tempchar, CLASS_NECROMANCER)) {
                    act("You panic and attempt to step through the shadows to "
                        "safety!", FALSE, tempchar, 0, 0, TO_CHAR);
                    spell_binding(GET_LEVEL(tempchar, NECROMANCER_LEVEL_IND),
                                  tempchar, tempchar, 0);
                } else if (HasClass(tempchar, CLASS_PSI)) {
                    /* 
                     * psis doorway to Zifnab
                     */
                    act("You panic and attempt to doorway to safety!",
                        FALSE, tempchar, 0, 0, TO_CHAR);
                    command_interpreter(tempchar, "doorway zifnab");
                } else {
                    /* 
                     * warriors types, druids, thieves, sorcerers flee
                     */
                    act("You panic and attempt to run away!", FALSE,
                        tempchar, 0, 0, TO_CHAR);
                    do_flee(tempchar, NULL, 0);
                }

                if (tempchar->in_room == currroomnum) {
                    act("You failed to get away, run!  RUN AWAY NOW!",
                        FALSE, tempchar, 0, 0, TO_CHAR);
                    do_flee(tempchar, NULL, 0);
                }
            }
        }
    }
    return (FALSE);
}


/*
 * @Name:           ghastsmell 
 * @description:    A mob proc which will disease people,
 *                  but only if the mob is fighting, and 
 *                  only when a player does a command, 
 *                  so the more you struggle, the worse it 
 *                  gets 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37804) 
 */
/* 
 * ghast smell which makes all in room get 
 * diseased if they are fighting someone
 */
int ghastsmell(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *tempchar;
    struct affected_type af;
    struct room_data *rp;

    if(!mob) {
        return (FALSE);
    }

    if (!number(0, 1)) {
        return (FALSE);
    }

    if (mob->specials.fighting) {
        af.type = SPELL_DISEASE;
        af.duration = 6;
        af.modifier = 0;
        af.location = 0;
        af.bitvector = 0;

        rp = real_roomp(mob->in_room);

        for (tempchar = rp->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (!IS_NPC(tempchar) && !IS_IMMORTAL(tempchar)) {
                act("A wave of stench rolls out from $n, "
                    "if a skunk's spray is like a dagger, \n\rthis is "
                    "like a claymore.", TRUE, mob, 0, tempchar, TO_VICT);
                if (!saves_spell(tempchar, SAVING_BREATH)) {
                    act("You immediately feel incredibly sick.", TRUE, mob, 0,
                        tempchar, TO_VICT);
                    affect_join(tempchar, &af, TRUE, FALSE);
                }
            }
        }
    }

    return (FALSE);
}

/*
 * @Name:           ghoultouch 
 * @description:    A mob proc which will stun people at
 *                  pretty much every opportunity, probably 
 *                  more of an annoyance than truly deadly 
 *                  if you have a group, but potentially 
 *                  killer if you are attempting to solo 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37805) 
 */
/* 
 * ghoul touch which does stunned for a round (GET_POS(ch) == POS_STUNNED, 
 * WAIT_STATE) (make this happen even if bashed)
 */
int ghoultouch(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob, int type)
{
    struct char_data *opponent;

    if( !mob ) {
        return( FALSE );
    }

    opponent = mob->specials.fighting;

    if (opponent && number(0, 1) && !IS_IMMUNE(opponent, IMM_HOLD)) {
        act("$n reaches out and touches you, you reel, losing focus!",
            FALSE, mob, 0, opponent, TO_VICT);
        act("$n reaches out and sends $N reeling, falling to the ground!",
            FALSE, mob, 0, opponent, TO_NOTVICT);
        GET_POS(opponent) = POSITION_STUNNED;
        stop_fighting(opponent);
        WAIT_STATE(opponent, PULSE_VIOLENCE);
    }

    return (FALSE);
}

/*
 * @Name:           shadowtouch 
 * @description:    A mob proc which will weaken, and tire 
 *                  opponents.  Weaken bigtime, down to a 
 *                  nautral 3 str + magical effects, and 
 *                  move get reduced 40% each time 
 * @Author:         Rick Peplinski (Talesian)
 * @Assigned to:    mob(37806) 
 */
/* 
 * shadow touch which does -str and -move
 */
int shadowtouch(struct char_data *ch, int cmd, char *arg,
                struct char_data *mob, int type)
{

    struct char_data *opponent;
    struct affected_type af;
    int             oppstr = 0;
    int             mod = 0;

    if( !mob ) {
        return( FALSE );
    }

    opponent = mob->specials.fighting;

    if (opponent && number(0, 1) && !IS_IMMUNE(opponent, IMM_DRAIN)) {
        act("$n reaches out and touches you, your limbs lose all their "
            "strength!", FALSE, mob, 0, opponent, TO_VICT);
        act("$n reaches out and touches $N, suddenly they look exhausted.",
            FALSE, mob, 0, opponent, TO_NOTVICT);

        if (!affected_by_spell(opponent, SPELL_WEAKNESS)) {
            if(GET_RADD(opponent) > 0) {
                oppstr += GET_RADD(opponent) / 10;
            }
            oppstr += GET_RSTR(opponent);
            /* 
             * basically take their natural strength and 
             * subtract enough to leave them with 3, 
             * anything magical added afterwards is ok
             */
            mod = oppstr - 3;
            af.type = SPELL_WEAKNESS;
            af.duration = 3;
            af.modifier = 0 - mod;
            af.location = APPLY_STR;
            af.bitvector = 0;
            affect_to_char(opponent, &af);
        }
        GET_MOVE(opponent) = (int) GET_MOVE(opponent) * .6;
    }
    return (FALSE);
}

/*
 * @Name:           moldexplosion 
 * @description:    A mob proc which if a player kills the mob, 
 *                  it poisons everybody in the room, no save 
 *                  except IMM_POISON.
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37807) 
 */
/* 
 * mold, which is fine if left alone, and 
 * blows up and poisons everybody
 * in room if it dies : type == EVENT_DEATH
 */
int moldexplosion(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{

    struct char_data *tempchar;
    struct affected_type af;

    if (type != EVENT_DEATH || cmd) {
        return (FALSE);
    }

    if (type == EVENT_DEATH) {
        af.type = SPELL_POISON;
        af.duration = 3;
        af.modifier = 0;
        af.location = APPLY_NONE;
        af.bitvector = AFF_POISON;

        act("Suddenly, the mold explodes, throwing a huge cloud of spores into"
            " the air!", FALSE, mob, 0, 0, TO_ROOM);

        for (tempchar = real_roomp(mob->in_room)->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (!IS_IMMORTAL(tempchar) && !IS_IMMUNE(tempchar, IMM_POISON)) {
                affect_to_char(tempchar, &af);
                send_to_char("You feel very sick.\n\r", tempchar);
            }
        }
    }
    return (FALSE);
}

/*
 * @Name:           boneshardbreather 
 * @description:    A mob proc which does a breath weapon 
 *                  for a bone golem. 
 * @Author:         Rick Peplinski (Talesian) 
 * @Assigned to:    mob(37809) 
 */
/* 
 * bone golem (breathe bone shards!)
 */
int boneshardbreather(struct char_data *ch, int cmd, char *arg,
                      struct char_data *mob, int type)
{
    struct char_data *tempchar;
    int             dam;

    if (cmd) {
        return (FALSE);
    }

    if (mob->specials.fighting && !number(0, 3)) {
        act("The dragon skull embedded in the bone golem's chest opens wide\n\r"
            "and spews out shards of bones that slice like the sharpest of "
            "daggers!", FALSE, mob, 0, 0, TO_ROOM);

        for (tempchar = real_roomp(mob->in_room)->people; tempchar;
             tempchar = tempchar->next_in_room) {
            if (!IS_NPC(tempchar) && !IS_IMMORTAL(tempchar)) {
                dam = dice(25, 10);
                if (saves_spell(tempchar, SAVING_SPELL)) {
                    dam = dam >> 1;
                }
                doroomdamage(tempchar, dam, TYPE_SLASH);
            }
        }

    }
    return (FALSE);
}

/*
 * @Name:           mistgolemtrap 
 * @description:    A mob proc which will 'capture' a PC 
 *                  (transfer him to a no-magic, no exit, 
 *                  silenced room) but everybody will be 
 *                  returned when mist golem dies. 
 * @Author:         Rick Peplinski (Talesian)
 * @Assigned to:    mob(37810) 
 */
/* 
 * mist golem (sucks somebody in, teleports them to a room "The Mist"
 * where can't get out until mist golem dies)
 */
int mistgolemtrap(struct char_data *ch, int cmd, char *arg,
                  struct char_data *mob, int type)
{
    struct char_data *opponent;
    struct char_data *tempchar;
    struct char_data *nextchar;

    if (cmd) {
        return (FALSE);
    }

    if (type == EVENT_DEATH) {
        for (tempchar = real_roomp(MISTROOMVNUM)->people; tempchar;
             tempchar = nextchar) {
            nextchar = tempchar->next_in_room;
            char_from_room(tempchar);
            char_to_room(tempchar, mob->in_room);
            act("Suddenly, the mist disappears and you find yourself somewhere"
                " else!", TRUE, tempchar, 0, 0, TO_CHAR);
            act("Suddenly, $n appears, looking bewildered.", TRUE,
                tempchar, 0, 0, TO_ROOM);
            do_look(tempchar, NULL, 15);
        }
    }

    if (mob->specials.fighting && !number(0, 1)) {
        opponent = mob->specials.fighting;
        act("The mist golem suddenly leans forward and engulfs you!\n\r"
            "You find yourself in another place.", TRUE, opponent, 0, 0,
            TO_CHAR);
        act("The mist golem suddenly leans forward and engulfs $n!\n\r"
            "They disappear into the body of the golem.", TRUE, opponent, 0, 0,
            TO_ROOM);
        stop_fighting(opponent);
        stop_fighting(mob);
        char_from_room(opponent);
        char_to_room(opponent, MISTROOMVNUM);
        do_look(opponent, NULL, 15);
        AttackRandomChar(mob);
    }
    return (FALSE);
}


/*
 * @Name:           skillfixer
 * @description:    A mob proc will fix the problem generated
 *                  by the lack of SKILL_KNOWN_?class? bit added
 *                  to the skill[].flags.  This proc will check
 *                  each player, and add the appropriate skill[].flags.
 *                  It will check each player on cmd then once in
 *                  a while it will go through the whole player list
 *                  and fix everybody to verify.
 * @Author:         Rick Peplinski (Talesian)
 * @Assigned to:    mob()
 */

int skillfixer(struct char_data *ch, int cmd, char *arg,
           struct char_data *mob, int type)
{
    int             i;
    int             j;
    int             k;
    int             m;
    int             n;
    char            buf[256];

    if (!ch->skills) {
        return( FALSE );
    }

    if (!(mob = get_char_room("nondescript man skill fixer", ch->in_room))) {
        Log("skill fixer proc is attached to a mob without proper name, "
            "in room %ld", ch->in_room);
        return (FALSE);
    }

    if (cmd != 531 || ch == mob ) {
        return( FALSE );
    }

    for (i = 0; i < MAX_SKILLS; i++) {
        if (!IS_SET(ch->skills[i].flags, SKILL_KNOWN)) {
            continue;
        }

        for (j = 0, m = 1; j < MAX_CLASS; j++, m <<= 1) {
            /*
             * Currently can only do a few classes because of size 
             * restrictions on char_skill_data 
             */
            if (!HasClass(ch, m) ||
                (m != CLASS_CLERIC && m != CLASS_MAGIC_USER &&
                 m != CLASS_SORCERER && m != CLASS_DRUID)) {
                continue;
            }

            for (k = 0; k < classes[j].skillCount; k++) {
                if (classes[j].skills[k].skillnum != i) {
                    continue;
                }

                if (m == CLASS_CLERIC && 
                    !IS_SET(ch->skills[i].flags, SKILL_KNOWN_CLERIC)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_CLERIC);
                } else if (m == CLASS_MAGIC_USER &&
                           !IS_SET(ch->skills[i].flags, SKILL_KNOWN_MAGE)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_MAGE);
                } else if (m == CLASS_SORCERER &&
                           !IS_SET(ch->skills[i].flags, SKILL_KNOWN_SORCERER)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_SORCERER);
                } else if (m == CLASS_DRUID &&
                           !IS_SET(ch->skills[i].flags, SKILL_KNOWN_DRUID)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_DRUID);
                }
            }

            for (n = 0; n < classes[j].mainskillCount; n++) {
                if (classes[j].mainskills[n].skillnum != i) {
                    continue;
                }

                if (m == CLASS_CLERIC && 
                    !IS_SET(ch->skills[i].flags, SKILL_KNOWN_CLERIC)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_CLERIC);
                } else if (m == CLASS_MAGIC_USER &&
                           IS_SET(ch->skills[i].flags, SKILL_KNOWN_MAGE)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_MAGE);
                } else if (m == CLASS_SORCERER &&
                           !IS_SET(ch->skills[i].flags, SKILL_KNOWN_SORCERER)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_SORCERER);
                } else if (m == CLASS_DRUID &&
                           !IS_SET(ch->skills[i].flags, SKILL_KNOWN_DRUID)) {
                    SET_BIT(ch->skills[i].flags, SKILL_KNOWN_DRUID);
                }
            }
        }
    }

    sprintf(buf, "tell %s Your skills have been patched up.", GET_NAME(ch));
    command_interpreter(mob, buf);
    return( TRUE );
}

/* procs for zone 39 */
int mazekeeper(struct char_data *ch, int cmd, char *arg,
               struct char_data *mob)
{
    char    buf[MAX_STRING_LENGTH];
    struct  obj_data *o;    
    int     objnum;
    struct  char_data *i;
    
    if (cmd != 17 || ch == mob || !arg || !ch || !mob) {
            return( FALSE );
    }

    objnum = 6575;
    
    for( o = real_roomp(ch->in_room)->contents; o; o = o->next_content) {
        if( o->item_number == objnum ) {
            return( FALSE );
        }
    }

    if (!strcasecmp(arg, "yes")) {
        for (i = real_roomp(ch->in_room)->people; i;
             i = i->next_in_room) {
            if (IS_FOLLOWING(ch, i) && GetMaxLevel(i) >= 41) {
                strcpy( buf, "say Your group is far too powerfull to enter!");
                command_interpreter(mob, buf);
                return( TRUE );
            }
        }
        strcpy(buf, "say Wonderful!! Let us begin!");
        command_interpreter(mob, buf);
        act("$c000WThe mazekeeper utters strange words and traces arcane"
            " symbols in the air.$c000w", FALSE, mob, 0, 0, TO_ROOM);
        act("$c000WSuddenly a large portal opens!$c000w",
            FALSE, mob, 0, 0, TO_ROOM);
        o = objectRead(6575, VIRTUAL);
        objectPutInRoom(o, ch->in_room);
        return(TRUE);
    } 
    
    if (!strcasecmp(arg, "no")) {
        strcpy(buf, "say Fine, if you don't want to play, you can die now!");
        command_interpreter(mob, buf);
        sprintf(buf, "kill %s", GET_NAME(ch));
        command_interpreter(mob, buf);
        return(TRUE);
    }
    return(FALSE);
} 

int mazekeeper_riddle_master(struct char_data *ch, int cmd, char *arg,
                             struct char_data *mob)
{
    char    buf[MAX_STRING_LENGTH];
    char    *argument = NULL,
            *arg1,
            *arg2;
    struct  obj_data *o;
    int     ret = FALSE;
    int     objnum;
    
    if (!ch || !mob || mob == ch || cmd != 83 || !arg) {
        return(FALSE);
    }

    objnum = 6580;

    for( o = real_roomp(ch->in_room)->contents; o; o = o->next_content) {
        if( o->item_number == objnum ) {
            return( FALSE );
        }
    }


    /* Keep the original arg for the caller in case we don't return TRUE */
    argument = strdup(arg);
    arg = argument;

    arg = get_argument(arg, &arg1);
    arg = get_argument(arg, &arg2);
    
    if (arg2 && !strcasecmp(arg2, "tomorrow")) {
        strcpy(buf, "say That is correct!");
        command_interpreter(mob, buf);
        strcpy(buf, "say I must say, I am impressed.");
        command_interpreter(mob, buf);
        strcpy(buf, "say It has been a very long time since the last "
                     "adventurer made it this far.");
        command_interpreter(mob, buf);
        strcpy(buf, "say Your reward is this armor crafted from special "
                     "ore found deep in our mines many years ago.");
        command_interpreter(mob, buf);
        strcpy(buf, "say Use it wisely, friend.  So long, and fare thee "
                     "well");
        command_interpreter(mob, buf);
        
        o = objectRead(6593, VIRTUAL);
        objectGiveToChar(o, ch);
        gain_exp(ch, 150000);
        
        act("The riddle master waves his hand and a shimmering portal "
            "appears.", FALSE, mob, NULL, NULL, TO_ROOM);
        act("Somehow you know that your journey has come to an end, and"
            "the portal is the way home.", FALSE, mob, NULL, NULL, TO_ROOM);

        o = objectRead(6580, VIRTUAL);
        objectPutInRoom(o, ch->in_room);
        ret = TRUE;
    }

    free(argument);
    return(ret);
}

    
int mazekeeper_riddle_common(struct char_data *ch, char *arg,
                             struct char_data *mob, struct riddle_answer *rid,
                             int ridCount, int exp, int portal)
{
    char    buf[MAX_STRING_LENGTH];
    char    *argument = NULL,
            *arg1,
            *arg2;
    struct  obj_data *o;
    int     ret = FALSE;
    int     i;
    
    /* Keep the original arg for the caller in case we don't return TRUE */
    argument = strdup(arg);
    arg = argument;

    arg = get_argument(arg, &arg1);
    arg = get_argument(arg, &arg2);
        
    if (!arg2) {
        free( argument );
        return( FALSE );
    }

    for( i = 0; i < ridCount && !ret; i++ ) {
        if( !strcasecmp(arg2, rid[i].answer ) ) {
            strcpy(buf, "say Excellent, you are correct!");
            command_interpreter(mob, buf);
            sprintf(buf, "say Take this %s as a reward.", rid[i].rewardText);
            command_interpreter(mob, buf);
            strcpy(buf, "say You may now move on to the next challenge, good "
                         "luck!");
            command_interpreter(mob, buf);
            
            o = objectRead(rid[i].reward, VIRTUAL);
            objectGiveToChar(o, ch);
            gain_exp(ch, exp);
            sprintf(buf, "$c000BYou receive $c000W%d $c000Bexperience!$c000w", 
                    exp);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);
            GET_GOLD(ch) = (GET_GOLD(ch) + exp);
            sprintf(buf, "$c000BYou receive $c000W%d $c000Bgold coins!$c000w", 
                    exp);
            act(buf, FALSE, ch, 0, 0, TO_CHAR);

            act("The riddler speaks in a strange language and traces an "
                "arcane symbol in the air.", FALSE, mob, 0, 0, TO_ROOM);
            act("A large portal opens in front of you!",
                FALSE, mob, 0, 0, TO_ROOM);

            o = objectRead(portal, VIRTUAL);
            objectPutInRoom(o, ch->in_room);

            act("The riddler disapears in a puff of smoke!", 
                FALSE, mob, NULL, NULL, TO_ROOM);
            char_from_room(mob);
            extract_char(mob);
            ret = TRUE;
        }
    } 
    
    if( !ret ) {
        strcpy(buf, "say HAH! Wrong answer, now you will die!");
        command_interpreter(mob, buf);
        sprintf(buf, "kill %s", GET_NAME(ch));
        command_interpreter(mob, buf);
        ret = TRUE;
    }

    free( argument );
    return(ret);
}


int mazekeeper_riddle_one(struct char_data *ch, int cmd, char *arg,
                          struct char_data *mob)
{
    static struct riddle_answer rid[] = {
        { "doll",   6581, "earring" },
        { "needle", 6582, "earring" },
        { "storm",  6583, "earring" }
    };
    
    if (!ch || !mob || mob == ch || cmd != 83 || !arg) {
        return(FALSE);
    }
    
    return( mazekeeper_riddle_common(ch, arg, mob, rid, NELEMENTS(rid),
                                     10000, 6576) );
}

int mazekeeper_riddle_two(struct char_data *ch, int cmd, char *arg,
                          struct char_data *mob)
{
    static struct riddle_answer rid[] = {
        { "breath", 6584, "ring" },
        { "tongue", 6585, "ring" },
        { "temper", 6586, "ring" }
    };
    
    if (!ch || !mob || mob == ch || cmd != 83 || !arg) {
        return(FALSE);
    }

    return( mazekeeper_riddle_common(ch, arg, mob, rid, NELEMENTS(rid),
                                     25000, 6577) );
}
    
int mazekeeper_riddle_three(struct char_data *ch, int cmd, char *arg,
                            struct char_data *mob)
{
    static struct riddle_answer rid[] = {
        { "time",        6587, "necklace" },
        { "temperature", 6588, "necklace" },
        { "pressure",    6589, "necklace" }
    };
    
    if (!ch || !mob || mob == ch || cmd != 83 || !arg) {
        return(FALSE);
    }

    return( mazekeeper_riddle_common(ch, arg, mob, rid, NELEMENTS(rid),
                                     50000, 6578) );
}
 
int mazekeeper_riddle_four(struct char_data *ch, int cmd, char *arg,
                            struct char_data *mob)
{
    static struct riddle_answer rid[] = {
        { "star",  6590, "bracelet" },
        { "sleep", 6591, "bracelet" },
        { "dream", 6592, "bracelet" }
    };
    
    if (!ch || !mob || mob == ch || cmd != 83 || !arg) {
        return(FALSE);
    }

    return( mazekeeper_riddle_common(ch, arg, mob, rid, NELEMENTS(rid),
                                     100000, 6579) );
}

struct dragon_def dragonTable[] = {
    { RACE_DRAGON_RED,   
      { { "$c000RA cone of fire",  spell_fire_breath } }, 1 },
    { RACE_DRAGON_BLACK, 
      { { "$c000GA cone of acid",  spell_acid_breath } }, 1 }, 
    { RACE_DRAGON_GREEN, 
      { { "$c000gA cloud of gas",  spell_gas_breath } }, 1 }, 
    { RACE_DRAGON_WHITE, 
      { { "$c000CA cone of frost", spell_frost_breath } }, 1 }, 
    { RACE_DRAGON_BLUE,
      { { "$c000BA bolt of lightning", spell_lightning_breath } }, 1 }, 
    { RACE_DRAGON_SILVER, 
      { { "$c000CA cone of frost", spell_frost_breath }, 
        { "$c000YA golden ray", spell_paralyze_breath } }, 2 },
    { RACE_DRAGON_GOLD,  
      { { "$c000RA cone of fire", spell_fire_breath }, 
        { "$c000gA cloud of gas", spell_gas_breath } }, 2 },
    { RACE_DRAGON_BRONZE,
      { { "$c000BA bolt of lightning", spell_lightning_breath }, 
        { "$c000yA repulsive cloud", spell_repulsion_breath } }, 2 },
    { RACE_DRAGON_COPPER,
      { { "$c000PA sticky cloud", spell_slow_breath }, 
        { "$c000GA cone of acid", spell_acid_breath } }, 2 },
    { RACE_DRAGON_BRASS,
      { { "$c0008A gloomy cloud", spell_sleep_breath }, 
        { "$c000yA cloud of blistering sand", spell_desertheat_breath } }, 2 },
    { RACE_DRAGON_AMETHYST,
      { { "$c000pA faceted violet lozenge", spell_lozenge_breath } }, 1 },
    { RACE_DRAGON_CRYSTAL,
      { { "$c000YA cone of glowing shards", spell_shard_breath } }, 1 },
    { RACE_DRAGON_EMERALD,
      { { "$c000cA loud keening wail", spell_sound_breath } }, 1 },
    { RACE_DRAGON_SAPPHIRE,
      { { "$c000cA cone of high-pitched, almost inaudible sound", 
         spell_sound_breath } }, 1 },
    { RACE_DRAGON_TOPAZ,
      { { "$c000yA cone of dehydration", spell_dehydration_breath } }, 1 },
    { RACE_DRAGON_BROWN,
      { { "$c000yA cloud of blistering sand", spell_desertheat_breath } }, 1 },
    { RACE_DRAGON_CLOUD,
      { { "$c000CAn icy blast of air", spell_frost_breath } }, 1 },
    { RACE_DRAGON_DEEP,
      { { "$c000gA cone of flesh-corrosive gas", spell_gas_breath } }, 1 },
    { RACE_DRAGON_MERCURY,
      { { "$c000YA beam of bright, yellow light", spell_light_breath } }, 1 },
    { RACE_DRAGON_MIST,
      { { "$c000CA cloud of scalding vapor", spell_vapor_breath } }, 1 },
    { RACE_DRAGON_SHADOW,
      { { "$c0008A cloud of blackness", spell_dark_breath } }, 1 },
    { RACE_DRAGON_STEEL,
      { { "$c000gA cube of toxic gas", spell_gas_breath } }, 1 },
    { RACE_DRAGON_YELLOW,
      { { "$c000YA cloud of blistering sand", spell_desertheat_breath } }, 1 },
    { RACE_DRAGON_TURTLE,
      { { "$c000CA cloud of scalding steam", spell_vapor_breath } }, 1 }
};
int dragonTableCount = NELEMENTS(dragonTable);

int dragon(struct char_data *ch, int cmd, char *arg, 
           struct char_data *mob, int type)
{
   char             buf[MAX_STRING_LENGTH];
   struct   char_data   *tar_char;
   int              i,
                    j,
                    level;
    
    if (cmd || GET_POS(ch) < POSITION_FIGHTING) {
        return( FALSE );
    }

    if (!ch->specials.fighting || !number(0, 2) ) {
        return( FALSE );
    }

    for( i = 0; i < dragonTableCount; i++ ) {
        if( dragonTable[i].race == GET_RACE(ch) ) {
            break;
        }
    }

    if( i == dragonTableCount ) {
        /* Bad dragon, he don't exist! */
        Log( "Dragon %s has an undefined breath type (race %d), defaulting to "
            "fire", GET_NAME(ch), GET_RACE(ch) );
        /* First entry is Red Dragon, breathes fire */
        i = 0;
    }

    j = number(0, dragonTable[i].breathCount - 1);

    act("$c000W$n rears back and inhales!$c000w", FALSE, ch, 0, 0, TO_ROOM);

    sprintf(buf, "%s spews forth from $n's mouth!$c000w",
            dragonTable[i].breath[j].spews );
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    WAIT_STATE(ch, PULSE_VIOLENCE * 3);
    level = GetMaxLevel(ch);
    
    for (tar_char = real_roomp(ch->in_room)->people; tar_char;
         tar_char = tar_char->next_in_room) {
        if (!in_group(ch, tar_char) && !IS_IMMORTAL(tar_char)) {
            (dragonTable[i].breath[j].func)(level, ch, tar_char, 0);
        }
    }
    return(TRUE);
}



void ThrowChar(struct char_data *ch, struct char_data *v, int dir)
{
    struct room_data *rp;
    int             or;
    char            buf[200];

    rp = real_roomp(v->in_room);
    if (rp && rp->dir_option[dir] && rp->dir_option[dir]->to_room &&
        EXIT(v, dir)->to_room != NOWHERE) {
        if (v->specials.fighting) {
            stop_fighting(v);
        }

        sprintf(buf, "%s picks you up and throws you %s\n\r",
                ch->player.short_descr, direction[dir].dir);
        send_to_char(buf, v);

        or = v->in_room;
        char_from_room(v);
        char_to_room(v, (real_roomp(or))->dir_option[dir]->to_room);
        do_look(v, NULL, 15);

        if (IS_SET(RM_FLAGS(v->in_room), DEATH) && !IS_IMMORTAL(v)) {
            NailThisSucker(v);
        }
    }
}




/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
