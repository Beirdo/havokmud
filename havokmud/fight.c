#include "config.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "protos.h"

#define DUAL_WIELD(ch) (ch->equipment[WIELD] && ch->equipment[HOLD]&&\
                        ITEM_TYPE(ch->equipment[WIELD])==ITEM_WEAPON && \
                        ITEM_TYPE(ch->equipment[HOLD])==ITEM_WEAPON)

#define MAX_NPC_CORPSE_TIME 5
#define MAX_PC_CORPSE_TIME 10

/*
 * real number of the severed head base item
 * for now I use the scraps number, should be fine
 */
#define SEVERED_HEAD    30

#define EXP_CAP         150000
#define EXP_CAP_1       250000
#define EXP_CAP_2       350000
#define EXP_CAP_3       450000
#define EXP_CAP_4       500000
#define EXP_CAP_5       550000
#define EXP_CAP_6       600000
#define EXP_CAP_7       700000
#define EXP_CAP_8       800000
#define EXP_CAP_OTHER   1000000

#define SMITH_SHIELD 52877

#define BAHAMUT 45461
#define BAHAMUT_ARMOR 45503
#define GUARDIAN_SIN 51821

/*
 * Structures
 */

extern struct index_data *obj_index;    /* Object maxxing */
struct char_data *combat_list = 0;      /* head of l-list of fighting
                                         * chars */
struct char_data *missile_list = 0;     /* head of l-list of fighting
                                         * chars */
struct char_data *combat_next_dude = 0; /* Next dude global trick */
struct char_data *missile_next_dude = 0;        /* Next dude global trick */
struct zone_data *zone_table;   /* table of reset data */

char            PeacefulWorks = 1;      /* set in @set */
char            DestroyedItems; /* set in MakeScraps */

/*
 * External structures
 */
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct message_list fight_messages[MAX_MESSAGES];
extern struct obj_data *object_list;
extern struct index_data *mob_index;
extern struct char_data *character_list;
extern struct spell_info_type spell_info[];
extern int      spell_index[MAX_SPL_LIST];
extern char    *spells[];
extern char    *ItemDamType[];
extern int      ItemSaveThrows[22][5];
extern struct dex_app_type dex_app[];
extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct int_app_type int_app[26];
extern struct wis_app_type wis_app[26];
extern const struct skillset weaponskills[];
extern char    *room_bits[];
extern int      thaco[MAX_CLASS][ABS_MAX_LVL];
int             can_see_linear(struct char_data *ch,
                               struct char_data *targ, int *rng, int *dr);
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
int             BarbarianToHitMagicBonus(struct char_data *ch);
int             berserkthaco(struct char_data *ch);
int             berserkdambonus(struct char_data *ch, int dam);
long            ExpCaps(int group_count, long total);
long            GroupLevelRatioExp(struct char_data *ch,
                                   int group_max_level,
                                   long experincepoints);

char           *replace_string(char *str, char *weapon, char *weapon_s,
                               char *location_hit, char *location_hit_s);
void            raw_kill_arena(struct char_data *ch);
char           *fread_string(FILE * f1);


void            DeleteHatreds(struct char_data *ch);
int             IsMagicSpell(int spell_num);
void            ch_printf(struct char_data *ch, char *fmt, ...);
int             IS_UNDERGROUND(struct char_data *ch);
int             clearpath(struct char_data *ch, long room, int direc);
 /*
  * Weapon attack texts
  */
struct attack_hit_type attack_hit_text[] = {
    {"hit", "hits"},
    {"pound", "pounds"},
    {"pierce", "pierces"},
    {"slash", "slashes"},
    {"whip", "whips"},
    {"claw", "claws"},
    {"bite", "bites"},
    {"sting", "stings"},
    {"crush", "crushes"},
    {"cleave", "cleaves"},
    {"stab", "stabs"},
    {"smash", "smashes"},
    {"smite", "smites"},
    {"blast", "blasts"},
    {"impale", "impales"},
    {"strike", "strikes"},      /* type RANGE_WEAPON */
    {"jab", "jabs"},
    {"punch", "punches"},
    {"strike", "strikes"}
};

 /*
  * Location of attack texts
  */
struct attack_hit_type location_hit_text[] = {
    {"in $S body", "body",},            /* 0 */
    {"in $S left leg", "left leg"},     /* 1 */
    {"in $S right leg", "right leg"},   /* 2 */
    {"in $S left arm", "left arm"},     /* 3 */
    {"in $S right arm", "right arm"},   /* 4 */
    {"in $S shoulder", "shoulder"},     /* 5 */
    {"in $S neck", "neck"},             /* 6 */
    {"in $S left foot", "left foot"},   /* 7 */
    {"in $S right foot", "right foot"}, /* 8 */
    {"in $S left hand", "left hand"},   /* 9 */
    {"in $S right hand", "right hand"}, /* 10 */
    {"in $S chest", "chest"},           /* 11 */
    {"in $S back", "back"},             /* 12 */
    {"in $S stomach", "stomach"},       /* 13 */
    {"in $S head", "head"}              /* 14 */
};

/*
 * The Fight related routines
 */

void appear(struct char_data *ch)
{
    act("$n suddenly appears!", FALSE, ch, 0, 0, TO_ROOM);

    if (affected_by_spell(ch, SPELL_INVISIBLE)) {
        affect_from_char(ch, SPELL_INVISIBLE);
    }
    if (affected_by_spell(ch, SPELL_GATHER_SHADOWS)) {
        affect_from_char(ch, SPELL_GATHER_SHADOWS);
    }
    if (affected_by_spell(ch, SPELL_INVIS_TO_ANIMALS)) {
        affect_from_char(ch, SPELL_INVIS_TO_ANIMALS);
    }
    if (affected_by_spell(ch, SPELL_INVIS_TO_UNDEAD)) {
        affect_from_char(ch, SPELL_INVIS_TO_UNDEAD);
    }
    REMOVE_BIT(ch->specials.affected_by, AFF_INVISIBLE);
}

int LevelMod(struct char_data *ch, struct char_data *v, int exp)
{
    float           ratio = 0.0;
    float           fexp;

    ratio = (float) GET_AVE_LEVEL(v) / GET_AVE_LEVEL(ch);

    if (ratio < 1.0) {
        fexp = ratio * exp;
    } else {
        fexp = exp;
    }

    return ((int) fexp);
}

int RatioExp(struct char_data *ch, struct char_data *victim, int total)
{
    if (!IS_SET(victim->specials.act, ACT_AGGRESSIVE) &&
        !IS_SET(victim->specials.act, ACT_META_AGG) &&
        !IS_AFFECTED(victim, AFF_CHARM)) {
        if (GetMaxLevel(ch) > 20) {
            total = LevelMod(ch, victim, total);
        }
    }

    if ((IS_SET(victim->specials.act, ACT_AGGRESSIVE) ||
         IS_SET(victim->specials.act, ACT_META_AGG)) &&
        !IS_AFFECTED(victim, AFF_CHARM)) {
        /*
         * make sure that poly mages don't abuse, by reducing their bonus
         */
        if (IS_NPC(ch)) {
            total *= 3;
            total /= 4;
        }
    }

    return (total);
}

void load_messages()
{
    FILE           *f1;
    int             i,
                    type;
    struct message_type *messages;
    char            chk[100];

    if (!(f1 = fopen(MESS_FILE, "r"))) {
        perror("read messages");
        assert(0);
    }

    /*
     * find the memset way of doing this...
     */

    for (i = 0; i < MAX_MESSAGES; i++) {
        fight_messages[i].a_type = 0;
        fight_messages[i].number_of_attacks = 0;
        fight_messages[i].msg = 0;
    }

    fscanf(f1, " %s \n", chk);

    i = 0;

    while (*chk == 'M') {
        fscanf(f1, " %d\n", &type);

        if (i >= MAX_MESSAGES) {
            Log("Too many combat messages.");
            exit(0);
        }

        CREATE(messages, struct message_type, 1);
        fight_messages[i].number_of_attacks++;
        fight_messages[i].a_type = type;
        messages->next = fight_messages[i].msg;
        fight_messages[i].msg = messages;

        messages->die_msg.attacker_msg = fread_string(f1);
        messages->die_msg.victim_msg = fread_string(f1);
        messages->die_msg.room_msg = fread_string(f1);
        messages->miss_msg.attacker_msg = fread_string(f1);
        messages->miss_msg.victim_msg = fread_string(f1);
        messages->miss_msg.room_msg = fread_string(f1);
        messages->hit_msg.attacker_msg = fread_string(f1);
        messages->hit_msg.victim_msg = fread_string(f1);
        messages->hit_msg.room_msg = fread_string(f1);
        messages->god_msg.attacker_msg = fread_string(f1);
        messages->god_msg.victim_msg = fread_string(f1);
        messages->god_msg.room_msg = fread_string(f1);
        fscanf(f1, " %s \n", chk);
        i++;
    }

    fclose(f1);
}

void update_pos(struct char_data *victim)
{
    if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POSITION_STUNNED)) {
        return;
    } else if (GET_HIT(victim) > 0) {
        if (!IS_AFFECTED(victim, AFF_PARALYSIS)) {
            if (!MOUNTED(victim)) {
                GET_POS(victim) = POSITION_STANDING;
            }
#if 0
            else GET_POS(victim) == POSITION_MOUNTED;
            /*
             * wasn't really doing anything.. (GH'04)
             * perhaps look into it later
             */
#endif
        } else {
            GET_POS(victim) = POSITION_STUNNED;
        }
    } else if (GET_HIT(victim) <= -11) {
        GET_POS(victim) = POSITION_DEAD;
    } else if (GET_HIT(victim) <= -6) {
        GET_POS(victim) = POSITION_MORTALLYW;
    } else if (GET_HIT(victim) <= -3) {
        GET_POS(victim) = POSITION_INCAP;
    } else {
        GET_POS(victim) = POSITION_STUNNED;
    }
}

int check_peaceful(struct char_data *ch, char *msg)
{
    struct room_data *rp;

    extern char     PeacefulWorks;

    if (!PeacefulWorks) {
        return (0);
    }
    if (!ch) {
        return (FALSE);
    }
    rp = real_roomp(ch->in_room);
    if (rp && rp->room_flags & PEACEFUL) {
        send_to_char(msg, ch);
        return 1;
    }
    return 0;
}

/*
 * start one char fighting another
 */
void set_fighting(struct char_data *ch, struct char_data *vict)
{
    if (ch->specials.fighting) {
        Log("Fighting character set to fighting another.");
        return;
    }

    if (vict->attackers <= 5) {
        vict->attackers += 1;
    } else {
        Log("more than 6 people attacking one target");
    }

    if (A_NOASSIST(ch, vict)) {
        act("$N is already engaged with someone else!", FALSE, ch, 0, vict,
            TO_CHAR);
#if 0
        stop_fighting(ch);
#endif
        return;
    }

    ch->next_fighting = combat_list;
    combat_list = ch;

    if (IS_AFFECTED(ch, AFF_SLEEP)) {
        affect_from_char(ch, SPELL_SLEEP);
    }
    /*
     * if memorizing, disturb it and remove it
     */

    if (affected_by_spell(ch, SKILL_MEMORIZE)) {
        affect_from_char(ch, SKILL_MEMORIZE);
        stop_memorizing(ch);
    }

    /*
     * same here
     */

    if (affected_by_spell(ch, SKILL_MEDITATE)) {
        affect_from_char(ch, SKILL_MEDITATE);
    }
    ch->specials.fighting = vict;
    GET_POS(ch) = POSITION_FIGHTING;

    /*
     * player has lost link and mob is killing him, force PC to flee
     */
    if (IS_LINKDEAD(ch)) {
        do_flee(ch, "\0", 0);
    }

    if (IS_LINKDEAD(vict)) {
        do_flee(vict, "\0", 0);
    }
}

/*
 * remove a char from the list of fighting chars
 */
void stop_fighting(struct char_data *ch)
{
    struct char_data *tmp;
    char            buf[300];

    if (!ch->specials.fighting) {
        sprintf(buf, "%s not fighting at invocation of stop_fighting",
                GET_NAME(ch));
        Log(buf);
        return;
    }

    ch->specials.fighting->attackers -= 1;

    if (ch->specials.fighting->attackers < 0) {
        Log("too few people attacking");
        ch->specials.fighting->attackers = 0;
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
        REMOVE_BIT(ch->specials.affected_by2, AFF2_BERSERK);
        act("$n calms down.", FALSE, ch, 0, 0, TO_ROOM);
        act("You calm down.", FALSE, ch, 0, 0, TO_CHAR);
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_STYLE_BERSERK)) {
        REMOVE_BIT(ch->specials.affected_by2, AFF2_STYLE_BERSERK);
        act("$n calms down.", FALSE, ch, 0, 0, TO_ROOM);
        act("You calm down.", FALSE, ch, 0, 0, TO_CHAR);
    }

    if (ch == combat_next_dude) {
        combat_next_dude = ch->next_fighting;
    }
    if (combat_list == ch) {
        combat_list = ch->next_fighting;
    } else {
        for (tmp = combat_list; tmp && (tmp->next_fighting != ch);
             tmp = tmp->next_fighting) {
            /*
             * Empty loop
             */
        }
        if (!tmp) {
            Log("Char fighting not found Error (fight.c, stop_fighting)");
            abort();
        }
        tmp->next_fighting = ch->next_fighting;
    }

    ch->next_fighting = 0;
    ch->specials.fighting = 0;
    if (MOUNTED(ch)) {
        GET_POS(ch) = POSITION_MOUNTED;
    } else {
        GET_POS(ch) = POSITION_STANDING;
    }
    update_pos(ch);
}

void make_corpse(struct char_data *ch, int killedbytype)
{
    struct obj_data *corpse,
                   *o,
                   *cp;
    struct obj_data *money;
    char            buf[MAX_INPUT_LENGTH + 80],
                    spec_desc[255];
    int             r_num,
                    i,
                    ADeadBody = FALSE;

    struct obj_data *create_money(int amount);

    CREATE(corpse, struct obj_data, 1);
    clear_object(corpse);

    corpse->item_number = NOWHERE;
    corpse->in_room = NOWHERE;
    corpse->is_corpse = TRUE;

    if (!IS_NPC(ch) || (!IsUndead(ch))) {
        /*
         * this is so we drop a severed head at the corpse, just for
         * visual
         */
        if ((GET_HIT(ch) < -50) && (killedbytype == TYPE_SLASH ||
                                    killedbytype == TYPE_CLEAVE)) {
            if ((r_num = real_object(SEVERED_HEAD)) >= 0) {
                cp = read_object(r_num, REAL);
                sprintf(buf, "head severed %s", corpse->name);
                corpse->beheaded_corpse = TRUE;
                if (cp->name) {
                    free(cp->name);
                }
                cp->name = strdup(buf);
                sprintf(buf, "the severed head of %s",
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
                if (cp->short_description) {
                    free(cp->short_description);
                }
                cp->short_description = strdup(buf);
                if (cp->action_description) {
                    free(cp->action_description);
                }
                cp->action_description = strdup(buf);
                sprintf(buf, "%s is lying on the ground.", buf);
                if (cp->description) {
                    free(cp->description);
                }
                cp->description = strdup(buf);

                cp->obj_flags.type_flag = ITEM_CONTAINER;
                cp->obj_flags.wear_flags = ITEM_TAKE;

                /*
                 * You can't store stuff in a corpse.
                 * Lennya: this makes corpses return a negative fullness %
                 */
                cp->obj_flags.value[0] = 0;

                /* race of corpse NOT USED HERE */
                cp->affected[0].modifier = GET_RACE(ch);

                /* level of corpse NOT USED HERE */
                cp->affected[1].modifier = GetMaxLevel(ch);

                /* corpse identifier */
                cp->obj_flags.value[3] = 1;

                if (IS_NPC(ch)) {
                    cp->obj_flags.timer = MAX_NPC_CORPSE_TIME + 2;
                } else {
                    cp->obj_flags.timer = MAX_PC_CORPSE_TIME + 3;
                }
                obj_to_room(cp, ch->in_room);
            }
        }

        /*
         * so we can have some description on the corpse
         */
        switch (killedbytype) {
        case SPELL_COLOUR_SPRAY:
            sprintf(spec_desc, "rainbow coloured corpse of %s is",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            break;
        case SPELL_SCOURGE_WARLOCK:
        case SPELL_ACID_BLAST:
            sprintf(spec_desc, "dissolving remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;
        case SPELL_FIRESHIELD:
        case SPELL_FIREBALL:
            sprintf(spec_desc, "smoldering remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;
        case SPELL_CHILLSHIELD:
            sprintf(spec_desc, "frozen remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;
        case SPELL_FINGER_OF_DEATH:
        case SPELL_LIFE_TAP:
        case SPELL_LIFE_DRAW:
        case SPELL_LIFE_LEECH:
        case SPELL_MIST_OF_DEATH:
            sprintf(spec_desc, "drained remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;

        case SPELL_CHAIN_LIGHTNING:
        case SPELL_LIGHTNING_BOLT:
            sprintf(spec_desc, "charred remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;
        case SKILL_PSIONIC_BLAST:
            sprintf(spec_desc, "jelly-fied remains of %s are",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            corpse->beheaded_corpse = TRUE;
            break;
        default:
            sprintf(spec_desc, "corpse of %s is",
                    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            break;
        }

        sprintf(buf, "corpse %s", spec_desc);
        if (corpse->name) {
            free(corpse->name);
        }
        corpse->name = strdup(buf);
        if (IS_AFFECTED(ch, AFF_FLYING)) {
            sprintf(buf, "The %s floating in the air here.", spec_desc);
        } else {
            sprintf(buf, "The %s lying here.", spec_desc);
        }
        if (corpse->description) {
            free(corpse->description);
        }
        corpse->description = strdup(buf);

        /*
         * for the dissolve message
         */
        sprintf(buf, "the corpse of %s",
                (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
        if (corpse->short_description) {
            free(corpse->short_description);
        }
        corpse->short_description = strdup(buf);

        ADeadBody = TRUE;
    } else if (IsUndead(ch)) {
        if (corpse->name) {
            free(corpse->name);
        }
        if (corpse->description) {
            free(corpse->description);
        }
        if (corpse->short_description) {
            free(corpse->short_description);
        }
        corpse->name = strdup("dust pile bones corpse");
        corpse->description = strdup("A pile of dust and bones is here.");
        corpse->short_description = strdup("a pile of dust and bones");
    }

    corpse->contains = ch->carrying;

    if (GET_GOLD(ch) > 0) {
        money = create_money(GET_GOLD(ch));
        GET_GOLD(ch) = 0;
        obj_to_obj(money, corpse);
    }

    corpse->obj_flags.type_flag = ITEM_CONTAINER;
    corpse->obj_flags.wear_flags = ITEM_TAKE;

    /*
     * You can't store stuff in a corpse
     */
    corpse->obj_flags.value[0] = 0;

    /*
     * race of corpse
     */
    corpse->affected[0].modifier = GET_RACE(ch);

    /*
     * level of corpse
     */
    corpse->affected[1].modifier = GetMaxLevel(ch);

    /*
     * corpse identifyer
     */
    corpse->obj_flags.value[3] = 1;

    if (ADeadBody) {
        corpse->obj_flags.weight = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
    } else {
        corpse->obj_flags.weight = 1 + IS_CARRYING_W(ch);
    }

    corpse->obj_flags.cost_per_day = 100000;
    if (IS_NPC(ch)) {
        corpse->obj_flags.timer = MAX_NPC_CORPSE_TIME;
    } else {
        corpse->obj_flags.timer = MAX_PC_CORPSE_TIME;
    }
    for (i = 0; i < MAX_WEAR; i++) {
        if (ch->equipment[i]) {
            obj_to_obj(unequip_char(ch, i), corpse);
        }
    }
    ch->carrying = 0;
    IS_CARRYING_N(ch) = 0;
    IS_CARRYING_W(ch) = 0;

    if (IS_NPC(ch)) {
        corpse->char_vnum = mob_index[ch->nr].virtual;
        corpse->char_f_pos = 0;
    } else {
        if (ch->desc) {
            corpse->char_f_pos = ch->desc->pos;
            corpse->char_vnum = 0;
        } else {
            corpse->char_f_pos = 0;
            corpse->char_vnum = 100;
        }
    }

    corpse->carried_by = 0;
    corpse->equipped_by = 0;

    corpse->next = object_list;
    object_list = corpse;

    for (o = corpse->contains; o; o = o->next_content) {
        o->in_obj = corpse;
    }
    object_list_new_owner(corpse, 0);

    obj_to_room(corpse, ch->in_room);

    /*
     * this must be set before dispel_magic, because if they
     * are flying and in a fly zone then the mud will crash
     */
    if (GET_POS(ch) != POSITION_DEAD) {
        GET_POS(ch) = POSITION_DEAD;
    }
    /*
     * remove spells
     */
    RemAllAffects(ch);
#if 0
    spell_dispel_magic(IMPLEMENTOR,ch,ch,0);
#endif

    check_falling_obj(corpse, ch->in_room);
}

void change_alignment(struct char_data *ch, struct char_data *victim)
{
    int             change,
                    diff,
                    d2;

    if (IS_NPC(ch)) {
        return;
    }
    if (IS_GOOD(ch) && (IS_GOOD(victim))) {
        change = (GET_ALIGNMENT(victim) / 200) *
                 (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
    } else if (IS_EVIL(ch) && (IS_GOOD(victim))) {
        change = (GET_ALIGNMENT(victim) / 30) *
                 (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
    } else if (IS_EVIL(victim) && (IS_GOOD(ch))) {
        change = (GET_ALIGNMENT(victim) / 30) *
                 (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
    } else if (IS_EVIL(ch) && (IS_EVIL(victim))) {
        change = ((GET_ALIGNMENT(victim) / 200) + 1) *
                 (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
    } else {
        change = ((GET_ALIGNMENT(victim) / 200) + 1) *
                 (MAX(1, GetMaxLevel(victim) - GetMaxLevel(ch)));
    }

    if (change == 0) {
        if (GET_ALIGNMENT(victim) > 0) {
            change = 1;
        } else if (GET_ALIGNMENT(victim) < 0) {
            change = -1;
        }
    }

    if (HasClass(ch, CLASS_DRUID) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        diff = 0 - GET_ALIGNMENT(ch);
        d2 = 0 - (GET_ALIGNMENT(ch) - change);
        if (diff < 0) {
            diff = -diff;
        }
        if (d2 < 0) {
            d2 = -d2;
        }
        if (d2 > diff) {
            send_to_char("Beware, you are straying from the path\n\r", ch);
            if (d2 > 150) {
                send_to_char("Your lack of faith is disturbing\n\r", ch);
                if (d2 > 275) {
                    send_to_char("If you do not mend your ways soon, you will"
                                 " be punished\n\r", ch);
                    if (d2 > 425) {
                        send_to_char("Your unfaithfullness demands "
                                     "punishment!\n\r", ch);
                        drop_level(ch, CLASS_DRUID, FALSE);
                    }
                }
            }
        }

    }

    if (HasClass(ch, CLASS_PALADIN) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        diff = GET_ALIGNMENT(ch);
        d2 = (GET_ALIGNMENT(ch) - change);
        if (diff < 0) {
            diff = -diff;
        }
        if (d2 < 0) {
            d2 = -d2;
        }
        if (d2 < diff) {
            send_to_char("Beware, you are straying from the path\n\r", ch);
            if (d2 < 950) {
                send_to_char("Your lack of faith is disturbing\n\r", ch);
                if (d2 < 550) {
                    send_to_char ("If you do not mend your ways soon, you "
                                  "will be punished\n\r", ch);
                    if (d2 < 350) {
                        send_to_char("Your unfaithfullness demands "
                                     "punishment!\n\r", ch);
                        drop_level(ch, CLASS_PALADIN, FALSE);
                    }
                }
            }
        }
    }

    if (HasClass(ch, CLASS_RANGER) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        diff = GET_ALIGNMENT(ch);
        d2 = (GET_ALIGNMENT(ch) - change);
        if (diff < 0) {
            diff = -diff;
        }
        if (d2 < 0) {
            d2 = -d2;
        }
        if (d2 < diff) {
            send_to_char("Beware, you are straying from the path\n\r", ch);
            if (d2 < 500) {
                send_to_char("Your lack of faith is disturbing\n\r", ch);
                if (d2 < 0) {
                    send_to_char("If you do not mend your ways soon, you "
                                 "will be punished\n\r", ch);
                    if (d2 < -350) {
                        send_to_char("Your unfaithfullness demands "
                                     "punishment!\n\r", ch);
                        drop_level(ch, CLASS_RANGER, FALSE);
                    }
                }
            }
        }
    }

    GET_ALIGNMENT(ch) -= change;

    if (HasClass(ch, CLASS_DRUID) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        if (GET_ALIGNMENT(ch) > 350 || GET_ALIGNMENT(ch) < -350) {
            send_to_char("The Patron of Druids and Rangers, has "
                         "excommunicated you for your heresies.\n\r", ch);
            send_to_char("You are forever more a mere cleric!\n\r", ch);
            REMOVE_BIT(ch->player.class, CLASS_DRUID);
            if (!HasClass(ch, CLASS_CLERIC)) {
                GET_LEVEL(ch, CLERIC_LEVEL_IND) =
                    GET_LEVEL(ch, DRUID_LEVEL_IND);
            }
            GET_LEVEL(ch, DRUID_LEVEL_IND) = 0;
            SET_BIT(ch->player.class, CLASS_CLERIC);

            /*
             * Used for the Dr/Ra Multiclass
             */
            if (HasClass(ch, CLASS_RANGER)
                && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
                send_to_char("The Patron of Rangers has excommunicated "
                             "you for your heresies.\n\r", ch);
                send_to_char("You are forever more a mere warrior!\n\r", ch);
                REMOVE_BIT(ch->player.class, CLASS_RANGER);
                if (!HasClass(ch, CLASS_WARRIOR)) {
                    GET_LEVEL(ch, WARRIOR_LEVEL_IND) =
                        GET_LEVEL(ch, RANGER_LEVEL_IND);
                }
                GET_LEVEL(ch, RANGER_LEVEL_IND) = 0;
                SET_BIT(ch->player.class, CLASS_WARRIOR);
            }
        }
    }

    if (HasClass(ch, CLASS_PALADIN) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        if (GET_ALIGNMENT(ch) < 350) {
            send_to_char("The Patron of Paladins has excommunicated you "
                         "for your heresies.\n\r", ch);
            send_to_char("You are forever more a mere warrior!\n\r", ch);
            REMOVE_BIT(ch->player.class, CLASS_PALADIN);
            if (!HasClass(ch, CLASS_WARRIOR)) {
                GET_LEVEL(ch, WARRIOR_LEVEL_IND) =
                    GET_LEVEL(ch, PALADIN_LEVEL_IND);
            }
            GET_LEVEL(ch, PALADIN_LEVEL_IND) = 0;
            SET_BIT(ch->player.class, CLASS_WARRIOR);
        }
    }

    if (HasClass(ch, CLASS_RANGER) && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
        if (GET_ALIGNMENT(ch) < -350) {
            send_to_char("The Patron of Rangers has excommunicated "
                         "you for your heresies\n\r", ch);
            send_to_char("You are forever more a mere warrior!\n\r", ch);
            REMOVE_BIT(ch->player.class, CLASS_RANGER);
            if (!HasClass(ch, CLASS_WARRIOR)) {
                GET_LEVEL(ch, WARRIOR_LEVEL_IND) =
                    GET_LEVEL(ch, RANGER_LEVEL_IND);
            }
            GET_LEVEL(ch, RANGER_LEVEL_IND) = 0;
            SET_BIT(ch->player.class, CLASS_WARRIOR);
        }
    }

    GET_ALIGNMENT(ch) = MAX(GET_ALIGNMENT(ch), -1000);
    GET_ALIGNMENT(ch) = MIN(GET_ALIGNMENT(ch), 1000);
}

void death_cry(struct char_data *ch)
{
    int             door,
                    was_in;

    if (ch->in_room == -1) {
        return;
    }
    act("$c0005Your blood freezes as you hear $c0015$n's$c0005 death cry.",
        FALSE, ch, 0, 0, TO_ROOM);
    was_in = ch->in_room;

    for (door = 0; door <= 5; door++) {
        if (CAN_GO(ch, door)) {
            ch->in_room = (real_roomp(was_in))->dir_option[door]->to_room;
            act("$c0005Your blood freezes as you hear someones death cry.",
                FALSE, ch, 0, 0, TO_ROOM);
            ch->in_room = was_in;
        }
    }
}

void raw_kill(struct char_data *ch, int killedbytype)
{
#if 0
    struct char_data *tmp,
                   *tch;
    char            buf[256];
#endif

    if (IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        raw_kill_arena(ch);
        return;
    }

    if (IS_MOB(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF) &&
            mob_index[ch->nr].func) {
        (*mob_index[ch->nr].func) (ch, 0, "", ch, EVENT_DEATH);
    }


    /*
     * tell mob to hate killer next load here, or near here
     */

    if (ch->specials.fighting) {
        stop_fighting(ch);
    }
    death_cry(ch);

    /*
     * give them some food and water so they don't whine.
     */
    if (GetMaxLevel(ch) < LOW_IMMORTAL) {
        GET_COND(ch, THIRST) = 20;
        GET_COND(ch, FULL) = 20;
    }

    /*
     * remove berserk after they flee/die...
     */

    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
        REMOVE_BIT(ch->specials.affected_by2, AFF2_BERSERK);
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_STYLE_BERSERK)) {
        REMOVE_BIT(ch->specials.affected_by2, AFF2_STYLE_BERSERK);
    }

    /*
     *   return them from polymorph
     */

    make_corpse(ch, killedbytype);
    zero_rent(ch);
    extract_char(ch);
}

void die(struct char_data *ch, int killedbytype)
{
    struct char_data *pers;
    int             i,
                    tmp;
    char            buf[80];
    int             fraction;

    if (ValidRoom(ch) == FALSE) {
        return;

    }
    if (!IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        /*
         * This  crash??
         * need at least 1/fraction worth of exp for the minimum needed for
         * the pc's current level, or else you lose a level.  If all three
         * classes are lacking in exp, you lose one level in each class.
         */

        fraction = 16;
        if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
            /*
             *   take char from storage, to room
             */
            if (ch->desc) {
                pers = ch->desc->original;
                char_from_room(pers);
                char_to_room(pers, ch->in_room);
                SwitchStuff(ch, pers);
                extract_char(ch);
                ch = pers;
            } else {
                /*
                 * we don't know who the original is.  Gets away with it,
                 * i guess
                 */
            }
        }
#if LEVEL_LOSS

        for (i = 0; i < MAX_CLASS; i++) {
            if (GET_LEVEL(ch, i) > 1) {
                if (GET_LEVEL(ch, i) >= LOW_IMMORTAL) {
                    break;
                }
                if (GET_EXP(ch) <
                    (titles[i][(int) GET_LEVEL(ch, i)].exp / fraction)) {
                    tmp = (ch->points.max_hit) / GetMaxLevel(ch);
                    ch->points.max_hit -= tmp;
                    GET_LEVEL(ch, i) -= 1;
                    ch->specials.spells_to_learn -=
                        MAX(1, MAX(2, wis_app[(int)GET_RWIS(ch)].bonus) /
                                      HowManyClasses(ch));
                    send_to_char("\n\rInsufficient experience has cost you a"
                                 " level.\n\r", ch);
                }
            }
        }
#endif
#if 0
        if (GetMaxLevel(ch) > 20) {
            gain_exp(ch, -GET_EXP(ch)*0.25);
        } else if (GetMaxLevel(ch) > 15) {
            gain_exp(ch, -GET_EXP(ch)*0.20);
        } else if (GetMaxLevel(ch) > 5) {
            gain_exp(ch, -GET_EXP(ch)*0.15);
        } else {
            gain_exp(ch, -GET_EXP(ch)*0.10);
        }
#endif
        gain_exp(ch, -GET_EXP(ch) * (GetMaxLevel(ch) / 100.0));

        GET_LEADERSHIP_EXP(ch) -= GET_LEADERSHIP_EXP(ch) / 2;

#if LEVEL_LOSS

        /*
         * warn people if their next death will result in a level loss
         */
        for (i = 0; i < MAX_CLASS; i++) {
            if (GET_LEVEL(ch, i) > 1 &&
                GET_EXP(ch) <
                    (titles[i][(int) GET_LEVEL(ch, i)].exp / fraction)) {
                send_to_char("\n\r\n\rWARNING WARNING WARNING WARNING "
                             "WARNING WARNING\n\r", ch);
                send_to_char("Your next death will result in the loss of a"
                             " level,\n\r", ch);
                sprintf(buf, "unless you get at least %ld more exp points.\n\r",
                        (titles[i][(int) GET_LEVEL(ch, i)].exp /
                         fraction) - GET_EXP(ch));
                send_to_char(buf, ch);
            }
        }
#endif

        /*
         **      Set the talk[2] to be TRUE, i.e. DEAD
         */
        ch->player.talks[2] = 1;
    }
    DeleteHatreds(ch);
    DeleteFears(ch);
    raw_kill(ch, killedbytype);
}

long ExpCaps(int group_count, long total)
{

    if (group_count >= 1) {
        switch (group_count) {
        case 1:
            if (total > EXP_CAP_1) {
                total = EXP_CAP_1;
            }
            break;
        case 2:
            if (total > EXP_CAP_2) {
                total = EXP_CAP_2;
            }
            break;
        case 3:
            if (total > EXP_CAP_3) {
                total = EXP_CAP_3;
            }
            break;
        case 4:
            if (total > EXP_CAP_4) {
                total = EXP_CAP_4;
            }
            break;
        case 5:
            if (total > EXP_CAP_5) {
                total = EXP_CAP_5;
            }
            break;
        case 6:
            if (total > EXP_CAP_6) {
                total = EXP_CAP_6;
            }
            break;
        case 7:
            if (total > EXP_CAP_7) {
                total = EXP_CAP_7;
            }
            break;
        case 8:
            if (total > EXP_CAP_8) {
                total = EXP_CAP_8;
            }
            break;
        default:
            if (total > EXP_CAP_OTHER) {
                total = EXP_CAP_OTHER;
            }
            break;
        }
    } else if (total > EXP_CAP) {
        /*
         * not grouped, so limit max exp gained so
         * grouping will be used more and benifical
         */
        total = EXP_CAP;
    }

    return (total);
}

long GroupLevelRatioExp(struct char_data *ch, int group_max_level,
                        long experincepoints)
{
    unsigned int    diff = 0;

    diff = abs(group_max_level - GetMaxLevel(ch));

    if (diff) {
        /*
         * More than 10 levels difference, then we knock down
         * the ratio of EXP he gets, keeping high level people
         * from getting newbies up to fast...
         */
        if (diff >= 10) {
            experincepoints = experincepoints / 2;
        } else if (diff >= 20) {
            experincepoints = experincepoints / 3;
        } else if (diff >= 30) {
            experincepoints = experincepoints / 4;
        } else if (diff >= 40) {
            experincepoints = experincepoints / 5;
        } else if (diff >= 49) {
            experincepoints = experincepoints / 6;
        }
    }

    return (experincepoints);
}


long NewExpCap(struct char_data *ch, long total)
{
    long            temp = 0,
                    temp2 = 0;
    int             x;

    if( !ch || IS_IMMORTAL(ch) ) {
        return( 0 );
    }

    for (x = 0; x < MAX_CLASS; x++) {
        if (GET_LEVEL(ch, x)) {
            temp += titles[x][(int)GET_LEVEL(ch, x) + 1].exp -
                    titles[x][(int)GET_LEVEL(ch, x)].exp;
        }
    }

    temp2 = temp * 0.10;
    if (total > temp2) {
#if 0
        sprintf(buf,"Capping at 10Percent: %dXXX%d",temp, temp2);
        send_to_char(buf,ch);
#endif
        return (temp * 0.10);
    } else {
#if 0
        sprintf(buf,"Using actual XP %d",total);
        send_to_char(buf,ch);
#endif
        return total;
    }
}

void group_gain(struct char_data *ch, struct char_data *victim)
{
    char            buf[256];
    int             no_members,
                    share;
    struct char_data *k;
    struct follow_type *f;
    int             total,
                    pc,
                    group_count = 0,
                    group_max_level = 1;

    if (!(k = ch->master)) {
        k = ch;
    }

    /*
     * can't get exp for killing players
     */

    if (!IS_NPC(victim)) {
        return;
    }

    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
        no_members = GET_AVE_LEVEL(k);
    } else {
        no_members = 0;
    }

    pc = FALSE;

    group_max_level = GetMaxLevel(k);

    for (f = k->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP) &&
            f->follower->in_room == ch->in_room) {
            no_members += GET_AVE_LEVEL(f->follower);
            if (IS_PC(f->follower)) {
                pc++;
            }
            if (IS_PC(f->follower) ||
                (IS_SET(f->follower->specials.act, ACT_POLYSELF) &&
                 f->follower->in_room == k->in_room)) {
                if (group_max_level < GetMaxLevel(f->follower)) {
                    group_max_level = GetMaxLevel(f->follower);
                }
                group_count++;
            }
        }
    }

    if (pc > 10) {
        pc = 10;
    }
    if (no_members >= 1) {
        share = (GET_EXP(victim) / no_members);
    } else {
        share = 0;
    }
    if (IS_AFFECTED(k, AFF_GROUP) && (k->in_room == ch->in_room)) {
        total = share * GET_AVE_LEVEL(k);
        if (pc) {
            total *= (100 + (3 * pc));
            total /= 100;
        }

        RatioExp(k, victim, total);
        total = GroupLevelRatioExp(k, group_max_level, total);
        total = ExpCaps(group_count, total);

        if (!IS_IMMORTAL(k)) {
            total = NewExpCap(k, total);
        }
        if (!ch->master && ch->followers) {
            GET_LEADERSHIP_EXP(ch) += total;
            ch_printf(ch, "Your leadership skills have served you well.\n\r",
                      total);
        } else if (!ch->master && !ch->followers) {
            GET_LEADERSHIP_EXP(ch) += total * 3 / 5;
            ch_printf(ch, "Your leadership skills have served you well.\n\r",
                      total * 3 / 5);
        }

        sprintf(buf, "You receive your share of %d experience.", total);
        act(buf, FALSE, k, 0, 0, TO_CHAR);
        gain_exp(k, total);
        change_alignment(k, victim);
    }

    for (f = k->followers; f; f = f->next) {
        if (IS_AFFECTED(f->follower, AFF_GROUP) &&
            f->follower->in_room == ch->in_room) {
            total = share * GET_AVE_LEVEL(f->follower);
            if (IS_PC(f->follower)) {
                total *= (100 + (1 * pc));
                total /= 100;
            } else {
                total /= 2;
            }
            if (IS_PC(f->follower)) {
                total = RatioExp(f->follower, victim, total);
                total = GroupLevelRatioExp(f->follower, group_max_level, total);
                total = ExpCaps(group_count, total);

                if (!IS_IMMORTAL(f->follower)) {
                    total = NewExpCap(f->follower, total);
                }
                if (!ch->master && ch->followers) {
                    GET_LEADERSHIP_EXP(ch) += total;
                    ch_printf(ch, "Your leadership skills have served you "
                                  "well.\n\r", total);
                } else if (!ch->master && !ch->followers) {
                    GET_LEADERSHIP_EXP(ch) += total * 3 / 5;
                    ch_printf(ch, "Your leadership skills have served you "
                                  "well.\n\r", total * 3 / 5);
                }

                sprintf(buf, "You receive your share of %d experience.", total);
                act(buf, FALSE, f->follower, 0, 0, TO_CHAR);
                gain_exp(f->follower, total);
                change_alignment(f->follower, victim);
            } else {
                if (f->follower->master &&
                    IS_AFFECTED(f->follower, AFF_CHARM)) {
                    total = RatioExp(f->follower->master, victim, total);
                    total = GroupLevelRatioExp(f->follower, group_max_level,
                                               total);
                    total = ExpCaps(group_count, total);

                    if (!IS_IMMORTAL(f->follower->master)) {
                        total = NewExpCap(f->follower->master, total);
                    }
                    if (f->follower->master->in_room == f->follower->in_room) {
                        sprintf(buf, "You receive $N's share of %d experience.",
                                total);
                        act(buf, FALSE, f->follower->master, 0, f->follower,
                            TO_CHAR);
                        gain_exp(f->follower->master, total);
                        change_alignment(f->follower, victim);
                    }
                } else {
                    total = RatioExp(f->follower, victim, total);
                    total = GroupLevelRatioExp(f->follower, group_max_level,
                                               total);
                    total = ExpCaps(group_count, total);

                    if (!IS_IMMORTAL(f->follower)) {
                        total = NewExpCap(f->follower, total);
                    }
                    if (!ch->master && ch->followers) {
                        GET_LEADERSHIP_EXP(ch) += total;
                        ch_printf(ch, "Your leadership skills have served you "
                                      "well.\n\r", total);
                    } else if (!ch->master && !ch->followers) {
                        GET_LEADERSHIP_EXP(ch) += total * 3 / 5;
                        ch_printf(ch, "Your leadership skills have served you "
                                      "well.\n\r", total * 3 / 5);
                    }
                    sprintf(buf, "You receive your share of %d experience.",
                            total);
                    act(buf, FALSE, f->follower, 0, 0, TO_CHAR);
                    gain_exp(f->follower, total);

                    change_alignment(f->follower, victim);
                }
            }
        }
    }
}

char           *replace_string(char *str, char *weapon, char *weapon_s,
                               char *location_hit, char *location_hit_s)
{
    static char     buf[256];
    char           *cp;

    cp = buf;

    for (; *str; str++) {
        if (*str == '#') {
            switch (*(++str)) {
            case 'W':
                for (; *weapon; *(cp++) = *(weapon++)) {
                    /*
                     * Empty loop
                     */
                }
                break;
            case 'w':
                for (; *weapon_s; *(cp++) = *(weapon_s++)) {
                    /*
                     * Empty loop
                     */
                }
                break;

                /*
                 * added this to show where the person was hit
                 */
            case 'L':
                for (; *location_hit; *(cp++) = *(location_hit++)) {
                    /*
                     * Empty loop
                     */
                }
                break;
            case 'l':
                for (; *location_hit_s; *(cp++) = *(location_hit_s++)) {
                    /*
                     * Empty loop
                     */
                }
                break;

            default:
                *(cp++) = '#';
                break;
            }
        } else {
            *(cp++) = *str;
        }

        *cp = 0;
    }

    return (buf);
}

void dam_message(int dam, struct char_data *ch, struct char_data *victim,
                 int w_type)
{
    struct obj_data *wield;
    char           *buf;
    char            buf2[MAX_STRING_LENGTH];

    int             snum,
                    hitloc;

   struct dam_weapon_type {
        char           *to_room;
        char           *to_char;
        char           *to_victim;
    };
    static struct dam_weapon_type dam_weapons[] = {
        {"$n misses $N.", "You miss $N.", "$n misses you."}, /* 0 */
        {"$n bruises $N with $s #w #l.", "You bruise $N as you #w $M #l.",
         "$n bruises you as $e #W your #L."}, /* 1 .. 2 */
        {"$n barely #W $N #l.", "You barely #w $N #l.",
         "$n barely #W your #L."}, /* 3.. 4 */
        {"$n #W $N #l.", "You #w $N #l.", "$n #W your #L."}, /* 5.. 6 */
        {"$n #W $N hard #l.", "You #w $N hard #l.",
         "$n #W you hard on your #L."}, /* 7..10 */
        {"$n #W $N very hard #l.", "You #w $N very hard #l.",
         "$n #W you very hard on your #L."}, /* 11..14 */
        {"$n #W $N $c0011extremely well$c0007 #l.",
         "You #w $N $c0011extremely well$c0007 #l.",
         "$n #W you $c0011extremely well$c0007 on your #L."}, /* 15..20 */
        {"$n $c0010massacres$c0007 $N with $s #w #l.",
         "You$c0010 massacre$c0007 $N with your #w #l.",
         "$n $c0010massacres$c0007 you with $s #w on your #L."}, /* > 20 */
        {"$n $c0009devastates$c0007 $N with $s #w #l.",
         "You $c0009devastate$c0007 $N with your #w #l.",
         "$n $c0009devastates$c0007 you with $s #w on your #L."},
        {"$n $c0012decimates$c0007 $N with $s #w #l.",
         "You $c0012decimate$c0007 $N with your #w #l.",
         "$n $c0012decimates$c0007 you with $s #w on your #L."},
        {"$n $c0008annihilates$c0007 $N with $s #w #l.",
         "You $c0008annihilate$c0007 $N with your #w #l.",
         "$n $c0008annihilates$c0007 you with $s #w on your #L."},
        {"dodge - please report seeing this",
         "dodge - please report seeing this",
         "dodge - please report seeing this."},
        { "shield block - please report seeing this",
          "shield block - please report seeing this",
          "shield block - please report seeing this" }
    };

    /*
     * Change to base of table with text
     */
    w_type -= TYPE_HIT;

    wield = ch->equipment[WIELD];

    if (dam == -3) {
        snum = 11;
    } else if (dam == -2) {
        snum = 12;
    } else if (dam <= 0) {
        snum = 0;
    } else if (dam <= 2) {
        snum = 1;
    } else if (dam <= 4) {
        snum = 2;
    } else if (dam <= 10) {
        snum = 3;
    } else if (dam <= 15) {
        snum = 4;
    } else if (dam <= 25) {
        snum = 5;
    } else if (dam <= 35) {
        snum = 6;
    } else if (dam <= 45) {
        snum = 7;
    } else if (dam <= 55) {
        snum = 8;
    } else if (dam <= 65) {
        snum = 9;
    } else {
        snum = 10;
    }

    /*
     * generate random hit location
     */
    hitloc = number(0, 14);

    /*
     * make body/chest hits happen more often than the others
     */
    if (hitloc != 0 && hitloc != 11 && hitloc != 13) {
        hitloc = number(0, 14);
    }
    /*
     * make sure the mob has this body part first!
     */
    if (!HasHands(victim)) {
        /* if not then just make it a body hit hitloc=0 */
        hitloc = 0;
    }

    buf = replace_string(dam_weapons[snum].to_room,
                         attack_hit_text[w_type].plural,
                         attack_hit_text[w_type].singular,
                         location_hit_text[hitloc].plural,
                         location_hit_text[hitloc].singular);
    act(buf, FALSE, ch, wield, victim, TO_NOTVICT);

    buf = replace_string(dam_weapons[snum].to_char,
                         attack_hit_text[w_type].plural,
                         attack_hit_text[w_type].singular,
                         location_hit_text[hitloc].plural,
                         location_hit_text[hitloc].singular);

    /*
     * @Desc Says how much damage your hitting after 200milxp @Author Greg
     * Hovey (GH) @Date April 2002
     */
    if (GET_EXP(ch) > 200000000 || IS_IMMORTAL(ch) ||
        IS_SET(ch->specials.act, PLR_LEGEND)) {
        sprintf(buf2, "%s $c0011($c0015%d$c0011)$c0007", buf, dam);
        act(buf2, FALSE, ch, wield, victim, TO_CHAR);
    } else {
        act(buf, FALSE, ch, wield, victim, TO_CHAR);
    }

    buf = replace_string(dam_weapons[snum].to_victim,
                         attack_hit_text[w_type].plural,
                         attack_hit_text[w_type].singular,
                         location_hit_text[hitloc].plural,
                         location_hit_text[hitloc].singular);
    act(buf, FALSE, ch, wield, victim, TO_VICT);
}

int DamCheckDeny(struct char_data *ch, struct char_data *victim, int type)
{
    struct room_data *rp;
    char            buf[MAX_INPUT_LENGTH + 20];

    /*
     * assert(GET_POS(victim) > POSITION_DEAD);
     */

    if (!GET_POS(victim) > POSITION_DEAD) {
        Log("!GET_POS(victim) > POSITION_DEAD in fight.c");
        return (TRUE);
    }

    rp = real_roomp(ch->in_room);
    if (rp && (rp->room_flags & PEACEFUL) && type != SPELL_POISON &&
        type != SPELL_DISEASE && type != SPELL_DECAY &&
        type != SPELL_HEAT_STUFF && type != TYPE_SUFFERING) {
        sprintf(buf, "damage(,,,%d) called in PEACEFUL room", type);
        Log(buf);
        return (TRUE);
        /*
         * true, they are denied from fighting
         */
    }
    return (FALSE);
}

int DamDetailsOk(struct char_data *ch, struct char_data *v, int dam, int type)
{
    if (dam < 0) {
        return (FALSE);
    }

    /*
     * we check this already I think, be sure to keep an eye out. msw
     */
    if ((type != TYPE_RANGE_WEAPON) && (ch->in_room != v->in_room)) {
        return (FALSE);
    }
    if (ch == v && type != SPELL_POISON && type != SPELL_HEAT_STUFF &&
        type != SPELL_DISEASE && type != SPELL_DECAY &&
        type != TYPE_SUFFERING) {
        return (FALSE);
    }

    if (MOUNTED(ch) && MOUNTED(ch) == v) {
        FallOffMount(ch, v);
        Dismount(ch, MOUNTED(ch), POSITION_SITTING);
    }

    return (TRUE);
}

int SetCharFighting(struct char_data *ch, struct char_data *v)
{
    if (GET_POS(ch) > POSITION_STUNNED) {
        if (!(ch->specials.fighting)) {
            set_fighting(ch, v);
            GET_POS(ch) = POSITION_FIGHTING;
            if (!(ch->specials.fighting)) {
                GET_POS(ch) = POSITION_STANDING;
            }
        } else {
            return (FALSE);
        }
    }
    return (TRUE);
}

int SetVictFighting(struct char_data *ch, struct char_data *v)
{
    if (v != ch && GET_POS(v) > POSITION_STUNNED && !(v->specials.fighting)) {
        if (ch->attackers < 6) {
            set_fighting(v, ch);
            GET_POS(v) = POSITION_FIGHTING;
        }
    } else {
        return (FALSE);
    }
    return (TRUE);
}

void WeaponSkillCheck(struct char_data *ch)
{
    struct obj_data *obj;
    int             weapontype = 0;
    int             found = 0;
    int             maxpoints = 0;
    int             totpoints = 0;

    int             fighter = 0;
    int             specialist = 0;
    int             lowest = 100;
    char            buf[256];

    if (!(obj = ch->equipment[WIELD])) {
        return;
    }
    if (!IS_WEAPON(obj)) {
        return;
    }
    /*
     * 350 - 409
     */
    weapontype = obj->weapontype + WEAPON_FIRST;

    if (ch->weaponskills.slot1 == weapontype) {
        found = 1;
    } else if (ch->weaponskills.slot2 == weapontype){
        found = 2;
    } else if (ch->weaponskills.slot3 == weapontype) {
        found = 3;
    } else if (ch->weaponskills.slot4 == weapontype) {
        found = 4;
    } else if (ch->weaponskills.slot5 == weapontype) {
        found = 5;
    } else if (ch->weaponskills.slot6 == weapontype) {
        found = 6;
    } else if (ch->weaponskills.slot7 == weapontype) {
        found = 7;
    } else if (ch->weaponskills.slot8 == weapontype) {
        found = 8;
    }

    if (ch->specials.remortclass == WARRIOR_LEVEL_IND + 1) {
        specialist = 1;
        fighter = 1;
        maxpoints = 400;
    } else if (HasClass(ch, CLASS_WARRIOR) || HasClass(ch, CLASS_BARBARIAN) ||
               HasClass(ch, CLASS_PALADIN) || HasClass(ch, CLASS_RANGER)) {
        fighter = 1;
        maxpoints = 200;
    } else {
        maxpoints = 100;
    }

    if (!found) {
        /*
         * assign the lowest available slot to this type
         */
        if (lowest > ch->weaponskills.grade1) {
            lowest = ch->weaponskills.grade1;
        }
        if (lowest > ch->weaponskills.grade2) {
            lowest = ch->weaponskills.grade2;
        }
        if (lowest > ch->weaponskills.grade3) {
            lowest = ch->weaponskills.grade3;
        }
        if (lowest > ch->weaponskills.grade4 && fighter) {
            lowest = ch->weaponskills.grade4;
        }
        if (lowest > ch->weaponskills.grade5 && fighter) {
            lowest = ch->weaponskills.grade5;
        }
        if (lowest > ch->weaponskills.grade6 && specialist) {
            lowest = ch->weaponskills.grade6;
        }
        if (lowest > ch->weaponskills.grade7 && specialist) {
            lowest = ch->weaponskills.grade7;
        }
        if (lowest > ch->weaponskills.grade8 && specialist) {
            lowest = ch->weaponskills.grade8;
        }

        if (lowest == ch->weaponskills.grade1) {
            ch->weaponskills.grade1 = 1;
            ch->weaponskills.slot1 = weapontype;
        } else if (lowest == ch->weaponskills.grade2) {
            ch->weaponskills.grade2 = 1;
            ch->weaponskills.slot2 = weapontype;
        } else if (lowest == ch->weaponskills.grade3) {
            ch->weaponskills.grade3 = 1;
            ch->weaponskills.slot3 = weapontype;
        } else if (lowest == ch->weaponskills.grade4) {
            ch->weaponskills.grade4 = 1;
            ch->weaponskills.slot4 = weapontype;
        } else if (lowest == ch->weaponskills.grade5) {
            ch->weaponskills.grade5 = 1;
            ch->weaponskills.slot5 = weapontype;
        } else if (lowest == ch->weaponskills.grade6) {
            ch->weaponskills.grade6 = 1;
            ch->weaponskills.slot6 = weapontype;
        } else if (lowest == ch->weaponskills.grade7) {
            ch->weaponskills.grade7 = 1;
            ch->weaponskills.slot7 = weapontype;
        } else if (lowest == ch->weaponskills.grade8) {
            ch->weaponskills.grade8 = 1;
            ch->weaponskills.slot8 = weapontype;
        } else {
            Log("got to bad spot in WeaponSkillCheck");
            return;
        }

        sprintf(buf, "You've decided to get in some practice with the %s.\n\r",
                weaponskills[weapontype - WEAPON_FIRST].name);
        send_to_char(buf, ch);
    } else {
        /*
         * 3% chance of skill increase
         */
        if (number(1, 100) < 4) {
            switch (found) {
            case 1:
                if (ch->weaponskills.grade1 < 100) {
                    ch->weaponskills.grade1++;
                }
                break;
            case 2:
                if (ch->weaponskills.grade2 < 100) {
                    ch->weaponskills.grade2++;
                }
                break;
            case 3:
                if (ch->weaponskills.grade3 < 100) {
                    ch->weaponskills.grade3++;
                }
                break;
            case 4:
                if (ch->weaponskills.grade4 < 100) {
                    ch->weaponskills.grade4++;
                }
                break;
            case 5:
                if (ch->weaponskills.grade5 < 100) {
                    ch->weaponskills.grade5++;
                }
                break;
            case 6:
                if (ch->weaponskills.grade6 < 100) {
                    ch->weaponskills.grade6++;
                }
                break;
            case 7:
                if (ch->weaponskills.grade7 < 100) {
                    ch->weaponskills.grade7++;
                }
                break;
            case 8:
                if (ch->weaponskills.grade8 < 100) {
                    ch->weaponskills.grade8++;
                }
                break;
            default:
                Log("odd spot in weapon increase");
                break;
            }
            send_to_char("Practice makes perfect!\n\r", ch);

            /*
             * now check for total skill points
             */
            totpoints = ch->weaponskills.grade1 + ch->weaponskills.grade2 +
                        ch->weaponskills.grade3 + ch->weaponskills.grade4 +
                        ch->weaponskills.grade5 + ch->weaponskills.grade6 +
                        ch->weaponskills.grade7 + ch->weaponskills.grade8;

            if (totpoints > maxpoints) {
                /*
                 * let's lower all the others a point
                 */
                if (found != 1 && ch->weaponskills.grade1 > 0) {
                    ch->weaponskills.grade1--;
                }
                if (found != 2 && ch->weaponskills.grade2 > 0) {
                    ch->weaponskills.grade2--;
                }
                if (found != 3 && ch->weaponskills.grade3 > 0) {
                    ch->weaponskills.grade3--;
                }
                if (found != 4 && ch->weaponskills.grade4 > 0 && fighter) {
                    ch->weaponskills.grade4--;
                }
                if (found != 5 && ch->weaponskills.grade5 > 0 && fighter) {
                    ch->weaponskills.grade5--;
                }
                if (found != 6 && ch->weaponskills.grade6 > 0 && specialist) {
                    ch->weaponskills.grade6--;
                }
                if (found != 7 && ch->weaponskills.grade7 > 0 && specialist) {
                    ch->weaponskills.grade7--;
                }
                if (found != 8 && ch->weaponskills.grade8 > 0 && specialist) {
                    ch->weaponskills.grade8--;
                }
            }
        }
    }
}

int ClassDamBonus(struct char_data *ch, struct char_data *v, int dam)
{
    if (ch->specials.remortclass == RANGER_LEVEL_IND + 1) {
        if (IsUndead(v)) {
            dam *= 1.1;
        }
        if (IS_EVIL(v)) {
            dam *= 1.05;
        }
    } else if (ch->specials.remortclass == PALADIN_LEVEL_IND + 1) {
        if (IsUndead(v)) {
            dam *= 1.1;
        }
        if (IS_EVIL(v)) {
            dam *= 1.2;
        } else if (IS_GOOD(v)) {
            dam *= 0.8;
        }
    } else if (ch->specials.remortclass == NECROMANCER_LEVEL_IND + 1) {
        if (IsUndead(v)) {
            dam *= 0.9;
        }
        if (IS_NEUTRAL(v)) {
            dam *= 1.05;
        } else if (IS_GOOD(v)) {
            dam *= 1.15;
        } else if (IS_EVIL(v)) {
            dam *= 0.8;
        }
    }

    return (dam);
}

int DamageTrivia(struct char_data *ch, struct char_data *v, int dam, int type)
{
    char            buf[255];
    struct affected_type *aff;
    int             right_protection = FALSE;
    int             index;

    if (v->master == ch) {
        stop_follower(v);
    }
    if (IS_AFFECTED(ch, AFF_INVISIBLE) ||
        IS_AFFECTED2(ch, AFF2_ANIMAL_INVIS) ||
        IS_AFFECTED2(ch, AFF2_ANIMAL_INVIS)) {
        appear(ch);
    }

    if (IS_AFFECTED(ch, AFF_SNEAK)) {
        affect_from_char(ch, SKILL_SNEAK);
    }

    if (IS_AFFECTED(ch, AFF_HIDE)) {
        REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
    }

#if PREVENT_PKILL
    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        (IS_PC(v) || IS_SET(v->specials.act, ACT_POLYSELF)) &&
        (ch != v) && !CanFightEachOther(ch, v)) {
        act("Your attack seems usless against $N!", FALSE, ch, 0, v, TO_CHAR);
        act("The attack from $n is futile!", FALSE, ch, 0, v, TO_VICT);
        dam = -1;
    }
#endif

    if (affected_by_spell(v, SPELL_MANA_SHIELD) && dam > 0) {
        while (GET_MANA(v) && dam) {
            GET_MANA(v) -= 1;
            dam -= 1;
        }
        if (dam && !GET_MANA(v)) {
            act("Your mana shield absorbs part of the blow, and is fully "
                "drained.", FALSE, ch, 0, v, TO_VICT);
            act("$N's mana shield absorbs part of your blow, and is fully "
                "drained.", FALSE, ch, 0, v, TO_CHAR);
            act("$N's mana shield absorbs part of the blow, and is fully "
                "drained.", FALSE, ch, 0, v, TO_NOTVICT);
            affect_from_char(v, SPELL_MANA_SHIELD);
        } else if (!dam && !GET_MANA(v)) {
            act("Your mana shield absorbs the blow, but is fully drained.",
                FALSE, ch, 0, v, TO_VICT);
            act("$N's mana shield absorbs the blow, but is fully drained.",
                FALSE, ch, 0, v, TO_CHAR);
            act("$N's mana shield absorbs the blow, but is fully drained.",
                FALSE, ch, 0, v, TO_NOTVICT);
            affect_from_char(v, SPELL_MANA_SHIELD);
            return (dam);
        } else {
            act("Your mana shield absorbs the blow!", FALSE, ch, 0, v,
                TO_VICT);
            act("$N's mana shield absorbs your blow!", FALSE, ch, 0, v,
                TO_CHAR);
            act("$N's mana shield absorbs $n's blow!", FALSE, ch, 0, v,
                TO_NOTVICT);
            return (dam);
        }
    }

    if (affected_by_spell(v, SPELL_IRON_SKINS) && dam > 0) {
        /*
         * locate the right affect.. is this laggy? probably.
         */
        for (aff = v->affected; aff; aff = aff->next) {
            if (aff->type == SPELL_IRON_SKINS) {
                break;
            }
        }

        if (aff->type == SPELL_IRON_SKINS) {
            while (aff->duration && dam) {
                aff->duration -= 1;
                dam -= 1;
            }
            if (dam && !aff->duration) {
                act("Your last iron skin is flayed, and $n hits you.",
                    FALSE, ch, 0, v, TO_VICT);
                act("$N's last iron skin is flayed, and you manage to hit $M.",
                    FALSE, ch, 0, v, TO_CHAR);
                act("$N's last iron skin is flayed, and $n manages to get a "
                    "hit in.", FALSE, ch, 0, v, TO_NOTVICT);
                affect_from_char(v, SPELL_IRON_SKINS);
            } else if (!dam && !aff->duration) {
                act("Your last iron skin is flayed, keeping you from harm one "
                    "last time.", FALSE, ch, 0, v, TO_VICT);
                act("$N's last iron skin is flayed, keeping $M from harm one "
                    "more time.", FALSE, ch, 0, v, TO_CHAR);
                act("$N's last iron skin is flayed, keeping $M from harm one "
                    "more time.", FALSE, ch, 0, v, TO_NOTVICT);
                affect_from_char(v, SPELL_IRON_SKINS);
                return (dam);
            } else {
                act("Some of your iron skins crumble to dust, leaving you "
                    "unharmed!", FALSE, ch, 0, v, TO_VICT);
                act("Some of $N's iron skins crumble to dust, leaving $M "
                    "unharmed!", FALSE, ch, 0, v, TO_CHAR);
                act("Some of $N's iron skins crumble to dust, leaving $M "
                    "unharmed!", FALSE, ch, 0, v, TO_NOTVICT);
                return (dam);
            }
        }
    }

    dam = ClassDamBonus(ch, v, dam);

    if (IS_AFFECTED(v, AFF_SANCTUARY)) {
        /*
         * Max 1/2 damage when sanct'd
         */
        dam = MAX((int) (dam / 2), 0);
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK) && type >= TYPE_HIT) {
        /*
         * More damage if berserked
         */
        dam = berserkdambonus(ch, dam);
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_STYLE_BERSERK) &&
        type >= TYPE_HIT) {
        dam = (int) (berserkdambonus(ch, dam) * 0.75);
    }
    dam = PreProcDam(v, type, dam);

    /*
     * shield makes you immune to magic missle!
     */

    if (affected_by_spell(v, SPELL_SHIELD) && type == SPELL_MAGIC_MISSILE) {
        act("$n's magic missle is deflected by $N's shield!", FALSE, ch, 0,
            v, TO_NOTVICT);
        act("$N's shield deflects your magic missle!", FALSE, ch, 0, v,
            TO_CHAR);
        act("Your shell deflects $n's magic missle!", FALSE, ch, 0, v,
            TO_VICT);
        dam = -1;
    }

    if (affected_by_spell(v, SKILL_TOWER_IRON_WILL) &&
        type == SKILL_PSIONIC_BLAST) {
        act("$n's psionic attack is ignored by $N!", FALSE, ch, 0, v,
            TO_NOTVICT);
        act("$N's psionic protections shield against your attack!", FALSE,
            ch, 0, v, TO_CHAR);
        act("Your psionic protections protection you against $n's attack!",
            FALSE, ch, 0, v, TO_VICT);
        dam = -1;
    }

    /*
     * we check for prot from breath weapons here
     */

    if (type >= FIRST_BREATH_WEAPON && type <= LAST_BREATH_WEAPON) {
        if (affected_by_spell(v, SPELL_PROT_DRAGON_BREATH)) {
            /*
             * immune to all breath
             */
            right_protection = TRUE;
        } else if (affected_by_spell(v, SPELL_PROT_BREATH_FIRE) &&
                   type == SPELL_FIRE_BREATH) {
            right_protection = TRUE;
        } else if (affected_by_spell(v, SPELL_PROT_BREATH_GAS) &&
                   type == SPELL_GAS_BREATH) {
            right_protection = TRUE;
        } else if (affected_by_spell(v, SPELL_PROT_BREATH_FROST) &&
                   type == SPELL_FROST_BREATH) {
            right_protection = TRUE;
        } else if (affected_by_spell(v, SPELL_PROT_BREATH_ACID) &&
                   type == SPELL_ACID_BREATH) {
            right_protection = TRUE;
        } else if (affected_by_spell(v, SPELL_PROT_BREATH_ELEC) &&
                   type == SPELL_LIGHTNING_BREATH) {
            right_protection = TRUE;
        }

        if (right_protection) {
            act("$N smiles as some of the breath is turned aside by $S "
                "protective globe!", FALSE, ch, 0, v, TO_NOTVICT);
            act("$N's protective globe deflects a bit of your breath!",
                FALSE, ch, 0, v, TO_CHAR);
            act("Your globe deflects the some of the breath weapon from $n!",
                FALSE, ch, 0, v, TO_VICT);
            dam = (int) dam / 4;
            /*
             * 1/4 half damage
             */
        }

    } else {
        index = spell_index[type];

        if (affected_by_spell(v, SPELL_ANTI_MAGIC_SHELL) &&
            IsMagicSpell(type)) {
            sprintf(buf, "$N snickers as the %s from $n fizzles on $S "
                         "anti-magic globe!", spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_NOTVICT);
            sprintf(buf, "$N's globes deflects your %s", spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_CHAR);
            sprintf(buf, "Your globe deflects the %s from $n!",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_VICT);
            dam = -1;
        } else if (affected_by_spell(v, SPELL_GLOBE_MINOR_INV) &&
                   type < TYPE_HIT && index != -1 &&
                   spell_info[index].min_level_magic < 6) {
            /*
             * minor globe check here immune to level 1-5 and below magic
             * user spells
             */
            sprintf(buf, "$N snickers as the %s from $n fizzles on $S globe!",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_NOTVICT);
            sprintf(buf, "$N's globes deflects your %s", spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_CHAR);
            sprintf(buf, "Your globe deflects the %s from $n!",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_VICT);
            dam = -1;
        }

        /*
         * major globe immune to level 5-10 magic user spells
         */
        if (affected_by_spell(v, SPELL_GLOBE_MAJOR_INV) && type < TYPE_HIT &&
            index != -1 && spell_info[index].min_level_magic < 11 &&
            spell_info[index].min_level_magic > 5) {

            sprintf(buf, "$N laughs as the %s from $n bounces off $S globe!",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_NOTVICT);
            sprintf(buf, "$N's globes completely deflects your %s",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_CHAR);
            sprintf(buf, "Your globe completely deflects the %s from $n!",
                    spells[type - 1]);
            act(buf, FALSE, ch, 0, v, TO_VICT);
            dam = -1;
        }
    }

    if (dam > -1) {
        dam = WeaponCheck(ch, v, type, dam);
        DamageStuff(v, type, dam);
        dam = MAX(dam, 0);
        /*
         *  check if this hit will send the target over the edge to -hits
         */
        if (GET_HIT(v) - dam < 1 && IS_AFFECTED(v, AFF_LIFE_PROT)) {
            BreakLifeSaverObj(v);
            dam = 0;
            REMOVE_BIT(ch->specials.affected_by, AFF_LIFE_PROT);
        }

        if (MOUNTED(v)) {
            if (!RideCheck(v, -(dam / 2))) {
                FallOffMount(v, MOUNTED(v));
                WAIT_STATE(v, PULSE_VIOLENCE * 2);
                Dismount(v, MOUNTED(v), POSITION_SITTING);
            }
        } else if (RIDDEN(v)) {
            if (!RideCheck(RIDDEN(v), -dam)) {
                FallOffMount(RIDDEN(v), v);
                WAIT_STATE(RIDDEN(v), PULSE_VIOLENCE * 2);
                Dismount(RIDDEN(v), v, POSITION_SITTING);
            }
        }
    }

    return (dam);
}

int DoDamage(struct char_data *ch, struct char_data *v, int dam, int type)
{
    int             lev;

    if (type >= TYPE_HIT && type <= TYPE_STRIKE) {
        specdamage(ch, v);
    }
    if (dam > 0) {
        GET_HIT(v) -= dam;
        if (type >= TYPE_HIT) {
            if (IS_AFFECTED(v, AFF_FIRESHIELD) &&
                !IS_AFFECTED(ch, AFF_FIRESHIELD)) {
                if (!saves_spell(ch, SAVING_SPELL - 4)) {
                    BurnWings(ch);
                }
                lev = GetMaxLevel(v);
                dam = dice(1, 6) + (lev / 2);
                if (damage(v, ch, dam, SPELL_FIRESHIELD) &&
                    GET_POS(ch) == POSITION_DEAD) {
                    return (TRUE);
                }
            }
            if (IS_AFFECTED(v, AFF_CHILLSHIELD) &&
                !IS_AFFECTED(ch, AFF_CHILLSHIELD)) {
                lev = GetMaxLevel(v);
                dam = dice(1, 6) + (lev / 2);
                if (damage(v, ch, dam, SPELL_CHILLSHIELD) &&
                    GET_POS(ch) == POSITION_DEAD) {
                    return (TRUE);
                }
            }
        }
        update_pos(v);
    }
    return (FALSE);
}

void DamageMessages(struct char_data *ch, struct char_data *v, int dam,
                    int attacktype)
{
    int             nr,
                    max_hit,
                    i,
                    j;
    struct message_type *messages;
    char            buf[500],
                    chbuf[500],
                    victbuf[500],
                    rmbuf[500],
                    dambuf[100];

    /* filter out kicks, hard coded in do_kick */
    if (attacktype == SKILL_KICK)
        return;

    if (attacktype >= TYPE_HIT && attacktype <= TYPE_STRIKE) {
        dam_message(dam, ch, v, attacktype);
        /*
         * do not wanna frag the bow, frag the arrow instead!
         */
        if (ch->equipment[WIELD] && attacktype != TYPE_RANGE_WEAPON) {
            BrittleCheck(ch, v, dam);
        }
    } else {
        for (i = 0; i < MAX_MESSAGES; i++) {
            if (fight_messages[i].a_type == attacktype) {
                nr = dice(1, fight_messages[i].number_of_attacks);
                for (j = 1, messages = fight_messages[i].msg;
                     (j < nr) && (messages); j++) {
                    messages = messages->next;
                }
                /*
                 * fixed damage displays for spells/backstab/etc, Lennya
                 * 20030326
                 */
                if (!IS_NPC(v) && (GetMaxLevel(v) > MAX_MORT)) {
                    sprintf(chbuf, "%s", messages->god_msg.attacker_msg);
                    sprintf(victbuf, "%s", messages->god_msg.victim_msg);
                    sprintf(rmbuf, "%s", messages->god_msg.room_msg);
                } else if (dam > 0) {
                    if (GET_POS(v) == POSITION_DEAD) {
                        sprintf(chbuf, "%s", messages->die_msg.attacker_msg);
                        sprintf(victbuf, "%s", messages->die_msg.victim_msg);
                        sprintf(rmbuf, "%s", messages->die_msg.room_msg);
                    } else {
                        sprintf(chbuf, "%s", messages->hit_msg.attacker_msg);
                        sprintf(victbuf, "%s", messages->hit_msg.victim_msg);
                        sprintf(rmbuf, "%s", messages->hit_msg.room_msg);
                    }
                } else if (dam <= 0) {
                    sprintf(chbuf, "%s", messages->miss_msg.attacker_msg);
                    sprintf(victbuf, "%s", messages->miss_msg.victim_msg);
                    sprintf(rmbuf, "%s", messages->miss_msg.room_msg);
                }

                /*
                 * add the damage display for imms and legends
                 */
                if (GET_EXP(ch) > 200000000 || IS_IMMORTAL(ch) ||
                    IS_SET(ch->specials.act, PLR_LEGEND)) {
                    if (dam < 0) {
                        dam = 0;
                    }
                    sprintf(dambuf, " $c000Y($c000W%d$c000Y)$c0007", dam);
                    strcat(chbuf, dambuf);
                    sprintf(dambuf, "%s", "");
                }

                act(chbuf, FALSE, ch, ch->equipment[WIELD], v, TO_CHAR);
                act(victbuf, FALSE, ch, ch->equipment[WIELD], v, TO_VICT);
                act(rmbuf, FALSE, ch, ch->equipment[WIELD], v, TO_NOTVICT);
            }
        }
    }

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
        act("$n is stunned, but will probably regain consciousness again.",
            TRUE, v, 0, 0, TO_ROOM);
        act("You're stunned, but you will probably regain consciousness again.",
            FALSE, v, 0, 0, TO_CHAR);
        break;
    case POSITION_DEAD:
        act("$c0015$n is dead! $c0011R.I.P.", TRUE, v, 0, 0, TO_ROOM);
        act("$c0009You are dead!  Sorry...", FALSE, v, 0, 0, TO_CHAR);
#if 0
        send_to_char("$c0009You are dead! Sorry..",v);
#endif
        break;

    default:
        /*
         * >= POSITION SLEEPING
         */
        max_hit = hit_limit(v);
        if (dam > (max_hit / 5)) {
            act("That Really $c0010HURT!$c0007", FALSE, v, 0, 0, TO_CHAR);
        }
        if (GET_HIT(v) < (max_hit / (v->style == FIGHTING_STYLE_EVASIVE ?
                                     3 : 5)) && GET_HIT(v) > 0) {
            if (GET_HIT(v) < (max_hit / 5)) {
                act("You wish that your wounds would stop $c0010BLEEDING"
                    "$c0007 so much!", FALSE, v, 0, 0, TO_CHAR);
            }
            if (IS_NPC(v) && !IS_SET(v->specials.act, ACT_POLYSELF) &&
                IS_SET(v->specials.act, ACT_WIMPY)) {
                strcpy(buf, "flee");
                command_interpreter(v, buf);
            } else if (!IS_NPC(v) && IS_SET(v->specials.act, PLR_WIMPY)) {
                strcpy(buf, "flee");
                command_interpreter(v, buf);
            }
        }

        if (MOUNTED(v)) {
            /*
             * chance they fall off
             */
            RideCheck(v, -dam / 2);
        }

        if (RIDDEN(v)) {
            /*
             * chance the rider falls off
             */
            RideCheck(RIDDEN(v), -dam);
        }
        break;
    }
}

int DamageEpilog(struct char_data *ch, struct char_data *victim,
                 int killedbytype)
{
    int             exp;
    char            buf[256];
    struct room_data *rp;

    extern char     DestroyedItems;

    if (IS_LINKDEAD(victim)) {
        if (GET_POS(victim) != POSITION_DEAD) {
            do_flee(victim, "\0", 0);
            return (FALSE);
        } else {
            die(victim, killedbytype);
            return (FALSE);
        }
    }

    if (!AWAKE(victim) && victim->specials.fighting) {
        stop_fighting(victim);
    }

    if (GET_POS(victim) == POSITION_DEAD) {
        /*
         * special for no-death rooms
         */
        rp = real_roomp(victim->in_room);
        if (rp && IS_SET(rp->room_flags, NO_DEATH)) {
            GET_HIT(victim) = 1;
            GET_POS(victim) = POSITION_STANDING;
            strcpy(buf, "flee");
            command_interpreter(victim, buf);
            return (FALSE);
        }

        if (ch->specials.fighting == victim) {
            stop_fighting(ch);
        }
        if (IS_NPC(victim) && !IS_SET(victim->specials.act, ACT_POLYSELF)) {
            if (IS_AFFECTED(ch, AFF_GROUP)) {
                group_gain(ch, victim);
            } else {
                /*
                 * Calculate level-difference bonus
                 */
                exp = GET_EXP(victim);

                if (!IS_PC(victim)) {
                    exp = ExpCaps(0, exp);

                    if (!IS_IMMORTAL(ch)) {
                        exp = NewExpCap(ch, exp);
                    }
                    GET_LEADERSHIP_EXP(ch) += exp * 3 / 5;
                    ch_printf(ch, "Your leadership skills have served you "
                                  "well.\n\r", exp * 3 / 5);

                    gain_exp(ch, exp);
                    sprintf(buf, "You receive %d experience from your "
                                 "battles.\n\r", exp);
                    send_to_char(buf, ch);
                }
                change_alignment(ch, victim);
            }
        }

        if (IS_PC(victim)) {
            if (victim->in_room > -1) {
                if (IS_NPC(ch) && !IS_SET(ch->specials.act, ACT_POLYSELF)) {
                    /*
                     * killed by npc
                     */
                    if (IS_MURDER(victim)) {
#if 0
                    REMOVE_BIT(victim->player.user_flags,MURDER_1);
#endif
                    }
                    /*
                     * same here, with stole
                     */
                    if (IS_STEALER(victim)) {
                        REMOVE_BIT(victim->player.user_flags, STOLE_1);
                    }
                    if (!IS_SET(real_roomp(victim->in_room)->room_flags,
                                ARENA_ROOM)) {
                        sprintf(buf, "%s killed by %s at %s\n\r",
                                GET_NAME(victim), ch->player.short_descr,
                                (real_roomp(victim->in_room))->name);
                    } else {
                        sprintf(buf, "%s killed by %s in ARENA!\n\r",
                                GET_NAME(victim), ch->player.short_descr);
                    }
                    send_to_all(buf);
                } else {
                    /*
                     * killed by PC
                     */
                    if (!IS_PC(victim) &&
                        !IS_SET(victim->specials.act, ACT_POLYSELF)) {
                        if (victim != ch && !IS_IMMORTAL(victim)) {
#if 0
                            SET_BIT(ch->player.user_flags,MURDER_1);
#endif
                            sprintf(buf, "Setting MURDER bit on %s for "
                                         "killing %s.",
                                    GET_NAME(ch), GET_NAME(victim));
                            Log(buf);
                        }
                    }
                    if (IS_PC(ch) && IS_PC(victim) &&
                        IS_SET(real_roomp(victim->in_room)->room_flags,
                               ARENA_ROOM)) {
                        sprintf(buf, "%s killed by %s in ARENA\n\r",
                                GET_NAME(victim), GET_NAME(ch));
                        send_to_all(buf);
                    }

                    if ((IS_GOOD(ch) && !IS_EVIL(victim)) ||
                        (IS_EVIL(ch) && IS_NEUTRAL(victim))) {
                        sprintf(buf, "%s killed by %s at %s -- <Player kill,"
                                     " Illegal>",
                                GET_NAME(victim), ch->player.name,
                                (real_roomp(victim->in_room))->name);
                    } else {
                        sprintf(buf, "%s killed by %s at %s",
                                GET_NAME(victim), GET_NAME(ch),
                                (real_roomp(victim->in_room))->name);

                    }
                }
            } else {
                sprintf(buf, "%s killed by %s at Nowhere.", GET_NAME(victim),
                        (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)));
            }
            log_sev(buf, 6);
        }
        if (IS_SET(real_roomp(victim->in_room)->room_flags, ARENA_ROOM)) {
            if (IS_PC(ch)) {
                ch->specials.a_kills = ch->specials.a_kills + 1;
            }
            if (IS_PC(victim)) {
                victim->specials.a_deaths = victim->specials.a_deaths + 1;
            }
        } else {
            if (IS_PC(ch)) {
                ch->specials.m_kills = ch->specials.m_kills + 1;
            }
            if (IS_PC(victim)) {
                victim->specials.m_deaths = victim->specials.m_deaths + 1;
            }
        }

        die(victim, killedbytype);

        /*
         *  if the victim is dead, return TRUE.
         */
        /*
         * added the pc check to fix crashes when a pc starves, posion,
         * etc -gordon jan232004-
         */
        if (!IS_PC(victim)) {
            if (IS_SET(ch->specials.act, PLR_AUTOGOLD)) {
                do_get(ch, "all.coin corpse", -1);
            }
            if (IS_SET(ch->specials.act, PLR_AUTOLOOT)) {
                do_get(ch, "all corpse", -1);
            }
        }
        victim = 0;
        return (TRUE);
    } else {
        if (DestroyedItems) {
            if (check_falling(victim)) {
                /*
                 * 0 = ok, 1 = dead
                 */
                return (TRUE);
            }
            DestroyedItems = 0;
        }
        return (FALSE);
    }
}

int MissileDamage(struct char_data *ch, struct char_data *victim,
                  int dam, int attacktype)
{
    if (DamCheckDeny(ch, victim, attacktype)) {
        return (FALSE);
    }
    dam = SkipImmortals(victim, dam, attacktype);

    if (!DamDetailsOk(ch, victim, dam, attacktype)) {
        return (FALSE);
    }
    SetVictFighting(ch, victim);

    /*
     * make the ch hate the loser who used a missile attack on them.
     */
    if (!IS_PC(victim) && !Hates(victim, ch)) {
        AddHated(victim, ch);
    }
    dam = DamageTrivia(ch, victim, dam, attacktype);

    if (DoDamage(ch, victim, dam, attacktype)) {
        return (TRUE);
    }
    DamageMessages(ch, victim, dam, attacktype);

    if (DamageEpilog(ch, victim, attacktype)) {
        return (TRUE);
    }
    return (FALSE);
    /*
     * not dead
     */
}

int damage(struct char_data *ch, struct char_data *victim,
           int dam, int attacktype)
{
    if (DamCheckDeny(ch, victim, attacktype)) {
        return (FALSE);
    }
    dam = SkipImmortals(victim, dam, attacktype);

    if (!DamDetailsOk(ch, victim, dam, attacktype)) {
        return (FALSE);
    }
    if (attacktype != TYPE_RANGE_WEAPON) {
        /*
         * this ain't smart, pc's wielding bows?
         */
        SetVictFighting(ch, victim);
        SetCharFighting(ch, victim);
    }

    dam = DamageTrivia(ch, victim, dam, attacktype);

    if (DoDamage(ch, victim, dam, attacktype)) {
        return (TRUE);
    }
    DamageMessages(ch, victim, dam, attacktype);

    if (DamageEpilog(ch, victim, attacktype)) {
        return (TRUE);
    }
    return (FALSE);
}

int GetWeaponType(struct char_data *ch, struct obj_data **wielded)
{
    int             w_type;

    if (ch->equipment[WIELD] &&
        (ch->equipment[WIELD]->obj_flags.type_flag == ITEM_WEAPON)) {

        *wielded = ch->equipment[WIELD];
        w_type = Getw_type(*wielded);
    } else {
        if (IS_NPC(ch) && (ch->specials.attack_type >= TYPE_HIT)) {
            w_type = ch->specials.attack_type;
        } else {
            w_type = TYPE_HIT;
        }
        *wielded = 0;
        /*
         * no weapon
         */

    }
    return (w_type);
}

int Getw_type(struct obj_data *wielded)
{
    int             w_type;

    switch (wielded->obj_flags.value[3]) {
    case 0:
        w_type = TYPE_SMITE;
        break;
    case 1:
        w_type = TYPE_STAB;
        break;
    case 2:
        w_type = TYPE_WHIP;
        break;
    case 3:
        w_type = TYPE_SLASH;
        break;
    case 4:
        w_type = TYPE_SMASH;
        break;
    case 5:
        w_type = TYPE_CLEAVE;
        break;
    case 6:
        w_type = TYPE_CRUSH;
        break;
    case 7:
        w_type = TYPE_BLUDGEON;
        break;
    case 8:
        w_type = TYPE_CLAW;
        break;
    case 9:
        w_type = TYPE_BITE;
        break;
    case 10:
        w_type = TYPE_STING;
        break;
    case 11:
        w_type = TYPE_PIERCE;
        break;
    case 12:
        w_type = TYPE_BLAST;
        break;
    case 13:
        w_type = TYPE_IMPALE;
        break;
    case 14:
        w_type = TYPE_RANGE_WEAPON;
        break;
    default:
        w_type = TYPE_HIT;
        break;
    }
    return (w_type);
}

int HitCheckDeny(struct char_data *ch, struct char_data *victim, int type,
                 int DistanceWeapon)
{
    struct room_data *rp;
    char            buf[256];
    extern char     PeacefulWorks;

    rp = real_roomp(ch->in_room);
    if (rp && rp->room_flags & PEACEFUL && PeacefulWorks) {
        sprintf(buf, "hit() called in PEACEFUL room");
        Log(buf);
        stop_fighting(ch);
        return (TRUE);
    }

#if PREVENT_PKILL
    /*
     * this should help stop pkills
     */

    if ((IS_PC(ch) || IS_SET(ch->specials.act, ACT_POLYSELF)) &&
        (IS_PC(victim) || IS_SET(victim->specials.act, ACT_POLYSELF)) &&
        (ch != victim) && !CanFightEachOther(ch, victim)) {
        sprintf(buf, "%s was found fighting %s!", GET_NAME(ch),
                GET_NAME(victim));
        Log(buf);
        act("You get an eerie feeling you should not be doing this, you FLEE!",
            FALSE, ch, 0, victim, TO_CHAR);
        act("$n seems about to attack you, then looks very scared!", FALSE,
            ch, 0, victim, TO_VICT);
        do_flee(ch, "", 0);
    }
#endif

    if ((ch->in_room != victim->in_room) && !DistanceWeapon) {
        sprintf(buf, "NOT in same room when fighting : %s, %s",
                ch->player.name, victim->player.name);
        Log(buf);
        stop_fighting(ch);
        return (TRUE);
    }

    if (GET_MOVE(ch) < -10) {
        send_to_char("You're too exhausted to fight\n\r", ch);
        stop_fighting(ch);
        return (TRUE);
    }

    if (victim->attackers >= 6 && ch->specials.fighting != victim) {
        send_to_char("You can't attack them,  no room!\n\r", ch);
        return (TRUE);
    }

    /*
     * if the character is already fighting several opponents, and he
     * wants to hit someone who is not currently attacking him, then deny
     * them. if he is already attacking that person, he can continue, even
     * if they stop fighting him.
     */
    if ((ch->attackers >= 6) && (victim->specials.fighting != ch) &&
        ch->specials.fighting != victim) {
        send_to_char("There are too many other people in the way.\n\r", ch);
        return (TRUE);
    }

#if 0
    /*
     * forces mob/pc to flee if person fighting cuts link
     */
    if (!IS_PC(ch)) {
        if (ch->specials.fighting && IS_PC(ch->specials.fighting) &&
            !ch->specials.fighting->desc) {
            do_flee(ch, "\0", 0);
            return (TRUE);
        }
    }
#endif

    /*
     * force link dead persons to flee from all battles
     */
    if (IS_LINKDEAD(victim) && (victim->specials.fighting)) {
        do_flee(victim, "", 0);
    }

    if (IS_LINKDEAD(ch) && (ch->specials.fighting)) {
        do_flee(ch, "", 0);
    }
    /*
     * end link dead flees
     */

#if 0

    if (IS_LINKDEAD(ch)) {
        return (TRUE);
    }
#endif

    if (victim == ch) {
        if (Hates(ch, victim)) {
            RemHated(ch, victim);
        }
        return (TRUE);
    }

    if (GET_POS(victim) == POSITION_DEAD) {
        return (TRUE);
    }
    if (MOUNTED(ch)) {
        if (!RideCheck(ch, -5)) {
            FallOffMount(ch, MOUNTED(ch));
            Dismount(ch, MOUNTED(ch), POSITION_SITTING);
            return (TRUE);
        }
    } else {
        if (RIDDEN(ch)) {
            if (!RideCheck(RIDDEN(ch), -10)) {
                FallOffMount(RIDDEN(ch), ch);
                Dismount(RIDDEN(ch), ch, POSITION_SITTING);
                return (TRUE);
            }
        }
    }

    return (FALSE);
}

int CalcThaco(struct char_data *ch)
{
    int             calc_thaco;
    extern struct str_app_type str_app[];
    int              bestclass;

    /*
     * Calculate the raw armor including magic armor
     * The lower AC, the better
     */

    if (!IS_NPC(ch)) {
        bestclass = BestFightingClass(ch);
        calc_thaco = thaco[bestclass][(int)GET_LEVEL(ch, bestclass)];
    } else {
        /*
         * THAC0 for monsters is set in the HitRoll
         */
        calc_thaco = 20;
    }

    /*
     * Drow are -4 to hit during daylight or lighted rooms.
     */
    if (!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DROW && IS_PC(ch) &&
        !IS_AFFECTED(ch, AFF_DARKNESS) && !IS_UNDERGROUND(ch)) {
        calc_thaco += 4;
    }

    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
        calc_thaco += berserkthaco(ch);
    }

    if (IS_AFFECTED2(ch, AFF2_PROTECT_FROM_GOOD) && ch->specials.fighting &&
        IS_GOOD(ch->specials.fighting)) {
        /*
         * get +1 to hit good
         */
        calc_thaco -= 1;
    }

    if (IS_AFFECTED(ch, AFF_PROTECT_FROM_EVIL) && ch->specials.fighting &&
        IS_EVIL(ch->specials.fighting)) {
        /*
         * get +1 to hit evil
         */
        calc_thaco -= 1;
    }

    /*
     * you get -4 to hit a mob if your evil and he has prot from evil
     */
    if (ch->specials.fighting && IS_EVIL(ch) &&
        IS_AFFECTED(ch->specials.fighting, AFF_PROTECT_FROM_EVIL)) {
        calc_thaco += 4;
    }

    /*
     * you get -4 to hit a mob if your good and he has prot from good
     */
    if (ch->specials.fighting && IS_GOOD(ch) &&
        IS_AFFECTED2(ch->specials.fighting, AFF2_PROTECT_FROM_GOOD)) {
        calc_thaco += 4;
    }

    if (HasClass(ch, CLASS_MONK)) {
        /*
         * monks get a bit of a bonus to hit -Lennya 20030901
         */
        calc_thaco -= (int) GetMaxLevel(ch) / 7;
    }

    calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
    calc_thaco -= GET_HITROLL(ch);
    calc_thaco += GET_COND(ch, DRUNK) / 5;
    return (calc_thaco);
}

int HitOrMiss(struct char_data *ch, struct char_data *victim, int calc_thaco)
{
    int             diceroll,
                    victim_ac;

    extern struct dex_app_type dex_app[];

    diceroll = number(1, 20);

    if (victim->style == FIGHTING_STYLE_BERSERKED ||
        victim->style == FIGHTING_STYLE_AGGRESSIVE) {
        victim_ac = (GET_AC(victim) + 20) / 10;
    } else if (victim->style == FIGHTING_STYLE_EVASIVE ||
               victim->style == FIGHTING_STYLE_DEFENSIVE) {
        victim_ac = (GET_AC(victim) - 25) / 10;
    } else {
        victim_ac = (GET_AC(victim)) / 10;
    }

    if (!AWAKE(victim)) {
        victim_ac -= dex_app[(int) GET_DEX(victim)].defensive;
    }
    victim_ac = MAX(-10, victim_ac);
    /*
     * -10 is lowest
     */

    if ((diceroll < 20) && AWAKE(victim) &&
        ((diceroll == 1) || ((calc_thaco - diceroll) > victim_ac))) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}

void MissVictim(struct char_data *ch, struct char_data *v, int type,
                int w_type, int (*dam_func) ())
{
    struct obj_data *o;

    if (type <= 0) {
        type = w_type;
    }
    if (dam_func == MissileDamage) {
        if (ch->equipment[WIELD]) {
            o = unequip_char(ch, WIELD);
            if (o) {
                act("$p falls to the ground harmlessly", FALSE, ch, o, 0,
                    TO_CHAR);
                act("$p falls to the ground harmlessly", FALSE, ch, o, 0,
                    TO_ROOM);
                obj_to_room(o, ch->in_room);
            }
        }
    }
    (*dam_func) (ch, v, 0, w_type);
}

int GetWeaponDam(struct char_data *ch, struct char_data *v,
                 struct obj_data *wielded)
{
    int             dam,
                    j;
    struct obj_data *obj;
    extern struct str_app_type str_app[];

    dam = str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += GET_DAMROLL(ch);

    if (!wielded) {
        if (IS_NPC(ch) || HasClass(ch, CLASS_MONK)) {
            dam += dice(ch->specials.damnodice, ch->specials.damsizedice);
        } else {
            /*
             * Max. 2 dam with bare hands
             */
            dam += number(0, 2);
        }
    } else {
        if (wielded->obj_flags.value[2] > 0) {
            dam += dice(wielded->obj_flags.value[1],
                        wielded->obj_flags.value[2]);
        } else {
            act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_CHAR);
            act("$p snaps into pieces!", TRUE, ch, wielded, 0, TO_ROOM);
            if ((obj = unequip_char(ch, WIELD)) != NULL) {
                MakeScrap(ch, v, obj);
                dam += 1;
            }
        }

        if (wielded->obj_flags.weight >
            str_app[STRENGTH_APPLY_INDEX(ch)].wield_w && ch->equipment[HOLD]) {
            /*
             * its too heavy to wield properly
             */
            dam /= 2;
        }

        /*
         * check for the various APPLY_RACE_SLAYER and APPLY_ALIGN_SLAYR
         * here.
         *
         * upped the multipliers a wee bit -Lennya 20040221
         */
        if (!A_NOSLAY(ch)) {
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                if (wielded->affected[j].location == APPLY_RACE_SLAYER &&
                    wielded->affected[j].modifier == GET_RACE(v)) {
                    dam *= 1.7;
                }
                if (wielded->affected[j].location == APPLY_ALIGN_SLAYER) {
                    if (wielded->affected[j].modifier > 1 && IS_GOOD(v)) {
                        dam *= 1.6;
                    } else if (wielded->affected[j].modifier == 1 &&
                             !IS_GOOD(v) && !IS_EVIL(v)) {
                        dam *= 1.6;
                    } else if (wielded->affected[j].modifier < 1 &&
                               IS_EVIL(v)) {
                        dam *= 1.6;
                    }
                }
            }
        }
    }

    if (GET_POS(v) < POSITION_FIGHTING) {
        dam *= 1 + (POSITION_FIGHTING - GET_POS(v)) / 3;
    }
    /*
     * Position sitting x 1.33
     * Position resting x 1.66
     * Position sleeping x 2.00
     * Position stunned x 2.33
     * Position incap x 2.66
     * Position mortally x 3.00
     */

    if (ch->style == FIGHTING_STYLE_AGGRESSIVE) {
        if (FSkillCheck(ch, FIGHTING_STYLE_AGGRESSIVE)) {
            /*
             * 20% more damage if successful
             */
            dam = dam * 1.20;
        }
    }

    if (ch->style == FIGHTING_STYLE_DEFENSIVE) {
        if (FSkillCheck(ch, FIGHTING_STYLE_DEFENSIVE)) {
            /*
             * 10% less damage if successful
             */
            dam = dam - dam * 0.10;
        } else {
            /*
             * 20% less damage if failed
             */
            dam = dam - dam * 0.20;
        }
    }
    if (GET_POS(v) <= POSITION_DEAD) {
        return (0);
    }
    dam = MAX(1, dam);
    return (dam);
}

int LoreBackstabBonus(struct char_data *ch, struct char_data *v)
{
    int             mult = 0;
    int             learn = 0;

    if (IsAnimal(v) && ch->skills[SKILL_CONS_ANIMAL].learned) {
        learn = ch->skills[SKILL_CONS_ANIMAL].learned;
    }
    if (IsVeggie(v) && ch->skills[SKILL_CONS_VEGGIE].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
    }
    if (IsDiabolic(v) && ch->skills[SKILL_CONS_DEMON].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
    }
    if (IsReptile(v) && ch->skills[SKILL_CONS_REPTILE].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
    }
    if (IsUndead(v) && ch->skills[SKILL_CONS_UNDEAD].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
    }
    if (IsGiantish(v) && ch->skills[SKILL_CONS_GIANT].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
    }
    if (IsPerson(v) && ch->skills[SKILL_CONS_PEOPLE].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
    }
    if (IsOther(v) && ch->skills[SKILL_CONS_OTHER].learned) {
        learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned / 2);
    }
    if (learn > 40) {
        mult += 1;
    }
    if (learn > 74) {
        mult += 1;
    }
    if (mult > 0) {
        send_to_char("Your lore aids your attack!\n\r", ch);
    }
    return (mult);
}

void HitVictim(struct char_data *ch, struct char_data *v, int dam,
               int type, int w_type, int (*dam_func) ())
{
    extern byte     lesser_backstab_mult[];
    extern byte     backstab_mult[];
    int             dead;
    int             tmp;
    struct obj_data *shield;

    if (type == SKILL_BACKSTAB) {
        if (GET_LEVEL(ch, THIEF_LEVEL_IND)) {
            if (!(ch->specials.remortclass == THIEF_LEVEL_IND + 1)) {
                tmp = lesser_backstab_mult[(int)GET_LEVEL(ch, THIEF_LEVEL_IND)];
            } else {
                tmp = backstab_mult[(int)GET_LEVEL(ch, THIEF_LEVEL_IND)];
            }
            tmp += LoreBackstabBonus(ch, v);
        } else {
            tmp = backstab_mult[GetMaxLevel(ch)];
        }
        dam *= tmp;
        dead = (*dam_func) (ch, v, dam, type);
    } else {
        /*
         * reduce damage for dodge skill:
         */
        if (v->skills && v->skills[SKILL_DODGE].learned) {
            if (v->style == FIGHTING_STYLE_DEFENSIVE) {
                if (number(1, 101) <=
                        ((FSkillCheck(v, FIGHTING_STYLE_DEFENSIVE)) ?
                         v->skills[SKILL_DODGE].learned * 1.25 :
                         v->skills[SKILL_DODGE].learned)) {
                    dam -= number(1,
                                  ((FSkillCheck(v, FIGHTING_STYLE_DEFENSIVE)) ?
                                   5 : 3));
                    if (HasClass(v, CLASS_MONK)) {
                        MonkDodge(ch, v, &dam);
                    }
                }
            }
        }
        if (v->equipment[WEAR_SHIELD]) {
            shield = v->equipment[WEAR_SHIELD];
            if (obj_index[shield->item_number].virtual == SMITH_SHIELD) {
                SmithShield(ch, v, shield, &dam);
            }
        }
        dead = (*dam_func) (ch, v, dam, w_type);
    }
    /*
     * if the victim survives, lets hit him with a weapon spell
     */
    if (!dead) {
        if (!A_NOWSPELLS(ch)) {
            WeaponSpell(ch, v, 0, w_type);
        }
    }
}

void root_hit(struct char_data *ch, struct char_data *victim, int type,
              int (*dam_func) (), int DistanceWeapon)
{
    int             temp;
    int             w_type,
                    thaco,
                    dam;
    struct obj_data *wielded = 0;
    /*
     * this is rather important.
     */

    if (HitCheckDeny(ch, victim, type, DistanceWeapon)) {
        return;
    }
    GET_MOVE(ch) -= 1;

    w_type = GetWeaponType(ch, &wielded);
    if (w_type == TYPE_HIT) {
        w_type = GetFormType(ch);
    }
    thaco = CalcThaco(ch);

    WeaponSkillCheck(ch);

    if (HitOrMiss(ch, victim, thaco)) {
        if ((dam = GetWeaponDam(ch, victim, wielded)) > 0) {
            if (number(1, 4) == 1) {
                /*
                 * lets add resistances
                 */
                temp = PreProcDam(victim, w_type, dam);

                if (temp == -1) {
                    ch_printf(ch, "Your attack against %s is futile.\n\r",
                              IS_NPC(victim) ? victim->player.short_descr :
                              GET_NAME(victim));
                } else if (temp < dam) {
                    ch_printf(ch, "%s seems to resist your attack!\n\r",
                              IS_NPC(victim) ? victim->player.short_descr :
                              GET_NAME(victim));
                } else if (temp > dam) {
                    send_to_char("Your attack seems to do an extraordinary "
                                 "amount of damage!\n\r", ch);
                }
            }
            HitVictim(ch, victim, dam, type, w_type, dam_func);
        } else {
            MissVictim(ch, victim, type, w_type, dam_func);
        }
    } else {
        MissVictim(ch, victim, type, w_type, dam_func);
    }
}

void MissileHit(struct char_data *ch, struct char_data *victim, int type)
{
    root_hit(ch, victim, type, MissileDamage, TRUE);
}

void hit(struct char_data *ch, struct char_data *victim, int type)
{
    int             dam = 0;

    if (A_NOASSIST(ch, victim)) {
        act("$N is already engaged with someone else!", FALSE, ch, 0,
            victim, TO_CHAR);
        return;
    }

    /*
     * let's see if this works
     */
    if (IS_AFFECTED(victim, AFF_BLADE_BARRIER) &&
        !IS_AFFECTED(ch, AFF_BLADE_BARRIER)) {
        /*
         * 8d8, half for save, on a successful hit
         */
        if (HitOrMiss(victim, ch, CalcThaco(victim))) {
            dam = dice(8, 8);
            if (saves_spell(ch, SAVING_SPELL)) {
                dam >>= 1;
            }
            if (damage(victim, ch, dam, SPELL_BLADE_BARRIER) &&
                GET_POS(ch) == POSITION_DEAD) {
                return;
            }
        } else {
            damage(victim, ch, 0, SPELL_BLADE_BARRIER);
        }
    }

    root_hit(ch, victim, type, damage, FALSE);
}

/*
 * control the fights going on
 */
void perform_violence(int pulse)
{
    struct follow_type *f;
    struct char_data *ch,
                   *vict;
    struct obj_data *tmp,
                   *tmp2,
                   *obj;
    int             i,
                    tdir,
                    cmv,
                    max_cmv,
                    caught,
                    rng,
                    tdr;
    float           x;
    int             perc;
    int             weapontype;
    struct room_data *rp;
    char            buf[MAX_INPUT_LENGTH + 40];
    struct char_data *rec;
    struct obj_data *weapon;

    for (ch = combat_list; ch; ch = combat_next_dude) {
        combat_next_dude = ch->next_fighting;
        rp = real_roomp(ch->in_room);

        /*
         * assert(ch->specials.fighting);
         */
        if (!ch->specials.fighting) {
            Log("!ch->specials.fighting in perform violence fight.c");
            return;
        } else if (rp && rp->room_flags & PEACEFUL) {
            sprintf(buf, "perform_violence() found %s fighting in a PEACEFUL "
                         "room.", ch->player.name);
            stop_fighting(ch);
            Log(buf);
        } else if (ch == ch->specials.fighting) {
            stop_fighting(ch);
        } else {
            if (IS_NPC(ch)) {
                DevelopHatred(ch, ch->specials.fighting);
                rec = ch->specials.fighting;
                if (!IS_PC(ch->specials.fighting)) {
                    /*
                     * while(rec->master)
                     */
                    for (i = 0; i < 4 && rec->master &&
                         rec->master->in_room == ch->in_room; i++) {
                        /*
                         * do it 4 times, should catch all the legal
                         * follower/master sequences
                         */
                        AddHated(ch, rec->master);
                        rec = rec->master;
                    }
                }
            }

            if (AWAKE(ch) && (ch->in_room == ch->specials.fighting->in_room) &&
                (!IS_AFFECTED(ch, AFF_PARALYSIS))) {
                if (!IS_NPC(ch)) {
                    /*
                     * set x = # of attacks
                     */

                    x = ch->mult_att;

                    if (ch->style == FIGHTING_STYLE_BERSERKED &&
                        !IS_SET(ch->specials.affected_by2,
                                AFF2_STYLE_BERSERK) &&
                        FSkillCheck(ch, FIGHTING_STYLE_BERSERKED)) {
                        /*
                         * yay, let's go 'zerk!
                         */
                        act("$c000B$n turns red and suddenly becomes raged "
                            "with anger!$c000w", TRUE, ch, 0, 0, TO_ROOM);
                        send_to_char("$c000BYou feel your rage overcome "
                                     "you!!\n\r$c000w", ch);
                        SET_BIT(ch->specials.affected_by2, AFF2_STYLE_BERSERK);
                    }

                    /*
                     * if dude is a monk, and is wielding something
                     */

                    if (HasClass(ch, CLASS_MONK) && !IS_IMMORTAL(ch) &&
                        ch->equipment[WIELD]) {
                        /*
                         * set it to one, they only get one attack
                         * two -Lennya
                         */
                        x = MIN(2, (1 + GET_LEVEL(ch, MONK_LEVEL_IND) / 16.0));
                    }

                    /*
                     * No clue why this should be in effect. Mounted
                     * fighting is a bother rather than a bonus, no need
                     * to make it even worse.  -Lennya 20030108 if
                     * (MOUNTED(ch)) { x /= 2.0; }
                     *
                     * Modifier for weaponskills -Gordon 1-18-04
                     */
                    if ((obj = ch->equipment[WIELD]) && IS_WEAPON(obj)) {
                        /*
                         * 350 - 409
                         */
                        weapontype = obj->weapontype + WEAPON_FIRST;

                        if (ch->weaponskills.slot1 == weapontype) {
                            x += (ch->weaponskills.grade1 - 50) / 100;
                        } else if (ch->weaponskills.slot2 == weapontype) {
                            x += (ch->weaponskills.grade2 - 50) / 100;
                        } else if (ch->weaponskills.slot3 == weapontype) {
                            x += (ch->weaponskills.grade3 - 50) / 100;
                        } else if (ch->weaponskills.slot4 == weapontype) {
                            x += (ch->weaponskills.grade4 - 50) / 100;
                        } else if (ch->weaponskills.slot5 == weapontype) {
                            x += (ch->weaponskills.grade5 - 50) / 100;
                        } else if (ch->weaponskills.slot6 == weapontype) {
                            x += (ch->weaponskills.grade6 - 50) / 100;
                        } else if (ch->weaponskills.slot7 == weapontype) {
                            x += (ch->weaponskills.grade7 - 50) / 100;
                        } else if (ch->weaponskills.slot8 == weapontype) {
                            x += (ch->weaponskills.grade8 - 50) / 100;
                        }
                    }

                    if (!A_NOHASTE(ch)) {
                        if (IS_SET(ch->specials.affected_by2, AFF2_HASTE)) {
                            /*
                             * I did this for the remortskills changes.
                             * Maybe I'll look at slow later (Gordon)
                             */
                            x = x + 1.5;
                        }

                        if (IS_SET(ch->specials.affected_by2, AFF2_SLOW))
                            x = x / 2;
                    }

                    if (ch->equipment[WIELD]) {
#if 0
                        send_to_char("Wielding a weapon",ch);
#endif
                        x += (float) ch->equipment[WIELD]->speed / 100;
                    } else if (HasClass(ch, CLASS_MONK)) {
                            x += 0.55;
                    }

                    /*
                     * work through all of their attacks, until there is
                     * not a full attack left
                     */

                    tmp = 0;
                    tmp2 = 0;

                    if (DUAL_WIELD(ch)) {
                        tmp = unequip_char(ch, HOLD);
                    }

                    /*
                     * have to check for monks holding things.
                     */
                    if (ch->equipment[HOLD] && !(ch->equipment[WIELD]) &&
                        ITEM_TYPE(ch->equipment[HOLD]) == ITEM_WEAPON &&
                        HasClass(ch, CLASS_MONK)) {
                        tmp2 = unequip_char(ch, HOLD);
                    }

                    if (IS_SET(ch->specials.affected_by2, AFF2_BERSERK)) {
                        x += 0.75;
                    }

                    if (IS_SET(ch->specials.affected_by2, AFF2_STYLE_BERSERK)) {
                        x += 0.50;
                    }

                    /*
                     * autoassist?
                     */
                    if (ch && IS_AFFECTED(ch, AFF_GROUP) && IS_PC(ch)) {
                        for (f = ch->followers; f; f = f->next) {
                            if ((GET_POS(f->follower) != POSITION_FIGHTING) &&
                                !f->follower->specials.fighting &&
                                f->follower->in_room == ch->in_room &&
                                GET_POS(f->follower) > POSITION_SITTING &&
                                IS_SET(f->follower->specials.act,
                                       PLR_AUTOASSIST) &&
                                IS_AFFECTED(f->follower, AFF_GROUP)) {
                                act("$n screams and runs into battle, weapons "
                                    "a swinging.", FALSE, f->follower, 0, 0,
                                    TO_ROOM);
                                ch_printf(f->follower,
                                          "You raise your weapon and run in to"
                                          " assist %s.\n\r",
                                          GET_NAME(f->follower->master));
#if 0
                                do_assist(f->follower,
                                GET_NAME(f->follower->master),
                                0);
#endif
                                set_fighting(f->follower,
                                             ch->specials.fighting);
                            }
                        }

                        if (ch->master &&
                            GET_POS(ch->master) != POSITION_FIGHTING &&
                            !ch->master->specials.fighting &&
                            ch->master->in_room == ch->in_room &&
                            GET_POS(ch->master) > POSITION_SITTING &&
                            IS_SET(ch->master->specials.act, PLR_AUTOASSIST) &&
                            IS_AFFECTED(ch->master, AFF_GROUP)) {

                            act("$n screams and runs into battle, weapons a "
                                "swinging.", FALSE, ch->master, 0, 0, TO_ROOM);
                            ch_printf(ch->master,
                                      "You raise your weapon and run in to "
                                      "assist %s.\n\r", GET_NAME(ch));
#if 0
                            do_assist(ch->master,
                            GET_NAME(f->follower->master),
                            0);
#endif
                            set_fighting(ch->master, ch->specials.fighting);
                        }
                    }

                    if (ch->style == FIGHTING_STYLE_EVASIVE) {
                        if (FSkillCheck(ch, FIGHTING_STYLE_EVASIVE)) {
                            x = x - x * 0.20;
                        } else {
                            x = x - x * 0.40;
                        }
                    }

                    if (affected_by_spell(ch, SKILL_FLURRY)) {
                        x *= 1.3;
                    }

                    while (x > 0.999) {
                        if (ch->specials.fighting) {
                            hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
                        } else {
                            x = 0.0;
                            break;
                        }
                        x -= 1.0;
                    }

                    if (x > .01) {
#if 1
                        /*
                         * check to see if the chance to make the last
                         * attack is successful
                         */
                        perc = number(1, 100);
                        if (perc <= (int) (x * 100.0) &&
                            ch->specials.fighting) {
                            hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
                        }
#else
                        /*
                         * lets give them the hit
                         */
                        if (ch->specials.fighting) {
                            hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
                        }
#endif
                    }

                    if (tmp) {
                        equip_char(ch, tmp, HOLD);
                    }
                    if (tmp2) {
                        equip_char(ch, tmp2, HOLD);
                    }

#if 1
                    /*
                     * check for the second attack
                     */
                    if (DUAL_WIELD(ch) && ch->skills) {
                        x = 1.0;

                        if (!A_NOHASTE(ch)) {
                            if (IS_SET(ch->specials.affected_by2, AFF2_HASTE) &&
                                (ch->specials.remortclass ==
                                 RANGER_LEVEL_IND + 1)) {
                                x = x + 0.75;
                            }
                            if (IS_SET(ch->specials.affected_by2, AFF2_SLOW)) {
                                x = x / 2;
                            }
                        }

                        while (x > 0.999) {
                            if (ch->specials.fighting) {
                                if ((perc = number(1, 101)) <
                                    ch->skills[SKILL_DUAL_WIELD].learned) {
                                    weapon = unequip_char(ch, WIELD);
                                    tmp = unequip_char(ch, HOLD);
                                    equip_char(ch, tmp, WIELD);

                                    if (ch->specials.fighting) {
                                        hit(ch, ch->specials.fighting,
                                            TYPE_UNDEFINED);
                                    }

                                    if (ch->equipment[WIELD]) {
                                        tmp = unequip_char(ch, WIELD);
                                        equip_char(ch, tmp, HOLD);

                                    }
                                    equip_char(ch, weapon, WIELD);
                                } else {
                                    /*
                                     * we failed, gotta stop attacking after
                                     * the fumble messages and learnage
                                     */
                                    if (!HasClass(ch, CLASS_RANGER) ||
                                        number(1, 20) > GET_DEX(ch)) {
                                        tmp = unequip_char(ch, HOLD);
                                        obj_to_room(tmp, ch->in_room);
                                        act("$c0014You fumble and drop $p",
                                            0, ch, tmp, tmp, TO_CHAR);
                                        act("$c0014$n fumbles and drops $p", 0,
                                            ch, tmp, tmp, TO_ROOM);

                                        if (HasClass(ch, CLASS_RANGER)) {
                                            LearnFromMistake(ch,
                                                             SKILL_DUAL_WIELD,
                                                             FALSE, 95);
                                        }

                                        if (number(1, 20) > GET_DEX(ch)) {
                                            tmp = unequip_char(ch, WIELD);
                                            obj_to_room(tmp, ch->in_room);
                                            act("$c0015and you fumble and drop"
                                                " $p too!", 0, ch, tmp, tmp,
                                                TO_CHAR);
                                            act("$c0015and then fumbles and "
                                                "drops $p as well!", 0, ch,
                                                tmp, tmp, TO_ROOM);

                                            if (HasClass(ch, CLASS_RANGER)) {
                                                LearnFromMistake(ch,
                                                             SKILL_DUAL_WIELD,
                                                             FALSE, 95);
                                            }

                                        }
                                        x = 0.0;
                                    }
                                }
                            } else {
                                x = 0.0;
                                break;
                            }
                            x -= 1.0;
                        }

                        if (x > .01) {
                            /*
                             * check to see if the chance to make the last
                             * attack is successful
                             */
                            perc = number(1, 100);
                            if (perc <= (int) (x * 100.0) &&
                                ch->specials.fighting) {
                                if ((perc = number(1, 101)) <
                                    ch->skills[SKILL_DUAL_WIELD].learned) {
                                    weapon = unequip_char(ch, WIELD);
                                    tmp = unequip_char(ch, HOLD);
                                    equip_char(ch, tmp, WIELD);

                                    if (ch->specials.fighting) {
                                        hit(ch, ch->specials.fighting,
                                            TYPE_UNDEFINED);
                                    }

                                    if (ch->equipment[WIELD]) {
                                        tmp = unequip_char(ch, WIELD);
                                        equip_char(ch, tmp, HOLD);

                                    }
                                    equip_char(ch, weapon, WIELD);
                                } else if (!HasClass(ch, CLASS_RANGER) ||
                                           number(1, 20) > GET_DEX(ch)) {
                                    tmp = unequip_char(ch, HOLD);
                                    obj_to_room(tmp, ch->in_room);
                                    act("$c0014You fumble and drop $p",
                                        0, ch, tmp, tmp, TO_CHAR);
                                    act("$c0014$n fumbles and drops $p",
                                        0, ch, tmp, tmp, TO_ROOM);

                                    if (HasClass(ch, CLASS_RANGER)) {
                                        LearnFromMistake(ch, SKILL_DUAL_WIELD,
                                                         FALSE, 95);
                                    }

                                    if (number(1, 20) >
                                        GET_DEX(ch)) {
                                        tmp = unequip_char(ch, WIELD);
                                        obj_to_room(tmp, ch->in_room);
                                        act("$c0015and you fumble and "
                                            "drop $p too!", 0, ch, tmp,
                                            tmp, TO_CHAR);
                                        act("$c0015and then fumbles and"
                                            " drops $p as well!", 0, ch,
                                            tmp, tmp, TO_ROOM);

                                        if (HasClass(ch, CLASS_RANGER)) {
                                            LearnFromMistake(ch,
                                                             SKILL_DUAL_WIELD,
                                                             FALSE, 95);
                                        }
                                    }
                                }
                            }
                        }
                    }
#endif
                } else {
                    /*
                     * We are a NPC
                     */
                    x = ch->mult_att;

                    if (!A_NOHASTE(ch)) {
                        if (IS_SET(ch->specials.affected_by2, AFF2_HASTE)) {
                            /*
                             * I did this for the remortskills changes.
                             * Maybe I'll look at slow later (Gordon)
                             */
                            x = x + 1.5;
                        }

                        if (IS_SET(ch->specials.affected_by2, AFF2_SLOW))
                            x = x / 2;
                    }

                    if (affected_by_spell(ch, SKILL_FLURRY)) {
                        x *= 1.3;
                    }

                    while (x > 0.999) {
                        if (ch->specials.fighting) {
                            hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
                        } else if ((vict = FindAHatee(ch)) != NULL &&
                            vict->attackers < 6) {
                            hit(ch, vict, TYPE_UNDEFINED);
                        } else if ((vict = FindAnAttacker(ch)) != NULL &&
                                   vict->attackers < 6) {
                            hit(ch, vict, TYPE_UNDEFINED);
                        }
                        x -= 1.0;
                    }
#if 0
                    if (GET_RACE(ch) == RACE_MFLAYER
                        && ch->specials.fighting)
                        MindflayerAttack(ch, ch->specials.fighting);
#endif
                    if (x > .01) {
                        /*
                         * check to see if the chance to make the last
                         * attack is successful
                         */
                        perc = number(1, 100);
                        if (perc <= (int) (x * 100.0)) {
                            if (ch->specials.fighting) {
                                hit(ch, ch->specials.fighting, TYPE_UNDEFINED);
                            } else if ((vict = FindAHatee(ch)) != NULL &&
                                       vict->attackers < 6) {
                                hit(ch, vict, TYPE_UNDEFINED);
                            } else if ((vict = FindAnAttacker(ch)) != NULL &&
                                       vict->attackers < 6) {
                                    hit(ch, vict, TYPE_UNDEFINED);
                            }
                        }
                    }
                }
                if (affected_by_spell(ch, SKILL_FLURRY)) {
                    affect_from_char(ch, SKILL_FLURRY);
                }
            } else {
                /*
                 * Not in same room, not awake, or paralyzed
                 */
                stop_fighting(ch);
            }
        }
    }

    /*
     * charging loop
     */
    for (ch = character_list; ch; ch = ch->next) {
        /*
         * If charging deal with that
         */
        if (ch->specials.charging) {
            caught = 0;
            max_cmv = 2;
            cmv = 0;
            while ((cmv < max_cmv) && (caught == 0)) {
                if (ch->in_room == ch->specials.charging->in_room) {
                    caught = 1;
                } else if (clearpath(ch, ch->in_room,
                                     ch->specials.charge_dir)) {
                    /*
                     * Continue in a straight line
                     */
                    do_move(ch, "\0", ch->specials.charge_dir + 1);
                    cmv++;
                } else {
                    caught = 2;
                }
            }

            switch (caught) {
            case 1:
                /*
                 * Caught him
                 */
                act("$n sees $N, and attacks!", TRUE, ch, 0,
                    ch->specials.charging, TO_NOTVICT);
                act("$n sees you, and attacks!", TRUE, ch, 0,
                    ch->specials.charging, TO_VICT);
                act("You see $N and attack!", TRUE, ch, 0,
                    ch->specials.charging, TO_CHAR);
                hit(ch, ch->specials.charging, TYPE_UNDEFINED);
                ch->specials.charging = NULL;
                break;
            case 2:
                /*
                 * End of line and didn't catch him
                 */
                tdir = can_see_linear(ch, ch->specials.charging, &rng, &tdr);
                if (tdir > -1) {
                    ch->specials.charge_dir = tdr;
                } else {
                    ch->specials.charging = NULL;
                    act("$n looks around, and sighs dejectedly.", FALSE,
                        ch, 0, 0, TO_ROOM);
                }
                break;
            default:
                break;
            }
        }
    }
}

#if 1
/*
 * This crashes the mud too
 */
struct char_data *FindVictim(struct char_data *ch)
{

    struct char_data *tmp_ch;
    struct room_data *rp;
    unsigned char   found = FALSE;
    unsigned short  ftot = 0,
                    ttot = 0,
                    ctot = 0,
                    ntot = 0,
                    mtot = 0,
                    ktot = 0,
                    dtot = 0;
    unsigned short  total;
    unsigned short  fjump = 0,
                    njump = 0,
                    cjump = 0,
                    mjump = 0,
                    tjump = 0,
                    kjump = 0,
                    djump = 0;

    if (!ch) {
        return (0);
    }

    if (ch->in_room < 0) {
        return (0);
    }
    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0
        Log("/* No room??? Crash??? */");
#endif
        return (0);
    }

    tmp_ch = rp->people;
    if (!tmp_ch) {
        return (0);
    }

    while (tmp_ch) {
        if ((CAN_SEE(ch, tmp_ch)) &&
            (!IS_SET(tmp_ch->specials.act, PLR_NOHASSLE)) &&
            (!IS_AFFECTED(tmp_ch, AFF_SNEAK)) && (ch != tmp_ch) &&
            (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) &&
            ((tmp_ch->specials.zone != ch->specials.zone &&
             !strchr(zone_table[ch->specials.zone].races, GET_RACE(tmp_ch))) ||
            IS_SET(tmp_ch->specials.act, ACT_ANNOYING)) &&
            !in_group(ch, tmp_ch)) {
            /*
             * a potential victim has been found
             */
            found = TRUE;

            if (!IS_NPC(tmp_ch)) {
                if ((affected_by_spell(tmp_ch, SKILL_DISGUISE) ||
                     affected_by_spell(tmp_ch, SKILL_PSYCHIC_IMPERSONATION)) &&
                    number(1, 101) > 50) {
                    /*
                     * 50/50 chance to not attack disguised person
                     */
                    return (NULL);
                }

                if (HasClass(tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                     CLASS_PALADIN | CLASS_RANGER)) {
                    ftot++;
                } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                    ctot++;
                } else if (HasClass(tmp_ch, CLASS_MAGIC_USER) ||
                           HasClass(tmp_ch, CLASS_SORCERER)) {
                    mtot++;
                } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                    ttot++;
                } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                    dtot++;
                } else if (HasClass(tmp_ch, CLASS_MONK)) {
                    ktot++;
                }
            } else {
                ntot++;
            }
        }
        tmp_ch = tmp_ch->next_in_room;
    }

    /*
     * if no legal enemies have been found, return 0
     */

    if (!found) {
        return (0);
    }

    /*
     * give higher priority to fighters, clerics, thieves,magic users if
     * int <= 12 give higher priority to fighters, clerics, magic users
     * thieves is inv > 12 give higher priority to magic users, fighters,
     * clerics, thieves if int > 15
     */

    /*
     * choose a target
     */

    if (ch->abilities.intel <= 3) {
        fjump = 2;
        cjump = 2;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 2;
        djump = 0;
    } else if (ch->abilities.intel <= 9) {
        fjump = 4;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 1;
        kjump = 2;
        djump = 2;
    } else if (ch->abilities.intel <= 12) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 3;
        djump = 2;
    } else if (ch->abilities.intel <= 15) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 3;
        kjump = 2;
        djump = 2;
    } else {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 1;
        mjump = 3;
        kjump = 3;
        djump = 2;
    }

    total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) + (njump * ntot) +
            (mjump * mtot) + (djump * dtot) + (kjump * ktot);
    total = (int) number(1, (int) total);

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            !IS_AFFECTED(tmp_ch, AFF_SNEAK) && ch != tmp_ch &&
            (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) &&
            ((tmp_ch->specials.zone != ch->specials.zone &&
             !strchr(zone_table[ch->specials.zone].races, GET_RACE(tmp_ch))) ||
             IS_SET(tmp_ch->specials.act, ACT_ANNOYING)) &&
            !in_group(ch, tmp_ch)) {

            if (IS_NPC(tmp_ch)) {
                total -= njump;
            } else if (HasClass(tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                         CLASS_PALADIN | CLASS_RANGER)) {
                total -= fjump;
            } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                total -= cjump;
            } else if (HasClass(tmp_ch, CLASS_MAGIC_USER) ||
                       HasClass(tmp_ch, CLASS_SORCERER)) {
                total -= mjump;
            } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                total -= tjump;
            } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                total -= djump;
            } else if (HasClass(tmp_ch, CLASS_MONK)) {
                total -= kjump;
            }

            if (total <= 0) {
                return (tmp_ch);
            }
        }
    }

    if (ch->specials.fighting) {
        return (ch->specials.fighting);
    }

    return (0);
}
#else
/*
 * Stockmuds version
 */
struct char_data *FindVictim(struct char_data *ch)
{
    struct char_data *vict;
    struct room_data *rp = ch->in_room;
    unsigned char   found = FALSE;
    unsigned short  ftot = 0,
                    ttot = 0,
                    ctot = 0,
                    ntot = 0,
                    mtot = 0,
                    ktot = 0,
                    dtot = 0;
    unsigned short  total;
    unsigned short  fjump = 0,
                    njump = 0,
                    cjump = 0,
                    mjump = 0,
                    tjump = 0,
                    kjump = 0,
                    djump = 0;
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0
        Log("No room data in FindVictim ??Crash???");
#endif
        return (0);
    }

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            !IS_AFFECTED(tmp_ch, AFF_SNEAK) && ch != tmp_ch &&
            (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(tmp_ch)) &&
            ((tmp_ch->specials.zone != ch->specials.zone &&
              !strchr(zone_table[ch->specials.zone].races, GET_RACE(tmp_ch))) ||
             IS_SET(tmp_ch->specials.act, ACT_ANNOYING)))  {
            for (vict = rp->people; vict; vict = vict->next_in_room) {
                if (CAN_SEE(ch, vict) &&
                    !IS_SET(vict->specials.act, PLR_NOHASSLE) &&
                    !IS_AFFECTED(vict, AFF_SNEAK) && ch != vict &&
                    (!IS_SET(ch->specials.act, ACT_WIMPY) || !AWAKE(vict)) &&
                    ((vict->specials.zone != ch->specials.zone &&
                      !strchr(zone_table[ch->specials.zone].races,
                              GET_RACE(vict))) ||
                     IS_SET(vict->specials.act, ACT_ANNOYING)) &&
                    !in_group(ch, vict)) {
                    /*
                     * a potential victim has been found
                     */
                    found = TRUE;
                    if (!IS_NPC(vict)) {
                        if (affected_by_spell(vict, SKILL_DISGUISE) ||
                            affected_by_spell(vict,
                                              SKILL_PSYCHIC_IMPERSONATION) &&
                            number(1, 101) > 50) {
                            /*
                             * 50/50 chance to not attack disguised person
                             */
                            return NULL;
                        }

                        if (HasClass(vict, CLASS_WARRIOR | CLASS_BARBARIAN |
                                           CLASS_PALADIN | CLASS_RANGER)) {
                            ftot++;
                        } else if (HasClass(vict, CLASS_CLERIC)) {
                            ctot++;
                        } else if (HasClass(vict, CLASS_MAGIC_USER) ||
                                   HasClass(vict, CLASS_SORCERER)) {
                            mtot++;
                        } else if (HasClass(vict, CLASS_THIEF | CLASS_PSI)) {
                            ttot++;
                        } else if (HasClass(vict, CLASS_DRUID)) {
                            dtot++;
                        } else if (HasClass(vict, CLASS_MONK)) {
                            ktot++;
                        }
                    } else {
                        ntot++;
                    }
                }

                /*
                 * if no legal enemies have been found, return 0
                 */
                if (!found)
                    return NULL;

                /*
                 * give higher priority to fighters, clerics, thieves,magic
                 * users if int <= 12
                 * give higher priority to fighters, clerics, magic users
                 * thieves is int > 12
                 * give higher priority to magic users, fighters, clerics,
                 * thieves if int > 15
                 */

                if (ch->abilities.intel <= 3) {
                    fjump = 2;
                    cjump = 2;
                    tjump = 2;
                    njump = 2;
                    mjump = 2;
                    kjump = 2;
                    djump = 0;
                } else if (ch->abilities.intel <= 9) {
                    fjump = 4;
                    cjump = 3;
                    tjump = 2;
                    njump = 2;
                    mjump = 1;
                    kjump = 2;
                    djump = 2;
                } else if (ch->abilities.intel <= 12) {
                    fjump = 3;
                    cjump = 3;
                    tjump = 2;
                    njump = 2;
                    mjump = 2;
                    kjump = 3;
                    djump = 2;
                } else if (ch->abilities.intel <= 15) {
                    fjump = 3;
                    cjump = 3;
                    tjump = 2;
                    njump = 2;
                    mjump = 3;
                    kjump = 2;
                    djump = 2;
                } else {
                    fjump = 3;
                    cjump = 3;
                    tjump = 2;
                    njump = 1;
                    mjump = 3;
                    kjump = 3;
                    djump = 2;
                }

                total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) +
                        (njump * ntot) + (mjump * mtot) + (djump * dtot) +
                        (kjump * ktot);

                total = (int) number(1, (int) total);

                for (vict = rp->people; vict; vict = vict->next_in_room) {
                    if (CAN_SEE(ch, vict) &&
                        !IS_SET(vict->specials.act, PLR_NOHASSLE) &&
                        !IS_AFFECTED(vict, AFF_SNEAK) && ch != vict &&
                        (!IS_SET(ch->specials.act, ACT_WIMPY) ||
                         !AWAKE(vict)) &&
                        ((vict->specials.zone != ch->specials.zone &&
                          !strchr(zone_table[ch->specials.zone].races,
                                  GET_RACE(vict))) ||
                         IS_SET(vict->specials.act, ACT_ANNOYING)) &&
                        !in_group(ch, vict)) {

                        if (IS_NPC(vict)) {
                            total -= njump;
                        } else if (HasClass(vict, CLASS_WARRIOR |
                                                  CLASS_BARBARIAN |
                                                  CLASS_PALADIN |
                                                  CLASS_RANGER)) {
                            total -= fjump;
                        } else if (HasClass(vict, CLASS_CLERIC)) {
                            total -= cjump;
                        } else if (HasClass(vict, CLASS_MAGIC_USER) ||
                                   HasClass(vict, CLASS_SORCERER)) {
                            total -= mjump;
                        } else if (HasClass(vict, CLASS_THIEF | CLASS_PSI)) {
                            total -= tjump;
                        } else if (HasClass(vict, CLASS_DRUID)) {
                            total -= djump;
                        } else if (HasClass(vict, CLASS_MONK)) {
                            total -= kjump;
                        }
                        if (total <= 0) {
                            return vict;
                        }
                    }
                }

                return ch->specials.fighting;
            }

            /*
             * THIS IS FUBAR!  Beirdo 02292004
             */
        }
    }
}
#endif

struct char_data *FindAnyVictim(struct char_data *ch)
{
    struct char_data *tmp_ch;
    unsigned char   found = FALSE;
    unsigned short  ftot = 0,
                    ttot = 0,
                    ctot = 0,
                    ntot = 0,
                    mtot = 0,
                    ktot = 0,
                    dtot = 0;
    unsigned short  total;
    unsigned short  fjump = 0,
                    njump = 0,
                    cjump = 0,
                    mjump = 0,
                    tjump = 0,
                    kjump = 0,
                    djump = 0;

    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0
        Log("No room data in FindMetaVictim ??Crash???");
#endif
        return (0);
    }
    if (ch->in_room < 0) {
        return (0);
    }
    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) &&
            (!SameRace(ch, tmp_ch) || (!IS_NPC(tmp_ch)))) {
            /*
             * a potential victim has been found
             */
            found = TRUE;
            if (!IS_NPC(tmp_ch)) {
                if (HasClass(tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                     CLASS_RANGER | CLASS_PALADIN)) {
                    ftot++;
                } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                    ctot++;
                } else if (HasClass(tmp_ch, CLASS_MAGIC_USER) ||
                           HasClass(tmp_ch, CLASS_SORCERER)) {
                    mtot++;
                } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                    ttot++;
                } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                    dtot++;
                } else if (HasClass(tmp_ch, CLASS_MONK)) {
                    ktot++;
                }
            } else {
                ntot++;
            }
        }
    }

    /*
     * if no legal enemies have been found, return 0
     */

    if (!found) {
        return (0);
    }

    /*
     * give higher priority to fighters, clerics, thieves,
     * magic users if int <= 12 give higher priority to
     * fighters, clerics, magic users thieves is inv > 12 give
     * higher priority to magic users, fighters, clerics,
     * thieves if int > 15
     */

    /*
     * choose a target
     */

    if (ch->abilities.intel <= 3) {
        fjump = 2;
        cjump = 2;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 2;
        djump = 0;
    } else if (ch->abilities.intel <= 9) {
        fjump = 4;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 1;
        kjump = 2;
        djump = 2;
    } else if (ch->abilities.intel <= 12) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 3;
        djump = 2;
    } else if (ch->abilities.intel <= 15) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 3;
        kjump = 2;
        djump = 2;
    } else {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 1;
        mjump = 3;
        kjump = 3;
        djump = 2;
    }

    total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) +
            (njump * ntot) + (mjump * mtot) + (djump * dtot) + (kjump * ktot);

    total = number(1, (int) total);

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            (!SameRace(tmp_ch, ch) || (!IS_NPC(tmp_ch)))) {
            if (IS_NPC(tmp_ch)) {
                total -= njump;
            } else if (HasClass(tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                         CLASS_PALADIN | CLASS_RANGER)) {
                total -= fjump;
            } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                total -= cjump;
            } else if (HasClass(tmp_ch, CLASS_MAGIC_USER) ||
                       HasClass(tmp_ch, CLASS_SORCERER)) {
                total -= mjump;
            } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                total -= tjump;
            } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                total -= djump;
            } else if (HasClass(tmp_ch, CLASS_MONK)) {
                total -= kjump;
            }
            if (total <= 0) {
                return (tmp_ch);
            }
        }
    }

    if (ch->specials.fighting) {
        return (ch->specials.fighting);
    }

    return (0);
}

/*
 * @Name:        CreateAMob
 * @description: Helper function for 'summoning' mobs.
 *               Will use a current mob as a focal point,
 *               and capable of giving default actions and
 *               descriptions, or specialized
 * @Author:      Rick Peplinski (Talesian)
 * @Assigned to obj/mob/room: N/A
 */
struct char_data *CreateAMob(struct char_data *mob,
                             int vmobnum, int MobAdjust, char *buf)
{
    struct char_data *mobtmp;
    /*
     *  create a mob, assign it as a follower, order it to
     *  attack a random pc in room
     *  MobAdjust bitflags 1 = make follower, 2 = guard on, 4 =
     *  attack a random char (if no char, be aggressive)
     */
    mobtmp = read_mobile(real_mobile(vmobnum), REAL);
    if (!mobtmp) {
        return NULL;
    }
    char_to_room(mobtmp, mob->in_room);
    if (*buf) {
        act(buf, TRUE, mobtmp, 0, 0, TO_ROOM);
    } else {
        act("$n suddenly springs into being!", TRUE, mobtmp, 0, 0, TO_ROOM);
    }
    if (IS_SET(MobAdjust, 1)) {
        add_follower(mobtmp, mob);
        SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
    }

    if (IS_SET(MobAdjust, 2)) {
        do_order(mob, "followers guard on", 0);
    }

    if (IS_SET(MobAdjust, 4) && AttackRandomChar(mobtmp) == NULL &&
        !IS_SET(mobtmp->specials.act, ACT_AGGRESSIVE)) {
        SET_BIT(mobtmp->specials.act, ACT_AGGRESSIVE);
    }
    return mobtmp;
}

/*
 * @Name:        AttackRandomChar
 * @description: Helper function for mobs.  Will use a
 *               current mob as a focal point, and will
 *               attack any non-immortal PC
 * @Author:      Rick Peplinski (Talesian)
 * @Assigned to obj/mob/room: N/A
 */
struct char_data *AttackRandomChar(struct char_data *mob)
{
    struct char_data *tempchar;
    int             i = 0;
    int             pctoattack;
    int             k = 1;

    for (tempchar = real_roomp(mob->in_room)->people; tempchar;
         tempchar = tempchar->next_in_room) {
        if (IS_PC(tempchar) && !IS_IMMORTAL(tempchar)) {
            i++;
        }
    }
    if (i > 0) {
        pctoattack = number(1, i);
        for (tempchar = real_roomp(mob->in_room)->people;
             tempchar; tempchar = tempchar->next_in_room) {
            if (IS_PC(tempchar) && !IS_IMMORTAL(tempchar)) {
                if (pctoattack == k) {
                    MobHit(mob, tempchar, 0);
                    return tempchar;
                }
                k++;
            }
        }
    }
    return NULL;
}

/*
 * @Name:        doroomdamage
 * @description: A helper function, to handle the odd damage
 *               things that happen in the various rooms and
 *               mobiles.  Add a damage type and the messages
 *               you want to use.  Use this for self-damaging
 *               effects, or individualized messages for odd
 *               situations.
 * @Author:      Rick Peplinski (Talesian)
 * @Assigned to obj/mob/room: N/A
 */
int doroomdamage(struct char_data *tempchar, int dam, int attacktype)
{

    if (DamCheckDeny(tempchar, tempchar, attacktype)) {
        return (FALSE);
    }

    dam = SkipImmortals(tempchar, dam, attacktype);

    dam = PreProcDam(tempchar, attacktype, dam);

    if (dam <= 0) {
        if (attacktype == SPELL_INCENDIARY_CLOUD) {
            act("The fire roars around you, but doesn't touch you.", FALSE,
                tempchar, 0, 0, TO_CHAR);
        } else if (attacktype == SPELL_BLADE_BARRIER) {
            act("The jaws try to clamp down on you, but you avoid their "
                "slashing edges.", FALSE, tempchar, 0, 0, TO_CHAR);
        } else if (attacktype == TYPE_SLASH) {
            act("The bone shards fly into you, but do not cut you.", FALSE,
                tempchar, 0, 0, TO_CHAR);
        }
        return (FALSE);
    } else if (attacktype == SPELL_INCENDIARY_CLOUD) {
        act("You are blasted by the fire, it burns unnaturally hot, like "
            "demonfire!", FALSE, tempchar, 0, 0, TO_CHAR);
    } else if (attacktype == SPELL_BLADE_BARRIER) {
        act("The jaws clamp down on you, cutting through skin, muscle, "
            "and bone!", FALSE, tempchar, 0, 0, TO_CHAR);
    } else if (attacktype == TYPE_SLASH) {
        act("The bone shards cut you badly, your flesh is being flayed "
            "from your bones!", FALSE, tempchar, 0, 0, TO_CHAR);
    }

    if (DoDamage(tempchar, tempchar, dam, attacktype)) {
        return (TRUE);
    }
    if (DamageEpilog(tempchar, tempchar, attacktype)) {
        return (TRUE);
    }
    return (FALSE);
}

void BreakLifeSaverObj(struct char_data *ch)
{
    int             found = FALSE,
                    i,
                    j;
    char            buf[200];
    struct obj_data *o;

    /*
     *  check eq for object with the effect
     */
    for (i = 0; i < MAX_WEAR && !found; i++) {
        if (ch->equipment[i]) {
            o = ch->equipment[i];
            for (j = 0; j < MAX_OBJ_AFFECT; j++) {
                if (o->affected[j].location == APPLY_SPELL &&
                    IS_SET(o->affected[j].modifier, AFF_LIFE_PROT)) {
                    found = i;
                }
            }
        }
    }

    if (found) {
        /*
         *  break the object.
         */
        sprintf(buf, "%s shatters with a blinding flash of light!\n\r",
                ch->equipment[found]->name);
        send_to_char(buf, ch);
        if ((o = unequip_char(ch, found)) != NULL) {
            MakeScrap(ch, NULL, o);
        }
    }

}

/*
 * Its void.. why does it return FALSE or TRUE???
 */
void BrittleCheck(struct char_data *ch, struct char_data *v, int dam)
{
    char            buf[200];
    struct obj_data *obj;

    if (dam <= 0) {
        return;
    }
    if (ch->equipment[WIELD] &&
        IS_OBJ_STAT(ch->equipment[WIELD], ITEM_BRITTLE) &&
        !IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM) &&
        ((obj = unequip_char(ch, WIELD)) != NULL)) {
        sprintf(buf, "%s shatters.\n\r", obj->short_description);
        send_to_char(buf, ch);
        MakeScrap(ch, v, obj);
    }
}

int PreProcDam(struct char_data *ch, int type, int dam)
{
    unsigned        Our_Bit;

    /*
     * long, intricate list, with the various bits and the
     * various spells and such determined
     */

    switch (type) {
    case SPELL_FIREBALL:
    case SPELL_FIRESHIELD:
    case SPELL_METEOR_SWARM:
    case SPELL_BURNING_HANDS:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_HEAT_STUFF:
    case SPELL_FIRESTORM:
    case SPELL_INCENDIARY_CLOUD:
    case SKILL_MIND_BURN:
        Our_Bit = IMM_FIRE;
        break;

    case SPELL_SHOCKING_GRASP:
    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
    case SPELL_CHAIN_LIGHTNING:
        Our_Bit = IMM_ELEC;
        break;

    case SPELL_CHILLSHIELD:
    case SPELL_CHILL_TOUCH:
    case SPELL_CONE_OF_COLD:
    case SPELL_ICE_STORM:
    case SPELL_FROST_BREATH:
        Our_Bit = IMM_COLD;
        break;

    case SPELL_MAGIC_MISSILE:
    case SPELL_COLOUR_SPRAY:
    case SPELL_GAS_BREATH:
    case SPELL_SUNRAY:
    case SPELL_DISINTEGRATE:
        Our_Bit = IMM_ENERGY;
        break;

    case SPELL_FINGER_OF_DEATH:
    case SPELL_LIFE_TAP:
    case SPELL_LIFE_DRAW:
    case SPELL_LIFE_LEECH:
    case SPELL_MIST_OF_DEATH:
    case SPELL_ENERGY_DRAIN:
        Our_Bit = IMM_DRAIN;
        break;

    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
        Our_Bit = IMM_ACID;
        break;

    case SKILL_BACKSTAB:
    case TYPE_PIERCE:
    case TYPE_STING:
    case TYPE_STAB:
    case TYPE_RANGE_WEAPON:
    case TYPE_IMPALE:
        Our_Bit = IMM_PIERCE;
        break;

    case TYPE_SLASH:
    case TYPE_WHIP:
    case TYPE_CLEAVE:
    case TYPE_CLAW:
    case SPELL_BLADE_BARRIER:
        Our_Bit = IMM_SLASH;
        break;

    case TYPE_BLUDGEON:
    case TYPE_HIT:
    case SKILL_KICK:
    case TYPE_CRUSH:
    case TYPE_BITE:
    case TYPE_SMASH:
    case TYPE_SMITE:
    case TYPE_BLAST:
    case TYPE_JAB:
    case TYPE_PUNCH:
    case TYPE_STRIKE:
        Our_Bit = IMM_BLUNT;
        break;

    case SPELL_SCOURGE_WARLOCK:
    case SPELL_DECAY:
    case SPELL_DISEASE:
    case SPELL_POISON:
        Our_Bit = IMM_POISON;
        break;

    default:
        return (dam);
        break;
    }

    if (Our_Bit == IMM_PIERCE &&
        ch->specials.remortclass == BARBARIAN_LEVEL_IND + 1) {
        /*
         * 25% discount on damage taken.
         */
        dam = dam - (int) (dam * GET_LEVEL(ch, BARBARIAN_LEVEL_IND)) / 200;
    }

    if (IS_SET(ch->M_immune, Our_Bit)) {
        /*
         * immune * overrides both suscept and  resist
         */
        dam = -1;
    } else if (IS_SET(ch->immune, Our_Bit) && !IS_SET(ch->susc, Our_Bit)) {
        /*
         * Make suscept cancel out resist
         */
        dam >>= 1;
        if (!(number(0, 9))) {
            send_to_char("Your resistances help to protect you from the "
                         "attack.\n\r", ch);
        }
    } else if (IS_SET(ch->susc, Our_Bit) && !IS_SET(ch->immune, Our_Bit)) {
        dam <<= 1;
        if (!(number(0, 9))) {
            send_to_char("Your susceptibility makes the attack do even more "
                         "damage.\n\r", ch);
        }
    }
    return (dam);
}

int DamageOneItem(struct char_data *ch, int dam_type, struct obj_data *obj)
{
    int             num;
    char            buf[256];
    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!IS_SET(rp->room_flags, ARENA_ROOM)) {
        num = DamagedByAttack(obj, dam_type);
        if (num != 0) {
            sprintf(buf, "%s is %s.\n\r",
                    obj->short_description, ItemDamType[dam_type - 1]);
            send_to_char(buf, ch);
            if (num == -1) {
                /*
                 * destroy object
                 */
                return (TRUE);
            } else if (DamageItem(ch, obj, num)) {
                /*
                 * "damage item" (armor), (weapon)
                 */
                return (TRUE);
            }
        }
    }
    return (FALSE);
}

void MakeScrap(struct char_data *ch, struct char_data *v, struct obj_data *obj)
{
    char            buf[200];
    struct obj_data *t,
                   *x;
    extern char     DestroyedItems;

    if (ValidRoom(ch) == FALSE) {
        return;
    }
    if (IS_SET(real_roomp(ch->in_room)->room_flags, ARENA_ROOM)) {
        return;
    }
    act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_CHAR);
    act("$p falls to the ground in scraps.", TRUE, ch, obj, 0, TO_ROOM);

    t = read_object(30, VIRTUAL);

    sprintf(buf, "Scraps from %s lie in a pile here.",
            obj->short_description);
    if (t->description) {
        free(t->description);
    }
    t->description = (char *) strdup(buf);
    if (obj->carried_by) {
        obj_from_char(obj);
    } else if (obj->equipped_by) {
        obj = unequip_char(ch, obj->eq_pos);
    }

    if (v) {
#if 0
        if (v->in_room != ch->in_room) {
            /*
             * for shooting * missles
             */
            obj_to_room(t, v->in_room);
        } else
#endif
            obj_to_room(t, ch->in_room);
    } else {
        obj_to_room(t, ch->in_room);
    }

    t->obj_flags.value[0] = 20;

    while (obj->contains) {
        x = obj->contains;
        obj_from_obj(x);
        obj_to_room(x, ch->in_room);
    }

    check_falling_obj(t, ch->in_room);
#if 0
    obj_index[obj->item_number].number--;
#endif
    extract_obj(obj);

    DestroyedItems = 1;
}

void DamageAllStuff(struct char_data *ch, int dam_type)
{
    int             j;
    struct obj_data *obj,
                   *next;

    /*
     * this procedure takes all of the items in equipment and
     * inventory and damages the ones that should be damaged
     */

    /*
     * equipment
     */
#if 0
    if (dam_type == FIRESHIELD) return;
#endif
    for (j = 0; j < MAX_WEAR; j++) {
        if (ch->equipment[j] && ch->equipment[j]->item_number >= 0) {
            obj = ch->equipment[j];
            if (DamageOneItem(ch, dam_type, obj)) {
                /*
                 * TRUE == destroyed
                 */
                if ((obj = unequip_char(ch, j)) != NULL) {
                    MakeScrap(ch, NULL, obj);
                } else {
                    Log("hmm, really wierd in DamageAllStuff!");
                }
            }
        }
    }

    /*
     * inventory
     */

    obj = ch->carrying;
    while (obj) {
        next = obj->next_content;
        if (obj->item_number >= 0 && DamageOneItem(ch, dam_type, obj)) {
            MakeScrap(ch, NULL, obj);
        }
        obj = next;
    }
}

int DamageItem(struct char_data *ch, struct obj_data *o, int num)
{
    /*
     * damage weaons or armor
     */

    if (ITEM_TYPE(o) == ITEM_ARMOR) {
        o->obj_flags.value[0] -= num;
        if (o->obj_flags.value[0] < 0) {
            return (TRUE);
        }
    } else if (ITEM_TYPE(o) == ITEM_WEAPON) {
        o->obj_flags.value[2] -= num;
        if (o->obj_flags.value[2] <= 0) {
            return (TRUE);
        }
    }
    return (FALSE);
}

int ItemSave(struct obj_data *i, int dam_type)
{
    int             num,
                    j;

    /*
     * obj fails save automatically it brittle
     */
    if (IS_OBJ_STAT(i, ITEM_BRITTLE)) {
        return (FALSE);
    }

    /*
     * this is to keep immune objects from getting dammaged
     */
    if (IS_OBJ_STAT(i, ITEM_IMMUNE)) {
        return (TRUE);
    }

    /*
     * this is to give resistant magic items a better chance
     * to save
     */
    if (IS_OBJ_STAT(i, ITEM_RESISTANT) && number(1, 100) >= 50) {
            return (TRUE);
    }

    num = number(1, 20);
    if (num <= 1) {
        return (FALSE);
    }
    if (num >= 20) {
        return (TRUE);
    }
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
        if ((i->affected[j].location == APPLY_SAVING_SPELL) ||
            (i->affected[j].location == APPLY_SAVE_ALL)) {
            num -= i->affected[j].modifier;
        }
    }

    if (i->affected[j].location != APPLY_NONE) {
        num += 1;
    }

    if (i->affected[j].location == APPLY_HITROLL) {
        num += i->affected[j].modifier;
    }

    if (ITEM_TYPE(i) != ITEM_ARMOR) {
        num += 1;
    }
    if (num <= 1) {
        return (FALSE);
    }
    if (num >= 20) {
        return (TRUE);
    }
    if (num >= ItemSaveThrows[(int) GET_ITEM_TYPE(i) - 1][dam_type - 1]) {
        return (TRUE);
    } else {
        return (FALSE);
    }
}

int DamagedByAttack(struct obj_data *i, int dam_type)
{
    int             num = 0;

    if (dam_type == FIRESHIELD) {
        /*
         * fireshield should scrap less -Lennya 20030322
         */
        if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)) {
            while (!ItemSave(i, dam_type)) {
                /*
                 * missed its save, give it another chance
                 */
                if (!ItemSave(i, dam_type)) {
                    /* failed two saves, let's dent it */
                    num += 1;
                    if (num > 75) {
                        return (num);
                    }
                }
            }
            return (num);
        } else if (ItemSave(i, dam_type)) {
            return (0);
        } else if (ItemSave(i, dam_type)) {
            /*
             * let's give it another chance to save
             */
            return (0);
        } else if (ItemSave(i, dam_type)) {
        /*
         * last chance to save
         */
            return (0);
        } else {
            /*
             * failed three saves, scrap it
             */
            return (-1);
        }
    } else if ((ITEM_TYPE(i) == ITEM_ARMOR) || (ITEM_TYPE(i) == ITEM_WEAPON)) {
        /*
         * not fireshield
         */
        while (!ItemSave(i, dam_type)) {
            num += 1;
            if (num > 75) {
                /*
                 * so anything with over 75 ac
                 * points will not be destroyed
                 */
                return (num);
            }
        }
        return (num);
    } else if (ItemSave(i, dam_type)) {
        return (0);
    } else {
        return (-1);
    }
}

/*
 * monk check for damage???
 */
int WeaponCheck(struct char_data *ch, struct char_data *v, int type, int dam)
{
    int             Immunity,
                    total,
                    j;

    Immunity = -1;
    if (IS_SET(v->M_immune, IMM_NONMAG)) {
        Immunity = 0;
    }
    if (IS_SET(v->M_immune, IMM_PLUS1)) {
        Immunity = 1;
    }
    if (IS_SET(v->M_immune, IMM_PLUS2)) {
        Immunity = 2;
    }
    if (IS_SET(v->M_immune, IMM_PLUS3)) {
        Immunity = 3;
    }
    if (IS_SET(v->M_immune, IMM_PLUS4)) {
        Immunity = 4;
    }

    if (Immunity < 0) {
        return (dam);
    }
    if ((type < TYPE_HIT) || (type > TYPE_STRIKE)) {
        return (dam);
    } else if (type == TYPE_HIT || IS_NPC(ch)) {
        if (GetMaxLevel(ch) > ((Immunity + 1) * (Immunity + 1)) + 6 ||
            ((HasClass(ch, CLASS_BARBARIAN) || HasClass(ch, CLASS_MONK)) &&
             BarbarianToHitMagicBonus(ch) >= Immunity)) {
            return (dam);
        } else {
            act("$N ignores your puny attack", FALSE, ch, 0, v, TO_CHAR);
            return (0);
        }
    } else {
        total = 0;
        if (!ch->equipment[WIELD] && !HasClass(ch, CLASS_MONK)) {
            return (0);
        } else if (!ch->equipment[WIELD] && HasClass(ch, CLASS_MONK)) {
            total = BarbarianToHitMagicBonus(ch);
            if (total > Immunity) {
                return (dam);
            } else {
                act("$N ignores your puny fists.", FALSE, ch, 0, v, TO_CHAR);
                return (0);
            }
        }

        for (j = 0; j < MAX_OBJ_AFFECT; j++) {
            if ((ch->equipment[WIELD]->affected[j].location == APPLY_HITROLL) ||
                (ch->equipment[WIELD]->affected[j].location == APPLY_HITNDAM)) {
                total += ch->equipment[WIELD]->affected[j].modifier;
            }
        }

        if (HasClass(ch, CLASS_BARBARIAN) &&
            BarbarianToHitMagicBonus(ch) > total) {
            total = BarbarianToHitMagicBonus(ch);
        }

        if (total > Immunity) {
            return (dam);
        } else {
            act("$N ignores your puny weapon", FALSE, ch, 0, v, TO_CHAR);
            return (0);
        }
    }
}

void DamageStuff(struct char_data *v, int type, int dam)
{
    int             num,
                    dam_type;
    struct obj_data *obj;

    /*
     * add a check for anti-magic shell or some other item
     * protection spell right here I would think
     */

    if (type >= TYPE_HIT && type <= TYPE_STRIKE) {
        /*
         * wear_neck through hold
         */
        num = number(3, 17);
        if (v->equipment[num] &&
            ((type == TYPE_BLUDGEON && dam > 10) ||
             (type == TYPE_CRUSH && dam > 5) ||
             (type == TYPE_SMASH && dam > 10) ||
             (type == TYPE_BITE && dam > 15) ||
             (type == TYPE_CLAW && dam > 20) ||
             (type == TYPE_SLASH && dam > 30) ||
             (type == TYPE_SMITE && dam > 10) ||
             (type == TYPE_JAB && dam > 20) ||
             (type == TYPE_PUNCH && dam > 20) ||
             (type == TYPE_STRIKE && dam > 20) ||
             (type == TYPE_HIT && dam > 20)) &&
            DamageOneItem(v, BLOW_DAMAGE, v->equipment[num]) &&
            (obj = unequip_char(v, num)) != NULL) {

            MakeScrap(v, NULL, obj);
        }
    } else {
        dam_type = GetItemDamageType(type);
        if (dam_type) {
            /*
             * as this number increases or decreases the chance of item
             * damage decreases or increases
             */
            num = number(1, 50);
            if (dam >= num) {
                DamageAllStuff(v, dam_type);
            }
        }
    }
}

int GetItemDamageType(int type)
{
    switch (type) {
    case SPELL_FIRESHIELD:
        return (FIRESHIELD);
        break;
    case SPELL_CHILLSHIELD:
        return (CHILLSHIELD);
        break;
    case SPELL_BLADE_BARRIER:
        return (BLADE_BARRIER);
        break;

    case SPELL_FIREBALL:
    case SPELL_METEOR_SWARM:
    case SPELL_FLAMESTRIKE:
    case SPELL_FIRE_BREATH:
    case SPELL_INCENDIARY_CLOUD:
    case SKILL_MIND_BURN:
        return (FIRE_DAMAGE);
        break;

    case SPELL_LIGHTNING_BOLT:
    case SPELL_CALL_LIGHTNING:
    case SPELL_LIGHTNING_BREATH:
        return (ELEC_DAMAGE);
        break;

    case SPELL_CONE_OF_COLD:
    case SPELL_ICE_STORM:
    case SPELL_FROST_BREATH:
        return (COLD_DAMAGE);
        break;

    case SPELL_COLOUR_SPRAY:
    case SPELL_GAS_BREATH:
    case SPELL_DISINTEGRATE:
        return (BLOW_DAMAGE);
        break;

    case SPELL_ACID_BREATH:
    case SPELL_ACID_BLAST:
        return (ACID_DAMAGE);
    default:
        return (0);
        break;
    }

}

int SkipImmortals(struct char_data *v, int amnt, int attacktype)
{
    /*
     * You can't damage an immortal!
     */
    if ((GetMaxLevel(v) > MAX_MORT) && !IS_NPC(v)) {
        amnt = 0;
    }
    /*
     * special type of monster
     */
    if (IS_NPC(v) && (IS_SET(v->specials.act, ACT_IMMORTAL))) {
        amnt = -1;
    }
#if 1
    if (IS_PC(v) && IS_LINKDEAD(v) &&
        (attacktype == TYPE_SUFFERING || attacktype == SPELL_DECAY ||
         attacktype == SPELL_DISEASE || attacktype == SPELL_POISON ||
         attacktype == SPELL_HEAT_STUFF)) {
        /*
         * link dead pc, no damage
         */
        amnt = -1;
    }
#endif

    return (amnt);

}

void WeaponSpell(struct char_data *c,
                 struct char_data *v, struct obj_data *obj, int type)
{
    int             j,
                    index,
                    num;
    struct obj_data *weapon;

    if (((c->in_room == v->in_room && GET_POS(v) != POSITION_DEAD) ||
         (GET_POS(v) != POSITION_DEAD && type == TYPE_RANGE_WEAPON)) &&
        ((c->equipment[WIELD] && type >= TYPE_BLUDGEON && type <= TYPE_SMITE) ||
         (type == TYPE_RANGE_WEAPON && obj))) {

        if (type == TYPE_RANGE_WEAPON) {
            weapon = obj;
        } else {
            weapon = c->equipment[WIELD];
        }

        for (j = 0; j < MAX_OBJ_AFFECT; j++) {
            if (weapon->affected[j].location == APPLY_WEAPON_SPELL) {
                num = weapon->affected[j].modifier;
                if (num <= 0) {
                    num = 1;
                }
                index = spell_index[num];
                if( index != -1 && spell_info[index].spell_pointer ) {
                    ((*spell_info[index].spell_pointer)
                      (6, c, "", SPELL_TYPE_WAND, v, 0));
                }
            }
        }
    }
}


struct char_data *FindAnAttacker(struct char_data *ch)
{
    struct room_data *rp;
    struct char_data *tmp_ch;
    unsigned char   found = FALSE;
    unsigned short  ftot = 0,
                    ttot = 0,
                    ctot = 0,
                    ntot = 0,
                    mtot = 0,
                    ktot = 0,
                    dtot = 0;
    unsigned short  total;
    unsigned short  fjump = 0,
                    njump = 0,
                    cjump = 0,
                    mjump = 0,
                    tjump = 0,
                    kjump = 0,
                    djump = 0;

    if (ch->in_room < 0) {
        return (0);
    }
    if (!ch) {
        Log("Findanattacker!!!");
        return (0);
    }

    if (ch->in_room < 0) {
        return (0);
    }
    rp = real_roomp(ch->in_room);
    if (!rp) {
        return (0);
    }

    tmp_ch = rp->people;
    if (!tmp_ch) {
        return (0);
    }

    while (tmp_ch) {
        if (ch != tmp_ch) {
            if (tmp_ch->specials.fighting == ch) {
                /*
                 * a potential victim has been found
                 */
                found = TRUE;
                if (!IS_NPC(tmp_ch)) {
                    if (HasClass(tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                         CLASS_PALADIN | CLASS_RANGER)) {
                        ftot++;
                    } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                        ctot++;
                    } else if (HasClass(tmp_ch, CLASS_MAGIC_USER)
                             || HasClass(tmp_ch, CLASS_SORCERER)) {
                        mtot++;
                    } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                        ttot++;
                    } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                        dtot++;
                    } else if (HasClass(tmp_ch, CLASS_MONK)) {
                        ktot++;
                    }
                } else {
                    ntot++;
                }
            }
        }
        tmp_ch = tmp_ch->next_in_room;
    }

    /*
     * if no legal enemies have been found, return 0
     */

    if (!found) {
        return (0);
    }

    /*
     * give higher priority to fighters, clerics, thieves,
     * magic users if int <= 12 give higher priority to
     * fighters, clerics, magic users thieves is inv > 12 give
     * higher priority to magic users, fighters, clerics,
     * thieves if int > 15
     */

    /*
     * choose a target
     */

    if (ch->abilities.intel <= 3) {
        fjump = 2;
        cjump = 2;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 2;
        djump = 0;
    } else if (ch->abilities.intel <= 9) {
        fjump = 4;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 1;
        kjump = 2;
        djump = 2;
    } else if (ch->abilities.intel <= 12) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 2;
        kjump = 3;
        djump = 2;
    } else if (ch->abilities.intel <= 15) {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 2;
        mjump = 3;
        kjump = 2;
        djump = 2;
    } else {
        fjump = 3;
        cjump = 3;
        tjump = 2;
        njump = 1;
        mjump = 3;
        kjump = 3;
        djump = 2;
    }

    total = (fjump * ftot) + (cjump * ctot) + (tjump * ttot) +
            (njump * ntot) + (mjump * mtot) + (djump * dtot) + (kjump * ktot);

    total = number(1, (int) total);

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (tmp_ch->specials.fighting == ch) {
            if (IS_NPC(tmp_ch)) {
                total -= njump;
            } else if (HasClass (tmp_ch, CLASS_WARRIOR | CLASS_BARBARIAN |
                                         CLASS_PALADIN | CLASS_RANGER)) {
                total -= fjump;
            } else if (HasClass(tmp_ch, CLASS_CLERIC)) {
                total -= cjump;
            } else if (HasClass(tmp_ch, CLASS_MAGIC_USER) ||
                       HasClass(tmp_ch, CLASS_SORCERER)) {
                total -= mjump;
            } else if (HasClass(tmp_ch, CLASS_THIEF | CLASS_PSI)) {
                total -= tjump;
            } else if (HasClass(tmp_ch, CLASS_DRUID)) {
                total -= djump;
            } else if (HasClass(tmp_ch, CLASS_MONK)) {
                total -= kjump;
            }
            if (total <= 0) {
                return (tmp_ch);
            }
        }
    }

    if (ch->specials.fighting) {
        return (ch->specials.fighting);
    }
    return (0);
}

void shoot(struct char_data *ch, struct char_data *victim)
{
#if 0
    struct obj_data *bow,
                   *arrow;
    int             oldth,
                    oldtd;
    int             tohit = 0,
                    todam = 0;

    /*
     * check for bow and arrow.
     */

    bow = ch->equipment[HOLD];
    arrow = ch->equipment[WIELD];

    /*
     * this is checked in do_shoot now
     */
    if (!bow) {
        send_to_char("You need a missile weapon (like a bow)\n\r", ch);
        return;
    } else if (!arrow) {
        send_to_char("You need a projectile to shoot!\n\r", ch);
    } else if (!bow && !arrow) {
        send_to_char
            ("You need a bow-like item, and a projectile to shoot!\n\r",
             ch);
    } else {
        /*
         * for bows:  value[0] = arror type
         *            value[1] = type 0=short,1=med,2=longranged
         *            value[2] = + to hit
         *            value[3] = + to damage
         */

        if (bow->obj_flags.value[0] != arrow->obj_flags.value[0]) {
            send_to_char
                ("That projectile does not fit in that projector.\n\r",
                 ch);
            return;
        }

        /*
         * check for bonuses on the bow.
         */
        tohit = bow->obj_flags.value[2];
        todam = bow->obj_flags.value[3];

        /*
         * temporarily remove other stuff and add bow bonuses.
         */
        oldth = GET_HITROLL(ch);
        oldtd = GET_DAMROLL(ch);
        /*
         * figure range mods for this weapon
         */
        if (victim->in_room != ch->in_room)
            switch (bow->obj_flags.value[1]) {
            case 0:
                tohit -= 4;     /* short range weapon -4 to hit */
                break;
            case 1:
                tohit -= 3;     /* med range weapon -3 to hit */
                break;
            case 2:
                tohit -= 2;     /* long range weapon -2 to hit */
                break;
            default:
                tohit -= 1;     /* unknown, default to -1 tohit */
                break;
            }

        /*
         * end switch
         */
        /*
         * set tohit and dam to bows only, lets not use
         * cumalitive of what
         */
        /*
         * they already have
         */
        GET_HITROLL(ch) = tohit;
        GET_DAMROLL(ch) = todam;

        act("$n shoots $p at $N!", FALSE, ch, arrow, victim, TO_NOTVICT);
        act("$n launches $p at you", FALSE, ch, arrow, victim, TO_VICT);
        act("You shoot at $N with $p", FALSE, ch, arrow, victim, TO_CHAR);

        /*
         * fire the weapon.
         */
        MissileHit(ch, victim, TYPE_UNDEFINED);

        GET_HITROLL(ch) = oldth;
        GET_DAMROLL(ch) = oldtd;

    }

    slog("end shoot, fight.c");

#endif
}

struct char_data *FindMetaVictim(struct char_data *ch)
{
    struct char_data *tmp_ch;
    unsigned char   found = FALSE;
    unsigned short  total = 0;

    struct room_data *rp;

    rp = real_roomp(ch->in_room);
    if (!rp) {
#if 0
        Log("No room data in FindMetaVictim ??Crash???");
#endif
        return (0);
    }

    if (ch->in_room < 0)
        return (0);

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            (!(IS_AFFECTED(ch, AFF_CHARM)) || (ch->master != tmp_ch)) &&
            !SameRace(ch, tmp_ch)) {

            found = TRUE;
            total++;
        }
    }

    /*
     * if no legal enemies have been found, return 0
     */

    if (!found) {
        return (0);
    }

    total = number(1, (int) total);

    for (tmp_ch = (real_roomp(ch->in_room))->people; tmp_ch;
         tmp_ch = tmp_ch->next_in_room) {
        if (CAN_SEE(ch, tmp_ch) &&
            !IS_SET(tmp_ch->specials.act, PLR_NOHASSLE) &&
            !SameRace(tmp_ch, ch)) {

            total--;
            if (total == 0) {
                return (tmp_ch);
            }
        }
    }

    if (ch->specials.fighting) {
        return (ch->specials.fighting);
    }
    return (0);

}

/*
 * returns, extracts, switches etc.. anyone.
 */
void NailThisSucker(struct char_data *ch)
{

    struct char_data *pers;
    long            room_num;
    char            buf[256];
    struct room_data *rp;
    struct obj_data *obj,
                   *next_o;

    rp = real_roomp(ch->in_room);
    room_num = ch->in_room;

    if (IS_SET(RM_FLAGS(ch->in_room), ARENA_ROOM)) {
        sprintf(buf, "%s has been thrown out of the ARENA!\n\r", GET_NAME(ch));
        send_to_all(buf);
        raw_kill_arena(ch);
        return;
    }

    death_cry(ch);

    if (IS_NPC(ch) && (IS_SET(ch->specials.act, ACT_POLYSELF))) {
        /*
         *   take char from storage, to room
         */
        pers = ch->desc->original;
        char_from_room(pers);
        char_to_room(pers, ch->in_room);
        SwitchStuff(ch, pers);
        extract_char(ch);
        ch = pers;
    }
    zero_rent(ch);
    extract_char(ch);

    /*
     * delete EQ dropped by them if room was a DT
     */
    if (IS_SET(rp->room_flags, DEATH)) {
        sprintf(buf, "%s hit a DeathTrap in room %s[%ld]\r\n",
                GET_NAME(ch), real_roomp(room_num)->name, room_num);
        Log(buf);
        for (obj = real_roomp(room_num)->contents; obj; obj = next_o) {
            next_o = obj->next_content;
#if 0
            obj_index[obj->item_number].number--;
            /*
             * added for maxxes (GH)
             */
#endif
            extract_obj(obj);
        }
    }
}

int GetFormType(struct char_data *ch)
{
    int             num;

    num = number(1, 100);

    /*
     * PC monks should do the monk thing, regardless of race
     * -Lennya
     */
    if (HasClass(ch, CLASS_MONK) && IS_PC(ch)) {
        switch (number(1, 5)) {
        case 1:
            return (TYPE_HIT);
        case 2:
            return (TYPE_SMASH);
        case 3:
            return (TYPE_JAB);
        case 4:
            return (TYPE_PUNCH);
        case 5:
            return (TYPE_STRIKE);
        default:
            return (TYPE_HIT);
        }
    }

    switch (GET_RACE(ch)) {
    case RACE_REPTILE:
        if (num <= 50) {
            return (TYPE_CLAW);
        } else {
            return (TYPE_BITE);
        }
        break;
    case RACE_LYCANTH:
    case RACE_DRAGON:
    case RACE_DRAGON_RED:
    case RACE_DRAGON_BLACK:
    case RACE_DRAGON_GREEN:
    case RACE_DRAGON_WHITE:
    case RACE_DRAGON_BLUE:
    case RACE_DRAGON_SILVER:
    case RACE_DRAGON_GOLD:
    case RACE_DRAGON_BRONZE:
    case RACE_DRAGON_COPPER:
    case RACE_DRAGON_BRASS:
    case RACE_PREDATOR:
    case RACE_LABRAT:
        if (num <= 33) {
            return (TYPE_BITE);
        } else {
            return (TYPE_CLAW);
        }
        break;
    case RACE_INSECT:
        if (num <= 50) {
            return (TYPE_BITE);
        } else {
            return (TYPE_STING);
        }
        break;
    case RACE_ARACHNID:
    case RACE_DINOSAUR:
    case RACE_FISH:
    case RACE_SNAKE:
        return (TYPE_BITE);
        break;
    case RACE_BIRD:
    case RACE_SKEXIE:
        return (TYPE_CLAW);
        break;
    case RACE_GIANT:
    case RACE_GIANT_HILL:
    case RACE_GIANT_FROST:
    case RACE_GIANT_FIRE:
    case RACE_GIANT_CLOUD:
    case RACE_GIANT_STORM:
    case RACE_GIANT_STONE:
    case RACE_GOLEM:
        return (TYPE_BLUDGEON);
        break;
    case RACE_DEMON:
    case RACE_DEVIL:
    case RACE_TROLL:
    case RACE_TROGMAN:
    case RACE_LIZARDMAN:
        return (TYPE_CLAW);
        break;
    case RACE_TREE:
        return (TYPE_SMITE);
        break;
    case RACE_MFLAYER:
        if (num <= 60) {
            return (TYPE_WHIP);
        } else if (num < 80) {
            return (TYPE_BITE);
        } else {
            return (TYPE_HIT);
        }
        break;
    case RACE_PRIMATE:
        if (num <= 70) {
            return (TYPE_HIT);
        } else {
            return (TYPE_BITE);
        }
        break;
    case RACE_TYTAN:
        return (TYPE_BLAST);
        break;
    default:
        if (HasClass(ch, CLASS_MONK)) {
            switch (number(1, 5)) {
            case 1:
                return (TYPE_HIT);
            case 2:
                return (TYPE_SMASH);
            case 3:
                return (TYPE_JAB);
            case 4:
                return (TYPE_PUNCH);
            case 5:
                return (TYPE_STRIKE);
            default:
                return (TYPE_HIT);
            }
        } else
            return (TYPE_HIT);
    }
}

int MonkDodge(struct char_data *ch, struct char_data *v, int *dam)
{
    int             x = 0;
    if (v->style == FIGHTING_STYLE_DEFENSIVE
        && (v->specials.remortclass == MONK_LEVEL_IND + 1)) {
        if (FSkillCheck(v, FIGHTING_STYLE_DEFENSIVE)) {
            x = v->skills[SKILL_DODGE].learned * 2.5;
        }
    }

    if (number(1, 20000) < (x == 0 ? v->skills[SKILL_DODGE].learned : x) *
                           GET_LEVEL(v, MONK_LEVEL_IND)) {
        *dam = -3;
        act("You dodge the attack.", FALSE, ch, 0, v, TO_VICT);
        act("$N dodges $n's attack.", FALSE, ch, 0, v, TO_NOTVICT);
        if (IS_PC(ch) && (GET_EXP(ch) > 200000000 ||
                          IS_SET(ch->specials.act, PLR_LEGEND))) {
            act("$N dodges your attack. $c000Y($c000W0$c000Y)",
                FALSE, ch, 0, v, TO_CHAR);
        } else {
            act("$N dodges your attack.", FALSE, ch, 0, v, TO_CHAR);
        }
    } else {
        *dam -= GET_LEVEL(ch, MONK_LEVEL_IND) / 10;
    }

    return (0);
}

int SmithShield(struct char_data *ch,
                struct char_data *v, struct obj_data *obj, int *dam)
{

    if (number(1, 180) <= GET_DEX(v)) {
        *dam = -2;
        act("$c000WYour shield seems to have a mind of its own and shifts to "
            "your defense.", FALSE, ch, 0, v, TO_VICT);
        act("$c000W$N uses $S $p to block $n's hit.", FALSE, ch, obj, v,
            TO_NOTVICT);
        if (IS_PC(ch) && (GET_EXP(ch) > 200000000 ||
                          IS_SET(ch->specials.act, PLR_LEGEND))) {
            act("$c000W$N uses $S $p to block your hit. $c000Y($c000W0"
                "$c000Y)", FALSE, ch, obj, v, TO_CHAR);
        } else {
            act("$c000W$N uses $S $p to block your hit.",
                FALSE, ch, obj, v, TO_CHAR);
        }
    }
    return (0);
}

int BarbarianToHitMagicBonus(struct char_data *ch)
{
    int             bonus = 0;

    if (GetMaxLevel(ch) <= 7) {
        bonus = 1;
    } else if (GetMaxLevel(ch) <= 12) {
        bonus = 2;
    } else if (GetMaxLevel(ch) <= 20) {
        bonus = 3;
    } else if (GetMaxLevel(ch) <= 28) {
        bonus = 4;
    } else if (GetMaxLevel(ch) <= 35) {
        bonus = 5;
    } else if (GetMaxLevel(ch) > 35) {
        bonus = 5;
    }
    return (bonus);
}

int berserkthaco(struct char_data *ch)
{
    if (GetMaxLevel(ch) <= 10) {
        /*
         * -5 to hit when * berserked
         */
        return (5);
    }

    if (GetMaxLevel(ch) <= 25) {
        /*
         * -3
         */
        return (3);
    }

    if (GetMaxLevel(ch) <= 40) {
        /*
         * -2
         */
        return (2);
    }
    return (2);
}

int berserkdambonus(struct char_data *ch, int dam)
{
    if (GetMaxLevel(ch) <= 10) {
        /*
         * 1.33 dam when berserked
         */
        return ((int) dam * 1.33);
    } else if (GetMaxLevel(ch) <= 25) {
        /*
         * 1.5
         */
        return ((int) dam * 1.5);
    } else if (GetMaxLevel(ch) <= 40) {
        /*
         * 1.7
         */
        return ((int) dam * 1.7);
    } else {
        return ((int) dam * 1.7);
    }
}

int range_hit(struct char_data *ch, struct char_data *targ, int rng, struct
              obj_data *missile, int tdir, int max_rng)
{
    int             calc_thaco,
                    i,
                    dam = 0,
                    diceroll,
                    victim_ac;
    char           *dir_name[] = {
        "the north",
        "the east",
        "the south",
        "the west",
        "above",
        "below"
    };
    int             rmod, cdir, rang, cdr;
    char            buf[MAX_STRING_LENGTH];

    /*
     * Returns 1 on a hit, 0 otherwise
     * Does the roll, damage, messages, and everything
     */

    if (!IS_NPC(ch)) {
        calc_thaco = 20;
        for (i = 1; i < 5; i++) {
            if (thaco[i - 1][GetMaxLevel(ch)] < calc_thaco) {
                calc_thaco = thaco[i - 1][GetMaxLevel(ch)];
            }
        }
    } else {
        /*
         * THAC0 for monsters is set in the HitRoll
         */
        calc_thaco = 20;
    }

    calc_thaco -= GET_HITROLL(ch);
    rmod = 20 * rng / max_rng;
    calc_thaco += rmod;
    if (GET_POS(targ) == POSITION_SITTING) {
        calc_thaco += 7;
    }
    if (GET_POS(targ) == POSITION_RESTING) {
        calc_thaco += 10;
    }

    diceroll = number(1, 20);
    victim_ac = GET_AC(targ) / 10;

    if (AWAKE(targ)) {
        victim_ac += dex_app[(int) GET_DEX(targ)].defensive;
    }
    victim_ac = MAX(-10, victim_ac);
    /*
     * -10 is lowest
     */

    if (diceroll < 20 &&
        (diceroll == 1 || calc_thaco - diceroll > victim_ac)) {
        /*
         * Missed!
         */
        if (rng > 0) {
            sprintf(buf, "$p from %s narrowly misses $n!",
                    dir_name[opdir(tdir)]);
            act(buf, FALSE, targ, missile, 0, TO_ROOM);
            act("$p misses $N.", TRUE, ch, missile, targ, TO_CHAR);
        } else {
            act("$p narrowly misses $n!", FALSE, targ, missile, 0, TO_ROOM);
        }
        if (AWAKE(targ)) {
            if (rng > 0) {
                sprintf(buf, "$p whizzes past from %s, narrowly missing you!",
                        dir_name[opdir(tdir)]);
                act(buf, TRUE, targ, missile, 0, TO_CHAR);
            } else {
                act("$n fires $p at you, narrowly missing!",
                    TRUE, ch, missile, targ, TO_VICT);
            }
            if (IS_NPC(targ)) {
                if (rng == 0) {
                    hit(targ, ch, TYPE_UNDEFINED);
                } else {

#if 1
                    cdir = can_see_linear(targ, ch, &rang, &cdr);
                    if (!(targ->specials.charging) && number(1, 10) < 4 &&
                        cdir != -1 && GET_POS(targ) == POSITION_STANDING) {
                        /*
                         * Ain't gonna take any more of this missile crap!
                         */
                        AddHated(targ, ch);
                        act("$n roars angrily and charges!", TRUE, targ, 0, 0,
                            TO_ROOM);
                        targ->specials.charging = ch;
                        targ->specials.charge_dir = cdr;
                    }
#endif
                }
            }
        }
        return 0;
    } else {
        dam += dice(missile->obj_flags.value[1], missile->obj_flags.value[2]);
        dam = MAX(1, dam);
        AddHated(targ, ch);
        sprintf(buf, "$p from %s hits $n!", dir_name[opdir(tdir)]);
        act("$p hits $N!", TRUE, ch, missile, targ, TO_CHAR);
        act(buf, TRUE, targ, missile, 0, TO_ROOM);
        sprintf(buf, "$p from %s hits you!", dir_name[opdir(tdir)]);
        act(buf, TRUE, targ, missile, 0, TO_CHAR);
        damage(ch, targ, dam, TYPE_RANGE_WEAPON);

        if ((!targ) || (GET_HIT(targ) < 1)) {
            return 1;
        } else if (ch->in_room != targ->in_room) {
            WeaponSpell(ch, targ, missile, TYPE_RANGE_WEAPON);
        }

#if 1
        if (GET_POS(targ) != POSITION_FIGHTING &&
            GET_POS(targ) > POSITION_STUNNED && IS_NPC(targ) &&
            !targ->specials.charging) {

            GET_POS(targ) = POSITION_STANDING;
            if (rng == 0) {
                hit(targ, ch, TYPE_UNDEFINED);
            } else {
                cdir = can_see_linear(targ, ch, &rang, &cdr);
                if (cdir != -1) {
                    act("$n roars angrily and charges!", TRUE, targ, 0, 0,
                        TO_ROOM);
                    targ->specials.charging = ch;
                    targ->specials.charge_dir = cdr;
                }
            }
        }
#endif
        return 1;
    }
}

void raw_kill_arena(struct char_data *ch)
{
    death_cry(ch);
    char_from_room(ch);
    send_to_char("Some mystical powers save you from death!", ch);

    char_to_room(ch, ch->specials.start_room);
    if (affected_by_spell(ch, SPELL_PARALYSIS)) {
        affect_from_char(ch, SPELL_PARALYSIS);
    }
    if (affected_by_spell(ch, SPELL_CHILL_TOUCH)) {
        affect_from_char(ch, SPELL_CHILL_TOUCH);
    }
    spell_dispel_magic(60, ch, ch, 0);
    GET_MANA(ch) = GET_MAX_MANA(ch);
    GET_HIT(ch) = GET_MAX_HIT(ch);
    GET_MOVE(ch) = GET_MAX_MOVE(ch);
    GET_POS(ch) = POSITION_SLEEPING;
    return;
}

void BurnWings(struct char_data *ch)
{
    struct affected_type af;

    if (IS_SET(ch->immune, IMM_FIRE)) {
        return;
    }

    if (IS_SET(ch->M_immune, IMM_FIRE) && dice(1, 2) > 1) {
            return;
    }

    if (GET_RACE(ch) == RACE_AVARIEL) {
        /*
         * Races with fire prone wings
         */
        if (IS_AFFECTED2(ch, AFF2_WINGSBURNED)) {
            affect_from_char(ch, COND_WINGS_BURNED);
        }

        send_to_char("The flames burn a swath of feathers from your wings!\n\r",
                     ch);
        act("$n has the feathers seared from $ss wings!\n\r", FALSE, ch, 0, 0,
            TO_ROOM);
        af.type = COND_WINGS_BURNED;
        af.location = APPLY_BV2;
        af.modifier = 0;
        af.duration = 536;
        af.bitvector = AFF2_WINGSBURNED;
        affect_to_char(ch, &af);
        if (affected_by_spell(ch, COND_WINGS_FLY)) {
            affect_from_char(ch, COND_WINGS_FLY);
            if (!IS_AFFECTED(ch, AFF_FLYING)) {
                send_to_char("Your wings can't support you and you plummet "
                             "towards the ground!", ch);
                act("$n's wings can't support him and he plummets towards the "
                    "ground!", FALSE, ch, 0, 0, TO_ROOM);
            }
            check_falling(ch);
        }
    }
}

void specdamage(struct char_data *ch, struct char_data *v)
{
    struct obj_data *object;

    if (affected_by_spell(ch, SPELL_VAMPIRIC_EMBRACE) && ch != v &&
        !IsUndead(v)) {
        if (HasClass(ch, CLASS_WARRIOR)) {
            if (number(1, 100) < 9) {
                vampiric_embrace(ch, v);
            }
        } else {
            if (number(1, 100) < 18) {
                vampiric_embrace(ch, v);
            }
        }
    }

    if (mob_index[ch->nr].virtual == BAHAMUT) {
        bahamut_prayer(ch, v);
    }
    if (ch->equipment[WEAR_BODY]) {
        object = ch->equipment[WEAR_BODY];
        if (obj_index[object->item_number].virtual == BAHAMUT_ARMOR) {
            bahamut_armor(ch, v);
        }
    }

    if (ch->equipment[WIELD]) {
        object = ch->equipment[WIELD];
        if (obj_index[object->item_number].virtual == GUARDIAN_SIN) {
            guardian_sin(ch, v);
        }
    }
}

int FSkillCheck(struct char_data *ch, int fskill)
{
    int             perc = 0,
                    tmp = 0,
                    max = 95;

    if (!ch) {
        return (FALSE);
    }
    if (!fskill) {
        return (FALSE);
    }
    /*
     * convert from bit to skill def
     */
    fskill = fskill + 298;
    perc = number(1, 110);

    /*
     * always 9% chance of failure
     */
    if (perc > 100) {
        return (FALSE);
    }

    /*
     * add some class modifiers to perc here
     */
    switch (fskill) {
    case FIGHTING_STYLE_BERSERKED:{
            perc += 50;
            if (HasClass(ch, CLASS_BARBARIAN)) {
                perc -= GetMaxLevel(ch);
            }
        }
        break;
    case FIGHTING_STYLE_EVASIVE:{
            if (HasClass(ch, CLASS_THIEF)) {
                tmp = GetMaxLevel(ch) - 25;
                if (!tmp < 0) {
                    perc -= tmp;
                }
            }
        }
        break;
    case FIGHTING_STYLE_DEFENSIVE:{
            if (HasClass(ch, CLASS_MONK)) {
                tmp = GetMaxLevel(ch) - 25;
                if (!tmp < 0) {
                    perc -= tmp;
                }
            }
        }
        break;
    case FIGHTING_STYLE_AGGRESSIVE:{
            if (HasClass(ch, CLASS_THIEF)) {
                perc += 20;
            }
        }
        break;
    }

    if (perc > ch->skills[fskill].learned) {
        /* fail */
        if (ch->skills[fskill].learned > 75 &&
            ch->skills[fskill].learned < max) {
            /*
             * only learn if skill is high enough
             */
            if (number(1, 10) == 1) {
                /*
                 * number(1, 96) >  ch->skills[fskill].learned &&
                 */
                send_to_char("You notice improvement with fighting in your "
                             "current style.\n\r", ch);
                ch->skills[fskill].learned += 1;

                if (ch->skills[fskill].learned >= max) {
                    send_to_char("You are now a master of fighting in your "
                                 "current style!\n\r", ch);
                }
            }
        }
        return (FALSE);
    } else {
        /*
         * skill successful
         */
        return (TRUE);
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
