#ifndef _oldutils_h
#define _oldutils_h

#if DEBUG_FREE

#define free(obj) fprintf(stderr, "freeing %d\n", sizeof(*obj));\
                      free(obj)

#endif

#define STATE(d) ((d)->connected)
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

#define URANGE(a, b, c)          ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))

#define TRUE  1

#define FALSE 0

#define LOWER(c) (((c)>='A'  && (c) <= 'Z') ? ((c)+('a'-'A')) : (c))

#define UPPER(c) (((c)>='a'  && (c) <= 'z') ? ((c)+('A'-'a')) : (c) )

#define ISNEWL(ch) ((ch) == '\n' || (ch) == '\r')

#define IS_WEAPON(o) (o->type_flag == ITEM_TYPE_WEAPON)

#define IS_RARE(obj) (IS_SET(obj->extra_flags, ITEM_RARE))

#define IF_STR(st) ((st) ? (st) : "\0")

#define CAP(st)  (*(st) = UPPER(*(st)), st)

#if 0
#define CREATE(result, type, number)  do {\
        if (!((result) = (type *) calloc ((number), sizeof(type))))\
                { perror("malloc failure"); abort(); }\
                } while(0)

#define RECREATE(result,type,number) do {\
  if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
                { perror("realloc failure"); abort(); } \
                } while(0)
#endif

#define SWITCH(a,b) { (a) ^= (b); \
                      (b) ^= (a); \
                      (a) ^= (b); }

#if 0
#define IS_DARK(room) \
    (!IS_SET(roomFindNum(room)->room_flags, EVER_LIGHT) && \
     roomFindNum(room)->light <= 0 && \
    ((IS_SET(roomFindNum(room)->room_flags, DARK)) || roomFindNum(room)->dark))

#define IS_LIGHT(room) \
    (IS_SET(roomFindNum(room)->room_flags, EVER_LIGHT) || \
     roomFindNum(room)->light > 0 || \
     (!IS_SET(roomFindNum(room)->room_flags, DARK) || \
      !roomFindNum(room)->dark))
#else

#define IS_DARK(room) \
    (!IS_SET(roomFindNum(room)->room_flags, EVER_LIGHT) && \
     (roomFindNum(room)->light <= 0 && \
      (IS_SET(roomFindNum(room)->room_flags, DARK) || \
       (IsDarkOutside(roomFindNum(room))))))

#define IS_LIGHT(room) \
    (IS_SET(roomFindNum(room)->room_flags, EVER_LIGHT) || \
     roomFindNum(room)->light > 0 || \
     (!IS_SET(roomFindNum(room)->room_flags, DARK) && \
      !IsDarkOutside(roomFindNum(room))))

#endif

#define SET_BIT(var,bit)  ((var) = (var) | (bit))

#define REMOVE_BIT(var,bit)  ((var) = (var) & ~(bit) )

#define RM_FLAGS(i)  ((roomFindNum(i))?roomFindNum(i)->room_flags:0)

#define GET_LEVEL(ch, i)   ((ch)->player.level[(i)])

#define GET_REQ(i) (i<2  ? "Awful" :(i<4  ? "Bad"     :(i<7  ? "Poor"      :\
(i<10 ? "Average" :(i<14 ? "Fair"    :(i<20 ? "Good"    :(i<24 ? "Very good" :\
        "Superb" )))))))

#define HSHR(ch) ((ch)->player.sex ?                                    \
        (((ch)->player.sex == 1) ? "his" : "her") : "its")

#define HSSH(ch) ((ch)->player.sex ?                                    \
        (((ch)->player.sex == 1) ? "he" : "she") : "it")

#define HMHR(ch) ((ch)->player.sex ?                                    \
        (((ch)->player.sex == 1) ? "him" : "her") : "it")

#define ANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "An" : "A")

#define SANA(obj) (index("aeiouyAEIOUY", *(obj)->name) ? "an" : "a")

#define IS_NPC(ch)  ((ch)->specials.npc)

#define IS_MOB(ch)  (IS_NPC(ch) && ((ch)->nr >-1))

#define GET_POS(ch)     ((ch)->specials.position)

#define GET_COND(ch, i) ((ch)->specials.conditions[(i)])

#define GET_NAME(ch)    ((ch)->player.name)

#define GET_TITLE(ch)   ((ch)->player.title)

#define GET_CLASS(ch)   ((ch)->player.class)

#define GET_HOME(ch)    ((ch)->player.hometown)

#define GET_AGE(ch)     (age(ch).year)

#define GET_STR(ch)     ((ch)->tmpabilities.str)

#define GET_ADD(ch)     ((ch)->tmpabilities.str_add)

#define GET_DEX(ch)     ((ch)->tmpabilities.dex)

#define GET_INT(ch)     ((ch)->tmpabilities.intel)

#define GET_WIS(ch)     ((ch)->tmpabilities.wis)

#define GET_CON(ch)     ((ch)->tmpabilities.con)

#define GET_CHR(ch)     ((ch)->tmpabilities.chr)

#define GET_RSTR(ch)     ((ch)->abilities.str)

#define GET_RADD(ch)     ((ch)->abilities.str_add)

#define GET_RDEX(ch)     ((ch)->abilities.dex)

#define GET_RINT(ch)     ((ch)->abilities.intel)

#define GET_RWIS(ch)     ((ch)->abilities.wis)

#define GET_RCON(ch)     ((ch)->abilities.con)

#define GET_RCHR(ch)     ((ch)->abilities.chr)

#define STRENGTH_APPLY_INDEX(ch) \
        ( ((GET_ADD(ch)==0) || (GET_STR(ch) != 18)) ? GET_STR(ch) :\
          (GET_ADD(ch) <= 50) ? 26 :( \
          (GET_ADD(ch) <= 75) ? 27 :( \
          (GET_ADD(ch) <= 90) ? 28 :( \
          (GET_ADD(ch) <= 99) ? 29 :  30 ) ) )                   \
        )

#define GET_AC(ch)      ((ch)->points.armor)

#define GET_HIT(ch)     ((ch)->points.hit)

#define GET_MAX_HIT(ch) (hit_limit(ch))

#define GET_MOVE(ch)    ((ch)->points.move)

#define GET_MAX_MOVE(ch) (move_limit(ch))

#define GET_MANA(ch)    ((ch)->points.mana)

#define GET_MAX_MANA(ch) (mana_limit(ch))

#define GET_GOLD(ch)    ((ch)->points.gold)

#define GET_BANK(ch)    ((ch)->points.bankgold)

#define GET_ZONE(ch)    ((ch)->specials.permissions)

#define GET_EXP(ch)     ((ch)->points.exp)

#define GET_LEADERSHIP_EXP(ch)  ((ch)->points.leadership_exp)

#define GET_PRAC(ch)     ((ch)->specials.spells_to_learn)

#define GET_HEIGHT(ch)  ((ch)->player.height)

#define GET_WEIGHT(ch)  ((ch)->player.weight)

#define GET_SEX(ch)     ((ch)->player.sex)

#define GET_RACE(ch)     ((ch)->race)

#define GET_HITROLL(ch) ((ch)->points.hitroll)

#define GET_DAMROLL(ch) ((ch)->points.damroll)

#define GET_SPECFLAGS(ch) ((ch)->player.user_flags)

#define AWAKE(ch) (GET_POS(ch) > POSITION_SLEEPING && \
                   !(ch)->specials.paralyzed )

#ifndef LAG_MOBILES
#define WAIT_STATE(ch, cycle) \
{ \
    struct char_data *_ch = (struct char_data *)(ch); \
    int _cycle = (int)(cycle); \
    if(_ch->desc && _ch->desc->wait < _cycle) { \
	    _ch->desc->wait = (GetMaxLevel(_ch) >= DEMIGOD ? 0 : _cycle); \
    } \
} (void)(0)
#else
#define WAIT_STATE(ch, cycle) \
{ \
    struct char_data *_ch = (struct char_data *)(ch); \
    int _cycle = (int)(cycle); \
    if(_ch->desc) { \
        if(_ch->desc->wait < _cycle) { \
            _ch->desc->wait = (GetMaxLevel(_ch) >= DEMIGOD ? 0 : _cycle); \
        } \
    } else { \
        if(_ch->specials.tick_to_lag < _cycle) { \
            _ch->specials.tick_to_lag = _cycle; \
        } \
    } \
} (void)(0)
#endif

/*
 * Object And Carry related macros 
 */

#define GET_XCOORD(obj)  ((obj)->value[0])
#define GET_YCOORD(obj)  ((obj)->value[1])

#define CAN_WEAR(obj, part) (IS_SET((obj)->wear_flags,part))

#define GET_OBJ_WEIGHT(obj) ((obj)->weight)

#define CAN_CARRY_W(ch) (str_app[STRENGTH_APPLY_INDEX(ch)].carry_w)

#define CAN_CARRY_N(ch) (5+GET_DEX(ch)/2+GetMaxLevel(ch)/2)

#define IS_CARRYING_W(ch) ((ch)->specials.carry_weight)

#define IS_CARRYING_N(ch) ((ch)->specials.carry_items)

#define CAN_CARRY_OBJ(ch,obj)  \
   (((IS_CARRYING_W(ch) + GET_OBJ_WEIGHT(obj)) <= CAN_CARRY_W(ch)) &&   \
    ((IS_CARRYING_N(ch) + 1) <= CAN_CARRY_N(ch)))

#define CAN_GET_OBJ(ch, obj)   \
   (CAN_WEAR((obj), ITEM_TAKE) && CAN_CARRY_OBJ((ch),(obj)) &&          \
    objectIsVisible((ch),(obj)))

/*
 * char name/short_desc(for mobs) or someone? 
 */

#define PERS(ch, vict)   ( \
        CAN_SEE(vict, ch) ? \
          (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : \
          "someone")
#define PERS_LOC(ch, vict)   ( \
        CAN_SEE(vict, ch) ? \
          (!IS_NPC(ch) ? (ch)->player.name : (ch)->player.short_descr) : \
          "")

#define OBJS(obj, vict) (objectIsVisible((vict), (obj)) ? \
        (obj)->short_description  : "something")

#define OBJN(obj, vict) (objectIsVisible((vict), (obj)) ? \
        fname((obj)->name) : "something")

#define OUTSIDE(ch) (!IS_SET(roomFindNum((ch)->in_room)->room_flags,INDOORS))

#define IS_IMMORTAL(ch) (!IS_NPC(ch) && (GetMaxLevel(ch) >= IMMORTAL))

#define IS_POLICE(ch) ((ch->nr == 3060) || (ch->nr == 3069) || \
                       (ch->nr == 3067))

#define IS_CORPSE(obj) (ITEM_TYPE((obj))==ITEM_TYPE_CONTAINER && \
                        (obj)->value[3] && \
                        KeywordsMatch(&keyCorpse, &(obj)->keywords))

#define EXIT(ch, door)  (roomFindNum((ch)->in_room)->dir_option[door])

#define EXIT_NUM(room_num, door)  (roomFindNum(room_num)->dir_option[door])

#if 0
int             exit_ok(struct room_direction_data *, struct room_data **);
#endif

#define CAN_GO(ch, door) (EXIT(ch,door) && roomFindNum(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_CLOSED))

#define CAN_GO_HUMAN(ch, door) (EXIT(ch,door) && \
                          roomFindNum(EXIT(ch,door)->to_room) \
                          && !IS_SET(EXIT(ch, door)->exit_info, EX_LOCKED))

#define GET_ALIGNMENT(ch) ((ch)->specials.alignment)

#define IS_GOOD(ch)    (GET_ALIGNMENT(ch) >= 350)
#define IS_EVIL(ch)    (GET_ALIGNMENT(ch) <= -350)
#define IS_NEUTRAL(ch) (!IS_GOOD(ch) && !IS_EVIL(ch))

#define ITEM_TYPE(obj)  ((int)(obj)->type_flag)

#define IS_PC(ch) (!IS_NPC((ch)) || (ch)->specials.polyself)

#define GET_AVE_LEVEL(ch) (GetMaxLevel(ch)+(GetSecMaxLev(ch)/2)+\
          (GetThirdMaxLev(ch)/3))

#define GET_ALIAS(ch, num) ((ch)->specials.A_list->com[(num)])

#define MOUNTED(ch) ((ch)->specials.mounted_on)
#define RIDDEN(ch) ((ch)->specials.ridden_by)

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
