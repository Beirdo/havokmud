#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"
/* #include "mail.h" 	 moved to protos.h */

/*   external vars  */

extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
	extern int top_of_world;
	extern struct int_app_type int_app[26];
extern char *pc_class_types[];
extern struct title_type titles[4][ABS_MAX_LVL];
extern char *dirs[];
extern struct QuestItem QuestList[4][IMMORTAL];
extern int gSeason;  /* what season is it ? */

extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *spells[];

/* extern procedures */
void do_group(struct char_data *ch, char *arg, int cmd);
int choose_exit_global(int a, int b, int c);
int go_direction(struct char_data *ch, int dir);
void hit(struct char_data *ch, struct char_data *victim, int type);
void gain_exp(struct char_data *ch, int gain);

/* char *strdup(char *source); */

struct char_data *FindVictim( struct char_data *ch);
struct char_data *char_holding( struct obj_data *obj);
void send_to_all(char *messg);
void do_shout(struct char_data *ch, char *argument, int cmd);
int IsUndead( struct char_data *ch);
struct time_info_data age(struct char_data *ch);
int CountLims(struct obj_data *obj);
struct char_data *FindAnAttacker( struct char_data *ch);
void NailThisSucker( struct char_data *ch);
int NumCharmedFollowersInRoom(struct char_data *ch);
struct char_data *FindMobDiffZoneSameRace(struct char_data *ch);
struct char_data *FindMobInRoomWithFunction(int room, int (*func)());
void do_stand(struct char_data *ch, char *arg, int cmd);
void do_sit(struct char_data *ch, char *arg, int cmd);
void do_shout(struct char_data *ch, char *arg, int cmd);
void do_emote(struct char_data *ch, char *arg, int cmd);
void do_say(struct char_data *ch, char *arg, int cmd);
void add_follower(struct char_data *ch, struct char_data *leader);
void stop_follower(struct char_data *ch);

/* chess_game() stuff starts here */
/* Inspiration and original idea by Feith */
/* Implementation by Gecko */

#define WHITE 0
#define BLACK 1

int side = WHITE;  /* to avoid having to pass side with each function call */

#define IS_BLACK(piece) (((piece) >= 1400) && ((piece) <= 1415))
#define IS_WHITE(piece) (((piece) >= 1448) && ((piece) <= 1463))
#define IS_PIECE(piece) ((IS_WHITE(piece)) || (IS_BLACK(piece)))
#define IS_ENEMY(piece) (side?IS_WHITE(piece):IS_BLACK(piece))
#define IS_FRIEND(piece) (side?IS_BLACK(piece):IS_WHITE(piece))
#define ON_BOARD(room) (((room) >= 1400) && ((room) <= 1463))
#define FORWARD (side?2:0)
#define BACK    (side?0:2)
#define LEFT    (side?1:3)
#define RIGHT   (side?3:1)

#define EXIT_ROOM(roomp,dir) ((roomp)?((roomp)->dir_option[dir]):NULL)
#define CAN_GO_ROOM(roomp,dir) (EXIT_ROOM(roomp,dir) && \
                               real_roomp(EXIT_ROOM(roomp,dir)->to_room))

/* get pointer to room in the given direction */
#define ROOMP(roomp,dir) ((CAN_GO_ROOM(roomp,dir)) ? \
                          real_roomp(EXIT_ROOM(roomp,dir)->to_room) : NULL)

struct room_data *forward_square(struct room_data *room)
{
  return ROOMP(room, FORWARD);
}

struct room_data *back_square(struct room_data *room)
{
  return ROOMP(room, BACK);
}

struct room_data *left_square(struct room_data *room)
{
  return ROOMP(room, LEFT);
}

struct room_data *right_square(struct room_data *room)
{
  return ROOMP(room, RIGHT);
}

struct room_data *forward_left_square(struct room_data *room)
{
  return ROOMP(ROOMP(room, FORWARD), LEFT);
}

struct room_data *forward_right_square(struct room_data *room)
{
  return ROOMP(ROOMP(room, FORWARD), RIGHT);
}

struct room_data *back_right_square(struct room_data *room)
{
  return ROOMP(ROOMP(room, BACK), RIGHT);
}

struct room_data *back_left_square(struct room_data *room)
{
  return ROOMP(ROOMP(room, BACK), LEFT);
}

struct char_data *square_contains_enemy(struct room_data *square)
{
  struct char_data *i;

  for (i = square->people; i; i = i->next_in_room)
    if (IS_ENEMY(mob_index[i->nr].virtual))
      return i;

  return NULL;
}

int square_contains_friend(struct room_data *square)
{
  struct char_data *i;

  for (i = square->people; i; i = i->next_in_room)
    if (IS_FRIEND(mob_index[i->nr].virtual))
      return TRUE;

  return FALSE;
}

int square_empty(struct room_data *square)
{
  struct char_data *i;

  for (i = square->people; i; i = i->next_in_room)
    if (IS_PIECE(mob_index[i->nr].virtual))
      return FALSE;

  return TRUE;
}

int chess_game(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct room_data *rp = NULL, *crp = real_roomp(ch->in_room);
  struct char_data *ep = NULL;
  int move_dir = 0, move_amount = 0, move_found = FALSE;
  int c = 0;

  if (cmd || !AWAKE(ch))
    return FALSE;

  /* keep original fighter() spec_proc for kings and knights */
  if (ch->specials.fighting)
    switch (mob_index[ch->nr].virtual) {
      case 1401: case 1404: case 1406: case 1457: case 1460: case 1462:
        return fighter(ch, cmd, arg, mob, type);
      default:
        return FALSE;
    }

  if (!crp || !ON_BOARD(crp->number))
    return FALSE;

  if (side == WHITE && IS_BLACK(mob_index[ch->nr].virtual))
    return FALSE;

  if (side == BLACK && IS_WHITE(mob_index[ch->nr].virtual))
    return FALSE;

  if (number(0,15))
    return FALSE;

  switch (mob_index[ch->nr].virtual) {
    case 1408: case 1409: case 1410: case 1411:  /* black pawns */
    case 1412: case 1413: case 1414: case 1415:
    case 1448: case 1449: case 1450: case 1451:  /* white pawns */
    case 1452: case 1453: case 1454: case 1455:
      move_dir = number(0,3);
      switch (move_dir) {
        case 0: rp = forward_left_square(crp);  break;
        case 1: rp = forward_right_square(crp); break;
        case 2: rp = forward_square(crp);       break;
        case 3:
          if (real_roomp(ch->in_room) &&
              (real_roomp(ch->in_room)->number == mob_index[ch->nr].virtual)) {
            rp = forward_square(crp);
            if (rp && square_empty(rp) && ON_BOARD(rp->number)) {
              crp = rp;
              rp = forward_square(crp);
            }
          }
      }
      if (rp && (!square_contains_friend(rp)) && ON_BOARD(rp->number)) {
        ep = square_contains_enemy(rp);
        if (((move_dir <= 1) && ep) || ((move_dir > 1) && !ep))
          move_found = TRUE;
      }
      break;

    case 1400:  /* black rooks */
    case 1407:
    case 1456:  /* white rooks */
    case 1463:
      move_dir = number(0,3);
      move_amount = number(1,7);
      for (c = 0; c < move_amount; c++) {
        switch(move_dir) {
          case 0: rp = forward_square(crp);  break;
          case 1: rp = back_square(crp);     break;
          case 2: rp = right_square(crp);    break;
          case 3: rp = left_square(crp);
        }
        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
          move_found = TRUE;
          if ((ep = square_contains_enemy(rp)))
            c = move_amount;
          else
            crp = rp;
        }
        else {
          c = move_amount;
          rp = crp;
        }
      }
      break;

    case 1401:  /* black knights */
    case 1406:
    case 1457:  /* white knights */
    case 1462:
      move_dir = number(0,7);
      switch(move_dir) {
        case 0: rp = forward_left_square(forward_square(crp));  break;
        case 1: rp = forward_right_square(forward_square(crp)); break;
        case 2: rp = forward_right_square(right_square(crp));   break;
        case 3: rp = back_right_square(right_square(crp));  break;
        case 4: rp = back_right_square(back_square(crp));   break;
        case 5: rp = back_left_square(back_square(crp));    break;
        case 6: rp = back_left_square(left_square(crp));    break;
        case 7: rp = forward_left_square(left_square(crp));
      }
      if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
        move_found = TRUE;
        ep = square_contains_enemy(rp);
      }
      break;

    case 1402:  /* black bishops */
    case 1405:
    case 1458:  /* white bishops */
    case 1461:
      move_dir = number(0,3);
      move_amount = number(1,7);
      for (c = 0; c < move_amount; c++) {
        switch(move_dir) {
          case 0: rp = forward_left_square(crp);  break;
          case 1: rp = forward_right_square(crp); break;
          case 2: rp = back_right_square(crp);    break;
          case 3: rp = back_left_square(crp);
        }
        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
          move_found = TRUE;
          if ((ep = square_contains_enemy(rp)))
            c = move_amount;
          else
            crp = rp;
        }
        else {
          c = move_amount;
          rp = crp;
        }
      }
      break;

    case 1403:  /* black queen */
    case 1459:  /* white queen */
      move_dir = number(0,7);
      move_amount = number(1,7);
      for (c = 0; c < move_amount; c++) {
        switch(move_dir) {
          case 0: rp = forward_left_square(crp);  break;
          case 1: rp = forward_square(crp);       break;
          case 2: rp = forward_right_square(crp); break;
          case 3: rp = right_square(crp);         break;
          case 4: rp = back_right_square(crp);    break;
          case 5: rp = back_square(crp);          break;
          case 6: rp = back_left_square(crp);     break;
          case 7: rp = left_square(crp);
        }
        if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
          move_found = TRUE;
          if ((ep = square_contains_enemy(rp)))
            c = move_amount;
          else
            crp = rp;
        }
        else {
          c = move_amount;
          rp = crp;
        }
      }
      break;

    case 1404:  /* black king */
    case 1460:  /* white king */
      move_dir = number(0,7);
      switch (move_dir) {
        case 0: rp = forward_left_square(crp);  break;
        case 1: rp = forward_square(crp);       break;
        case 2: rp = forward_right_square(crp); break;
        case 3: rp = right_square(crp);         break;
        case 4: rp = back_right_square(crp);    break;
        case 5: rp = back_square(crp);          break;
        case 6: rp = back_left_square(crp);     break;
        case 7: rp = left_square(crp);
      }
      if (rp && !square_contains_friend(rp) && ON_BOARD(rp->number)) {
        move_found = TRUE;
        ep = square_contains_enemy(rp);
      }
      break;
  }

  if (move_found && rp) {
    do_emote(ch, "leaves the room.", 0);
    char_from_room(ch);
    char_to_room(ch, rp->number);
    do_emote(ch, "has arrived.", 0);

    if (ep) {
      if (side)
        switch(number(0,3)) {
          case 0:
            do_emote(ch, "grins evilly and says, 'ONLY EVIL shall rule!'", 0);
            break;
          case 1:
            do_emote(ch, "leers cruelly and says, 'You will die now!'", 0);
            break;
          case 2:
            do_emote(ch, "issues a bloodcurdling scream.", 0);
            break;
          case 3:
            do_emote(ch, "glares with black anger.", 0);
        }
      else
        switch(number(0,3)) {
          case 0:
            do_emote(ch, "glows an even brighter pristine white.", 0);
            break;
          case 1:
            do_emote(ch, "chants a prayer and begins battle.", 0);
            break;
          case 2:
            do_emote(ch, "says, 'Black shall lose!", 0);
            break;
          case 3:
            do_emote(ch, "shouts, 'For the Flame! The Flame!'", 0);
        }
      hit(ch, ep, TYPE_UNDEFINED);
    }
    side = (side + 1) % 2;
    return TRUE;
  }
  return FALSE;
}

int AcidBlob(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct obj_data *i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content) {
    if (IS_SET(i->obj_flags.wear_flags, ITEM_TAKE) && !strncmp(i->name, "corpse", 6)) {
      act("$n destroys some trash.", FALSE, ch, 0, 0, TO_ROOM);

      obj_from_room(i);
      extract_obj(i);
      return(TRUE);
    }
  }
  return(FALSE);
}

int death_knight(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

  if (number(0,1)) {
    return(fighter(mob, cmd, arg, mob, type));
  } else {
    return(magic_user(mob, cmd, arg, mob, type));
  }
}

int acid_monster(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 if (cmd)
    return(FALSE);

    if (ch->specials.fighting && number(0,2)) {
     act("$n spits a stream of acid at $N!",FALSE,ch,0,ch->specials.fighting,TO_NOTVICT);
     act("$n spits a stream of acid at you!",FALSE,ch,0,ch->specials.fighting,TO_VICT);
     spell_acid_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
     return(TRUE);
    }

    return(FALSE);
}
int avatar_celestian(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

  if (number(0,1))   {
    return(fighter(mob, cmd, arg, mob, type));
  }   else   {
    return(magic_user(mob, cmd, arg, mob, type));
  }

}


int lizardman_shaman(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

 if (cmd) return(FALSE);
 if (!AWAKE(mob)) return(FALSE);

 if (ch->specials.fighting)
 {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
          (GET_POS(ch) > POSITION_STUNNED))
        StandUp(ch);

 switch(number(1,4))
  {
   case 1:return(cleric(mob,cmd,arg,mob,type));
   	  break;
   case 2:return(fighter(mob,cmd,arg,mob,type));
   	  break;
   case 3:do_emote(ch, "leers cruelly and says, 'You will die now!'", 0);
   	  break;
   case 4:do_emote(ch, "hisses in anger, 'Leave our home now or die!'", 0);
   	  break;
  } /* end switch */

 } /* fighting */
  else
   {
   if (number(1,4)==4)
    {
     switch(number(1,3))
      {
       case 1: do_emote(ch,"waves his hands about uttering some slurred words.",0);
       	       break;
       case 2: do_emote(ch,"looks upwards and calls to some unseen god.",0);
               break;
       case 3: do_emote(ch,"grabs his chest and screams ,'Death to all that oppose you!'",0);
       	       break;
      } /* end switch */
    }   /* random emote */
   } /* else not fighting */
}


int baby_bear(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *t;
  struct room_data *rp;

  if(cmd)
    return(FALSE);

  if(ch->specials.fighting)  {
    command_interpreter(ch,"scream");
    rp = real_roomp(ch->in_room);
    if (!rp) return(FALSE);

    for(t=rp->people; t ; t=t->next_in_room) {
      if(GET_POS(t) == POSITION_SLEEPING)
        if(number(0,1))
          do_wake(t,"",0);
    }
  }
  return(FALSE);
}

#define TimnusRoom 22602
#define TIMNUSNORTHLIMIT 30
#define TIMNUSWESTLIMIT 9

int timnus(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  /* north = 1 */
  /* west  = 4 */

  struct char_data *vict;
  byte lspell;

  if(cmd) {
    if(cmd == 1 && ch->in_room == TimnusRoom) {
      if ((TIMNUSNORTHLIMIT < GetMaxLevel(ch)) &&
	  (GetMaxLevel(ch) < LOW_IMMORTAL))       {
	if (!check_soundproof(ch)) {
	  act("$n tells you 'Thou art not pure enough of heart.'", TRUE,
	      mob, 0, ch, TO_VICT);
	}
	act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
	return(TRUE);
      }
      return(FALSE);
    } else if(cmd == 4 && ch->in_room == TimnusRoom) {
      if ((TIMNUSWESTLIMIT < GetMaxLevel(ch)) &&
	  (GetMaxLevel(ch) < LOW_IMMORTAL))       {
	if (!check_soundproof(ch)) {
	  act("$n tells you 'Thou art not pure enough of heart.'", TRUE,
	      mob, 0, ch, TO_VICT);
	}
	act("$n grins evilly.", TRUE, mob, 0, ch, TO_VICT);
	return(TRUE);
      }
    }
    return(FALSE);
  } else {			/* end of if(cmd) */
    if(!IS_PC(ch)) {
      if (ch->specials.fighting) {
	if ((GET_POS(ch) < POSITION_FIGHTING) &&
	    (GET_POS(ch) > POSITION_STUNNED)){
	  StandUp(ch);
	} else {
	  if(check_soundproof(ch) || check_nomagic(ch,0,0))
	    MakeNiftyAttack(ch);
	  else {
	    vict = FindVictim(ch);
	    if(!vict)
	      vict = ch->specials.fighting;
	    if(!vict)
	      return(FALSE);
	    lspell = number(0,GetMaxLevel(ch));
	    if (!IS_PC(ch)) {
	      lspell+= GetMaxLevel(ch)/5;
	    }
	    lspell = MIN(GetMaxLevel(ch), lspell);

	    if(lspell < 1)
	      lspell=1;

	    if(IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
	      act("$n utters the words 'Let me see the light!'",
		  TRUE, ch, 0, 0, TO_ROOM);
	      cast_cure_blind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(GET_MOVE(ch) < 0) {
	      act("$n pulls a glass of lemonade out of thin air.  How refreshing.",
		  1,ch,0,0,TO_ROOM);
	      cast_refresh(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(!(IS_AFFECTED(ch,AFF_FIRESHIELD)) && (lspell > 39)) {
	      act("$n utters the words 'FLAME ON!!!'",1,ch,0,0,TO_ROOM);
	      cast_fireshield(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(!(IS_AFFECTED(ch,AFF_SANCTUARY)) && (lspell > 25)) {
              act("$n utters the words 'Don't you just hate it when I do this?'",1,ch,0,0,TO_ROOM);
              cast_sanctuary(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
              return(FALSE);
            }

            if(((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 25)) && (GetMaxLevel(ch) >= GetMaxLevel(vict))) {
              act("$n utters the words 'Do unto others as you'd have them do unto you...'",
	      1, ch, 0, 0, TO_ROOM);
      cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
              return(FALSE);
            }

            if(((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 25)) && (GetMaxLevel(ch) >= GetMaxLevel(vict))) {
              act("$n utters the words 'Do unto others as you'd have them do unto you...'",
	      1, ch, 0, 0, TO_ROOM);
              cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
              return(FALSE);
            }

            if(lspell > 40) {
               act("$n utters the words 'Here's a penny, go buy a brain, and give me the change'",
               1, ch, 0, 0, TO_ROOM);
               cast_feeblemind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
               return(FALSE);
            }
	    /* well, spells failed or not needed, let's kick someone :) */

	    FighterMove(ch);
	    return(FALSE);
	  }
	}
      }
    }
  }
}


int winger(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;

  if(cmd)
    return(FALSE);

  if (check_soundproof(ch)) return(FALSE);

  if (check_nomagic(ch, 0, 0))  return(FALSE);

  if (IS_AFFECTED(ch, AFF_PARALYSIS)) return(FALSE);

  if (ch->specials.fighting) {

    vict = FindVictim(ch);

    if(!vict)
      vict = ch->specials.fighting;

    if(!vict)
      return(FALSE);

    if(IS_AFFECTED(vict,AFF_FLYING)) {
      act("$n utters the words 'Instant Wing Remover!'.",1, ch, 0, 0, TO_ROOM);
      cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(TRUE);
    } else {
      /* As per request of Alfred, if we are fighting and they aren't  */
      /* affected by fly, then eek, we flee! */
      do_flee(ch,"",0);
      return(TRUE);
    }
  }
  return(FALSE);
}

int village_princess(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

  if (cmd) return(FALSE);
  if (!AWAKE(ch)) return(FALSE);

	/* Add more princess type things, holler for help etc... */
 if (ch->specials.fighting)
 {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
          (GET_POS(ch) > POSITION_STUNNED))
        StandUp(ch);
  if (number(0,1))
  {
    return(fighter(mob, cmd, arg, mob, type));
  }
   else
  {
    return(magic_user(mob, cmd, arg, mob, type));
  }
 } /* not fighting... */
}

#define STRAHD_ZOMBIE 30000
int strahd_zombie(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 struct char_data *mobtmp;
 char buf[128];

  if(cmd)
    return(FALSE);

 if (ch->specials.fighting)
 {
    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED))
         StandUp(ch);

    switch (number(0,4)) {
	case 1: return(shadow(ch,cmd,arg,mob,type));
	case 2: { if (number(0,1)) {
	          mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
	          char_to_room(mobtmp, ch->in_room);
	          act("A body part falls from $n and forms into $N!", FALSE, ch, 0, mobtmp, TO_ROOM);
	    /* avoid same name fighting */
	          if (GET_NAME(ch)!=GET_NAME(ch->specials.fighting)) {
	           sprintf(buf,"kill %s",GET_NAME(ch->specials.fighting));
	           command_interpreter(mobtmp,buf);
	     	   }
	     	  }
	          return(TRUE);
		 }
		break;
        case 3 : return(wraith(ch,cmd,arg,mob,type));
        	break;
	default: return(shadow(ch,cmd,arg,mob,type));
		break;
    } /* end switch */

   } /* end was fighting */

   return(FALSE);
} /* end zombie */


#define SUN_SWORD_RAVENLOFT  30750
#define MAX_SUN_SWORDS	     3		/* to limit the number of sun swords */
#define HOLY_ITEM_RAVENLOFT  30751
#define STRAHD_VAMPIRE       30113	/* mob number of strahd */
#define STRAHD_RELOCATE_ROOM 30008      /* room to put second strahd */
#define STRAHD2_ITEM1        31798
#define STRAHD2_ITEM2        31799

int strahd_vampire(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *mobtmp;
  static struct char_data *tmp;
  struct obj_data  *target_obj ;
  char buf[80];
  int i,r_num,virtual,hasitem=FALSE;
  static int location=-1;
  static int vampnum=0;

int sun_loc[] =
    {    30041,    30085,    30088,    30025    };
int holy_loc[] =
    {     30037,     30015,     30086     };

	/* first time he dies load another strahd at second location */
	/* and force it to hate the person that he was fighting      */

if (type == EVENT_DEATH)    {
	switch(vampnum) {
         case -1:vampnum=0;
         	 break;
	  case 0: {
	   vampnum++;
    	   mobtmp = read_mobile(real_mobile(STRAHD_VAMPIRE),REAL);
    	   char_to_room(mobtmp, STRAHD_RELOCATE_ROOM);
    	   AddHated(mobtmp,tmp);
	   SetHunting(mobtmp,tmp);
 	 /*new items for strahd number 2*/
 	 if ((r_num = real_object(STRAHD2_ITEM1)) >= 0) {
	     target_obj = read_object(r_num, REAL);
	     obj_to_char(target_obj, mobtmp);
   	 }
 	 if ((r_num = real_object(STRAHD2_ITEM2)) >= 0) {
	   target_obj = read_object(r_num, REAL);
	   obj_to_char(target_obj, mobtmp);
   	 }
 	   act("$n falls to the ground and crumbles into dust, a faint green shadow leaves the corpse.", FALSE, ch, 0, 0, TO_ROOM);
#if 1
    act("$n summons an army of his faithful followers!", FALSE, mobtmp, 0, 0, TO_ROOM);
	ch=mobtmp;
	for (i=0;i<7;i++) {	/* summon 7 zombies */
    	    mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
	    char_to_room(mobtmp, ch->in_room);
	    add_follower(mobtmp, ch);
	    SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
	    AddHated(mobtmp,tmp);
	  }
           do_order(ch, "followers guard on", 0);
#endif
	   return(TRUE);
  	  } break;

	  default:{
   act("$n falls to the ground and crumbles into dust, a faint sigh of relief seems to fill the castle.",FALSE, ch, 0, 0, TO_ROOM);
        vampnum=-1;
    return(TRUE);
	          } break;
	} /* end switch */
    } else {	 /* end event DEATH */
      if (vampnum == -1)  /* fix this if the vamp is loaded and we are not dead */
              vampnum=0;
   }


  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

 if (ch->specials.fighting)
 {
   tmp=ch->specials.fighting;

	switch(number(0,5)) {
	 case 1: return(magic_user(mob,cmd,arg,mob,type));
	        break;
 	 case 2:return(magic_user(mob,cmd,arg,mob,type));
	 	break;
	 case 3: {
 /* drain one level, check for sun or holy item here before doing! */

  if (tmp->equipment[WIELD]) {
virtual = (tmp->equipment[WIELD]->item_number >= 0) ? obj_index[tmp->equipment[WIELD]->item_number].virtual : 0;
 if (virtual == SUN_SWORD_RAVENLOFT)
    hasitem = TRUE;
   } /* end wielding */

 if (tmp->equipment[HOLD]) {
    virtual = (tmp->equipment[HOLD]->item_number >= 0) ? obj_index[tmp->equipment[HOLD]->item_number].virtual : 0;
 if (virtual == HOLY_ITEM_RAVENLOFT)
 	hasitem = TRUE;
} /* end holding */

 if (hasitem) {
 /*  act("$n tries to bite you, but is forced back by an item you have!", FALSE, ch, 0, tmp, TO_CHAR);*/
     act("$n tries to bite $N, but is repelled by some powerful force!", FALSE, ch, 0, tmp, TO_ROOM);
     return(TRUE);
   }
              /* wel'p they screwed up and didn't have either the sun sword */
              /* or the holy symbol, lets drain'em one level! */

  if (GetMaxLevel(tmp) <=1) {
  	 log("Strahd tried to drain char below level 1");
     } else {
/* fix this later... 						   */
/*  act("$N bites you hard, OUCH!!!", FALSE, tmp, 0, ch, TO_CHAR); */

	  act("$n bites $N who looks drained and weak!", FALSE, ch, 0, tmp, TO_ROOM);
	  drop_level(tmp,BestClassBIT(tmp),FALSE);
          set_title(tmp);
          do_save(tmp,"",0);
    } /* else GetMaxLevel */
 } break;

		/* summon up Strahd Zombie and order it to protect and kill */
 default:{    	 tmp =ch->specials.fighting;
	    	 mobtmp = read_mobile(real_mobile(STRAHD_ZOMBIE),REAL);
	    	 char_to_room(mobtmp, ch->in_room);
	    	 add_follower(mobtmp, ch);
	    	 SET_BIT(mobtmp->specials.affected_by, AFF_CHARM);
	    	 act("$n summons one of his faithful followers, $N!", FALSE, ch, 0, mobtmp, TO_ROOM);
            	 do_order(ch, "followers guard on", 0);
            	 sprintf(buf,"followers kill %s",GET_NAME(tmp));
            	 do_order(ch,buf,0);
	    	 return(TRUE);
	  } break;

	} /* end switch */

    } /* end isfighting? */

  if (!ch->specials.fighting)  {
     /* is the holy symbol in game? */
      if (location==-1)       {
   	if ( (target_obj=get_obj_vis_world(ch, "baabaabowie", NULL)) &&
       	      target_obj->in_room != NOWHERE )
         {
	 /* items already loaded! */
          location = target_obj->in_room;
	  return;
         }
      else
      if (vampnum !=0)
         {
         	/* do not load the items since this is the second vamp */
         } else
       		/* item was not found, lets load up the items and randomise the */
      	    	/* locations!                                                   */
      {

	log("Loading Ravenloft random location items.");
	/* place sun sword ... */
/*        if (obj_index[SUN_SWORD_RAVENLOFT].number < MAX_SUN_SWORDS) */

  	  if ((r_num = real_object(SUN_SWORD_RAVENLOFT)) >= 0)
  	  {
    	    target_obj = read_object(r_num, REAL);
            obj_to_room(target_obj,sun_loc[number(0,3)]);
   	  }

   	  /* place holy symbol */
  	  if ((r_num = real_object(HOLY_ITEM_RAVENLOFT)) >= 0)
  	  {
    	    target_obj = read_object(r_num, REAL);
    	    obj_to_room(target_obj,holy_loc[number(0,2)]);
   	  }

	  return(FALSE);
      } /* end loading items random rooms */


   } /* location != -1 */


   } /* end not fighting */

  return(magic_user(mob,cmd,arg,mob,type));
} /* end strahd */

int banshee(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

  struct char_data *tmp;
  int victim;

  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

  if (ch->specials.fighting &&
    (ch->specials.fighting->in_room == ch->in_room))
  {
    if (IS_DARK(ch->in_room))  {
      for (tmp = real_roomp(ch->in_room)->people; tmp; tmp = tmp->next_in_room)
      {
        if (tmp == ch || IS_IMMORTAL(tmp)) {
        continue;
      } else {
        victim=1;
        act("$n wails loudly draining the life from your body!", TRUE, ch, 0, 0, TO_ROOM);
        /* do wail damage stuff here */
      }
    } /* end for */
  } else /* not dark */
    if (number(0,1))  {
      return(magic_user(mob,cmd,arg,mob,type));
    } else  {
      tmp=ch->specials.fighting;
      if (GetMaxLevel(tmp) <=1)
      {
        log("Banshee tried to drain char below level 1");
      } else /* if GetmaxLevel */
      {
        act("$n bites $N who looks drained!", FALSE, ch, 0, tmp, TO_ROOM);
        act("$n bites you hard!", FALSE, ch, 0, tmp, TO_CHAR);
        drop_level(tmp,BestClassBIT(tmp),FALSE);
      } /* else GetMaxLevel */
        set_title(tmp);
        do_save(tmp,"",0);
    }

  } /* end she is fighting */

  return(FALSE);
} /* end banshee */



int mad_gertruda(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
          return(fighter(mob, cmd, arg, mob, type));
      }
  return(FALSE);
}

int mad_cyrus(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  if (cmd) return(FALSE);
  if (!AWAKE(mob)) return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
       return(fighter(mob, cmd, arg, mob, type));
      }
  return(FALSE);
}

#define IRON_DIR_STOP  EAST
#define IRON_ROOM_STOP 30078
int raven_iron_golem(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *v;

  if (!AWAKE(ch))
     return(FALSE);

    if (cmd == IRON_DIR_STOP && ch->in_room == IRON_ROOM_STOP) {
        return(CheckForBlockedMove(ch,cmd,arg,IRON_ROOM_STOP,IRON_DIR_STOP,0));
      }
if (cmd)
    return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {
      if (number(0,1)) {

    act("$n belows out a gout of green gas!",TRUE,ch,0,0,TO_ROOM);
    for (v = real_roomp(ch->in_room)->people;v;v = v->next_in_room) {
        if (ch != v && !IS_IMMORTAL(v)) {
        act("The green gas fills your lungs!.",FALSE,v,0,0,TO_CHAR);
	cast_sleep(GetMaxLevel(ch),v,"",SPELL_TYPE_SPELL, v, 0);
       }
      }	    /* end for */
	return(TRUE);
      } else {
        return(fighter(mob, cmd, arg, mob, type));
       }
      }

  return(FALSE);
}

#define RESCUE_VIRGIN  1950
#define RESCUE_ROOM    1983
#define SNAKE_GOD      1953
int snake_avt(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

 byte lspell=0;
 struct char_data *vict;

#if 0
 static int done=-1;
			/* so we only load the virgin once a boot */
				/* case the virgin is killed and  */
				/* a second snake avt is loaded   */

				/* does not work!! */
if (type == EVENT_BIRTH && ch->in_room == RESCUE_ROOM) {
   done++;
   return(FALSE);
  }

#endif

if (type == EVENT_DEATH && ch->in_room == RESCUE_ROOM) {
	   mob = get_char_vis(ch,"zifnab");
	   cast_portal(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,mob,0);
	   mob = read_mobile(real_mobile(RESCUE_VIRGIN),REAL);
    	   char_to_room(mob, RESCUE_ROOM);
  act("After defeating the avatar of the snake god a mystical door appears.",
      FALSE,ch,0,0,TO_ROOM);
  act("You quickly release the prisoner strapped to the altar and decide it",
      FALSE,ch,0,0,TO_ROOM);
  act("would be best to quickly enter the portal and leave this horrid place.",
      FALSE,ch,0,0,TO_ROOM);
  act("The prisoner says 'Yes, enter the portal to safety, but first unlock",
      FALSE,ch,0,0,TO_ROOM);
  act("the altar and take the spoils due your party.'",
      FALSE,ch,0,0,TO_ROOM);
  act("You listen to her and decide to QUICKLY do so before the portal disappears.",
      FALSE,ch,0,0,TO_ROOM);
  act("\n", FALSE,ch,0,0,TO_ROOM);
      return(FALSE);
	 }

  if (!AWAKE(ch))
     return(FALSE);

if (cmd)
    return(FALSE);

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {

    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED))
         StandUp(ch);


	    vict = FindVictim(ch);
	    if(!vict)
	      vict = ch->specials.fighting;
	    if(!vict)
	      return(FALSE);
	    lspell = number(0,GetMaxLevel(ch));
	    if (!IS_PC(ch)) {
	      lspell+= GetMaxLevel(ch)/5;
	    }
	    lspell = MIN(GetMaxLevel(ch), lspell);

	    if(lspell < 1)
	      lspell=1;

	    if(IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
	      act("$n utters the words 'Let me see the light!'",
		  TRUE, ch, 0, 0, TO_ROOM);
	      cast_cure_blind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(GET_MOVE(ch) < 0) {
	      act("$n pulls a vial of human blood out of thin air.  How refreshing.",
		  1,ch,0,0,TO_ROOM);
	      cast_refresh(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(!(IS_AFFECTED(ch,AFF_FIRESHIELD)) && (lspell > 39)) {
	      act("$n utters the words 'zimja'",1,ch,0,0,TO_ROOM);
	      cast_fireshield(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
	      return(FALSE);
	    }

	    if(!(IS_AFFECTED(ch,AFF_SANCTUARY)) && (lspell > 25)) {
              act("$n utters the words 'zija'",1,ch,0,0,TO_ROOM);
              cast_sanctuary(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,ch,0);
              return(FALSE);
            }

            if(((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 25)) && (GetMaxLevel(ch) >= GetMaxLevel(vict))) {
              act("$n utters the words 'paf zija'",
	      1, ch, 0, 0, TO_ROOM);
              cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
              return(FALSE);
            }

            if(((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 25)) && (GetMaxLevel(ch) >= GetMaxLevel(vict))) {
              act("$n utters the words 'paf zimja'",
	      1, ch, 0, 0, TO_ROOM);
              cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
              return(FALSE);
            }

            if(lspell > 40) {
               act("$n utters the words 'zerumba'",
               1, ch, 0, 0, TO_ROOM);
               cast_feeblemind(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
               return(FALSE);
            }
	    /* well, spells failed or not needed, let's kick someone :) */

      switch(number(0,3)) {
        case 1: return(magic_user(mob,cmd,arg,mob,type));
		break;
	case 2: return(cleric(mob,cmd,arg,mob,type));
			break;
        case 3: return(snake(mob,cmd,arg,mob,type));
        		break;
        default: return(fighter(mob,cmd,arg,mob,type));
        	break;

        } /* end switch */


   }  /* end fighting */

 return(magic_user(mob,cmd,arg,mob,type));
}

int virgin_sac(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

#if 1

 if (type == EVENT_DEATH && ch->in_room == RESCUE_ROOM) {
	   mob = read_mobile(real_mobile(SNAKE_GOD),REAL);
    	   char_to_room(mob, RESCUE_ROOM);
  act("$n says 'You have completed the sacrific, you shall be rewarded with death!'",
      FALSE,mob,0,0,TO_ROOM);
    return(TRUE);
  }

#endif

 if (!AWAKE(ch))
     return(FALSE);

if (cmd)
    return(FALSE);

  if (ch->specials.fighting) {
    if ((GET_POS(ch) < POSITION_FIGHTING) && (GET_POS(ch) > POSITION_STUNNED))
         StandUp(ch);
    return(TRUE);
  }

 return(FALSE);
}

int snake_guardians(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *snakemob;

  if (!AWAKE(ch))
    return(FALSE);


  if (!cmd) {
    if (ch->specials.fighting) {
      snake(ch, cmd, arg, mob, type);
    }
  } else {
   snakemob = FindMobInRoomWithFunction(ch->in_room, snake_guardians);
   if (cmd >= 1 && cmd <= 6) {
    if (cmd == 3) return(FALSE);  /* can always go south */
    		/* everything else gets ya attacked */
    if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF) && !IS_IMMORTAL(ch)) {
	act("$N blocks your path and attacks!",FALSE,ch,0,snakemob, TO_CHAR);
        act("$N hisses at $n and attacks!",TRUE, ch, 0, snakemob, TO_ROOM);
        hit(snakemob,ch,0);
      return(TRUE);
    }
   }
  }
  return(FALSE);
}





#if EGO

int EvilBlade(struct char_data *ch, int cmd, char *arg,struct obj_data *tobj, int type)
{
   extern struct str_app_type str_app[];
   struct obj_data *obj, *blade;
   struct char_data *joe, *holder;
   struct char_data *lowjoe = 0;
   char arg1[128], buf[250];

   if (type != PULSE_COMMAND)
     return(FALSE);

   if (IS_IMMORTAL(ch)) return(FALSE);
   if (!real_roomp(ch->in_room)) return(FALSE);

   for (obj = real_roomp(ch->in_room)->contents;
	obj ; obj = obj->next_content) {
     if (obj_index[obj->item_number].func == EvilBlade) {
       /* I am on the floor */
       for (joe = real_roomp(ch->in_room)->people; joe ;
	    joe = joe->next_in_room) {
	 if ((GET_ALIGNMENT(joe) <= -400) && (!IS_IMMORTAL(joe))) {
	   if (lowjoe) {
	     if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)){
	       lowjoe = joe;
	     }
	   } else lowjoe = joe;
	 }
       }
       if (lowjoe) {
	 if (CAN_GET_OBJ(lowjoe, obj)) {
	   obj_from_room(obj);
	   obj_to_char(obj,lowjoe);

	   sprintf(buf,"%s leaps into your hands!\n\r",obj->short_description);
	   send_to_char(buf,lowjoe);

	   sprintf(buf,"%s jumps from the floor and leaps into %s's hands!\n\r",
	    obj->short_description,GET_NAME(lowjoe));
	   act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);

	   if (!EgoBladeSave(lowjoe)) {
	     if (!lowjoe->equipment[WIELD]) {
	   sprintf(buf,"%s forces you to wield it!\n\r",obj->short_description);
           send_to_char(buf,lowjoe);
	       wear(lowjoe, obj, 12);
	       return(FALSE);
	     } else {
	       sprintf(buf,"You can feel %s attept to make you wield it.\n\r",
                 obj->short_description);
	       send_to_char(buf,lowjoe);
	       return(FALSE);
	     }
	   }
	 }
       }
     }
   }
   for (holder = real_roomp(ch->in_room)->people; holder ;
	holder = holder->next_in_room) {
     for (obj = holder->carrying; obj ; obj = obj->next_content) {
       if ((obj_index[obj->item_number].func) &&
	   (obj_index[obj->item_number].func != board)){
	 /*held*/
	 if (holder->equipment[WIELD]) {
	   if ((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
	sprintf(buf,"%s gets pissed off that you are wielding another weapon!\n\r",
        obj->short_description);
	send_to_char(buf,holder);
	sprintf(buf,"%s knocks %s out of your hands!!\n\r",obj->short_description,
	holder->equipment[WIELD]->short_description);
	send_to_char(buf,holder);
	     blade = unequip_char(holder,WIELD);
	     if (blade) obj_to_room(blade,holder->in_room);
	     if (!holder->equipment[WIELD]) {
	     sprintf(buf,"%s forces you yto wield it!\n\r",
	       obj->short_description);
	send_to_char(buf,holder);
	       wear(holder, obj, 12);
	       return(FALSE);
	     }
	   }
	 }
	 if (!EgoBladeSave(holder)) {
	   if (!EgoBladeSave(holder)) {
	     if (!holder->equipment[WIELD]) {
	     sprintf(buf,"%s forces you yto wield it!\n\r",
	       obj->short_description);
	send_to_char(buf,holder);
	       wear(holder, obj, 12);
	       return(FALSE);
	     }
	   }
	 }
	 if (affected_by_spell(holder,SPELL_CHARM_PERSON)) {
	   affect_from_char(holder,SPELL_CHARM_PERSON);
	  sprintf(buf,"Due to the %s, you feel less enthused about your master.\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }
       }
     }
     if (holder->equipment[WIELD]) {
       if ((obj_index[holder->equipment[WIELD]->item_number].func)
	   && (obj_index[holder->equipment[WIELD]->item_number].func != board)){
	 /*YES! I am being held!*/
	 obj = holder->equipment[WIELD];
	 if (affected_by_spell(holder,SPELL_CHARM_PERSON)) {
	   affect_from_char(holder,SPELL_CHARM_PERSON);
	  sprintf(buf,"Due to the %s, you feel less enthused about your master.\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }
	 if (holder->specials.fighting) {
	sprintf(buf,"%s almost sings in your hand!\n\r",
	  obj->short_description);
	  send_to_char(buf,holder);
sprintf(buf,"You can hear $n's %s almost sing with joy!",obj->short_description);
	   act(buf,FALSE, holder, 0, 0, TO_ROOM);
	   if ((holder == ch) && (cmd == 151)) {
	     if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
	      sprintf(buf,"You can feel %s attempt to stay in the fight!\n\r",
	      		obj->short_description);
	      		send_to_char(buf,ch);
	       return(FALSE);
	     } else {
	     sprintf(buf,"%s laughs at your attempt to flee from a fight!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);
	       GET_HIT(ch) -= 25;
	       if (GET_HIT(ch) < 0) {
		 GET_HIT(ch) = 0;
		 GET_POS(ch) = POSITION_STUNNED;
	       }
	       return(TRUE);
	     }
	   }
	 }
	 if ((cmd == 66) && (holder == ch)) {
	   one_argument(arg, arg1);
	   if (strcmp(arg1,"all") == 0) {
	     if (!EgoBladeSave(ch)) {

	     sprintf(buf,"%s laughs at your attempt remove it!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);

	       GET_HIT(ch) -= 25;
	       if (GET_HIT(ch) < 0) {
		 GET_HIT(ch) = 0;
		 GET_POS(ch) = POSITION_STUNNED;
	       }
	       return(TRUE);
	     } else {
	     sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",
	     	obj->short_description);
	     	send_to_char(buf,ch);
	       return(FALSE);
	     }
	   } else {
	     if (isname(arg1,obj->name)) {
	       if (!EgoBladeSave(ch)) {

	     sprintf(buf,"%s laughs at your attempt to remove it!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);

		 GET_HIT(ch) -= 25;
		 if (GET_HIT(ch) < 0) {
		   GET_HIT(ch) = 0;
		   GET_POS(ch) = POSITION_STUNNED;
		 }
		 return(TRUE);
	       } else {
	       sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",obj->short_description);
	       send_to_char(buf,ch);
		 return(FALSE);
	       }
	     }
	   }
	 }
	 for (joe = real_roomp(holder->in_room)->people; joe ;
	      joe = joe->next_in_room) {
	   if ((GET_ALIGNMENT(joe) >= 500) &&
	       (IS_MOB(joe)) && (CAN_SEE(holder,joe)) && (holder != joe)) {
	     if (lowjoe) {
	       if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)){
		 lowjoe = joe;
	       }
	     } else lowjoe = joe;
	   }
	 }
	 if (lowjoe) {
	   if (!EgoBladeSave(holder)) {
	     if (GET_POS(holder) != POSITION_STANDING) {
	     sprintf(buf,"%s yanks you yo your feet!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	       GET_POS(holder) = POSITION_STANDING;
	     }
	     sprintf(buf,"%s leaps out of control!!\n\r",
	       obj->short_description);
	       send_to_char(buf,holder);
	sprintf(buf,"%s jumps for $n's neck!",obj->short_description);
	     act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);
	     do_hit(holder,lowjoe->player.name, 0);
	     return(TRUE);
	   } else {
	     return(FALSE);
	   }
	 }
	 if ((cmd == 70) && (holder == ch)) {
	 sprintf(buf,"%s almost sings in your hands!!\n\r",
	 	obj->short_description);
	 	send_to_char(buf,ch);
	sprintf(buf,"You can hear $n's %s almost sing with joy!",
		obj->short_description);

	   act(buf,FALSE, ch, 0, 0, TO_ROOM);
	   return(FALSE);
	 }
       }
     }
   }
   return(FALSE);
 }

int GoodBlade(struct char_data *ch, int cmd, char *arg,struct obj_data *tobj, int type)
{
   extern struct str_app_type str_app[];
   struct obj_data *obj, *blade;
   struct char_data *joe, *holder;
   struct char_data *lowjoe = 0;
   char arg1[128], buf[250];

   if (type != PULSE_COMMAND)
     return(FALSE);

	return(FALSE); /* disabled */

   if (IS_IMMORTAL(ch)) return(FALSE);
   if (!real_roomp(ch->in_room)) return(FALSE);




   for (obj = real_roomp(ch->in_room)->contents;
	obj ; obj = obj->next_content) {
     if (obj_index[obj->item_number].func == GoodBlade) {
       /* I am on the floor */
       for (joe = real_roomp(ch->in_room)->people; joe ;
	    joe = joe->next_in_room) {
	 if ((GET_ALIGNMENT(joe) >= 350) && (!IS_IMMORTAL(joe))) {
	   if (lowjoe) {
	     if (GET_ALIGNMENT(joe) > GET_ALIGNMENT(lowjoe)){
	       lowjoe = joe;
	     }
	   } else lowjoe = joe;
	 }
       }
       if (lowjoe) {
	 if (CAN_GET_OBJ(lowjoe, obj)) {
	   obj_from_room(obj);
	   obj_to_char(obj,lowjoe);

	   sprintf(buf,"%s leaps into your hands!\n\r",obj->short_description);
	   send_to_char(buf,lowjoe);

	   sprintf(buf,"%s jumps from the floor and leaps into %s's hands!\n\r",
	    obj->short_description,GET_NAME(lowjoe));
	   act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);

	   if (!EgoBladeSave(lowjoe)) {
	     if (!lowjoe->equipment[WIELD]) {
	   sprintf(buf,"%s forces you to wield it!\n\r",obj->short_description);
           send_to_char(buf,lowjoe);
	       wear(lowjoe, obj, 12);
	       return(FALSE);
	     } else {
	       sprintf(buf,"You can feel %s attept to make you wield it.\n\r",
                 obj->short_description);
	       send_to_char(buf,lowjoe);
	       return(FALSE);
	     }
	   }
	 }
       }
     }
   }
   for (holder = real_roomp(ch->in_room)->people; holder ;
	holder = holder->next_in_room) {
     for (obj = holder->carrying; obj ; obj = obj->next_content) {
       if ((obj_index[obj->item_number].func) &&
	   (obj_index[obj->item_number].func != board)){
	 /*held*/
	 if (holder->equipment[WIELD]) {
	   if ((!EgoBladeSave(holder)) && (!EgoBladeSave(holder))) {
	sprintf(buf,"%s gets pissed off that you are wielding another weapon!\n\r",
        obj->short_description);
	send_to_char(buf,holder);
	sprintf(buf,"%s knocks %s out of your hands!!\n\r",obj->short_description,
	holder->equipment[WIELD]->short_description);
	send_to_char(buf,holder);
	     blade = unequip_char(holder,WIELD);
	     if (blade) obj_to_room(blade,holder->in_room);
	     if (!holder->equipment[WIELD]) {
	     sprintf(buf,"%s forces you to wield it!\n\r",
	       obj->short_description);
	send_to_char(buf,holder);
	       wear(holder, obj, 12);
	       return(FALSE);
	     }
	   }
	 }
	 if (!EgoBladeSave(holder)) {
	   if (!EgoBladeSave(holder)) {
	     if (!holder->equipment[WIELD]) {
	     sprintf(buf,"%s forces you yto wield it!\n\r",
	       obj->short_description);
	send_to_char(buf,holder);
	       wear(holder, obj, 12);
	       return(FALSE);
	     }
	   }
	 }
	 if (affected_by_spell(holder,SPELL_CHARM_PERSON)) {
	   affect_from_char(holder,SPELL_CHARM_PERSON);
	  sprintf(buf,"Due to the effects of %s, you feel less enthused about your master.\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }
       }
     }
     if (holder->equipment[WIELD])
     {
       if ((obj_index[holder->equipment[WIELD]->item_number].func)
	   && (obj_index[holder->equipment[WIELD]->item_number].func != board)){
	 /*YES! I am being held!*/
	 obj = holder->equipment[WIELD];

		/* remove charm */
	 if (affected_by_spell(holder,SPELL_CHARM_PERSON)) 	 {
	   affect_from_char(holder,SPELL_CHARM_PERSON);
	  sprintf(buf,"Due to the effects of %s, you feel less enthused about your master.\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }

		/* remove blindness */
	 if (affected_by_spell(holder,SPELL_BLINDNESS)) 	 {
	   affect_from_char(holder,SPELL_BLINDNESS);
	  sprintf(buf,"%s hums in your hands, you can see!\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }

	/* remove chill touch */
	 if (affected_by_spell(holder,SPELL_CHILL_TOUCH)) 	 {
	   affect_from_char(holder,SPELL_CHILL_TOUCH);
	  sprintf(buf,"%s hums in your hands, you feel warm again!\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }


	/* remove slow */
	 if (affected_by_spell(holder,SPELL_SLOW)) 	 {
	   affect_from_char(holder,SPELL_SLOW);
	  sprintf(buf,"%s hums in your hands, you feel yourself speed back up!\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }

	/* remove poison */
	 if (affected_by_spell(holder,SPELL_POISON)) 	 {
	   affect_from_char(holder,SPELL_POISON);
	  sprintf(buf,"%s hums in your hands, the sick feeling fades!\n\r",
	  	obj->short_description);
	   send_to_char(buf,holder);
	 }

		/* wielder is hurt, heal them */
if ((number(1,101) > 90) && GET_HIT(holder) < GET_MAX_HIT(holder)/2) {
    act("You get a gentle warm pulse from $p, you feel MUCH better!",FALSE,holder,obj,0,TO_CHAR);
    act("$n smiles as $p pulses in $s hands!",FALSE,holder,obj,0,TO_ROOM);
    GET_HIT(holder) = (GET_MAX_HIT(holder)-number(1,10));
    return(FALSE);
  }

	 if (holder->specials.fighting) {
	sprintf(buf,"%s almost sings in your hand!\n\r",
	  obj->short_description);
	  send_to_char(buf,holder);
sprintf(buf,"You can hear %s almost sing with joy in $n's hands!",obj->short_description);
	   act(buf,FALSE, holder, 0, 0, TO_ROOM);
	   if ((holder == ch) && (cmd == 151)) {
	     if (EgoBladeSave(ch) && EgoBladeSave(ch)) {
	      sprintf(buf,"You can feel %s attempt to stay in the fight!\n\r",
	      		obj->short_description);
	      		send_to_char(buf,ch);
	       return(FALSE);
	     } else {
	     sprintf(buf,"%s laughs at your attempt to flee from a fight!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);
	       GET_HIT(ch) -= 10;
	       if (GET_HIT(ch) < 0) {
		 GET_HIT(ch) = 0;
		 GET_POS(ch) = POSITION_STUNNED;
	       }
	       return(TRUE);
	     }
	   }
	 }
	 if ((cmd == 66) && (holder == ch)) {
	   one_argument(arg, arg1);
	   if (strcmp(arg1,"all") == 0) {
	     if (!EgoBladeSave(ch)) {

	     sprintf(buf,"%s laughs at your attempt remove it!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);

	       GET_HIT(ch) -= 10;
	       if (GET_HIT(ch) < 0) {
		 GET_HIT(ch) = 0;
		 GET_POS(ch) = POSITION_STUNNED;
	       }
	       return(TRUE);
	     } else {
	     sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",
	     	obj->short_description);
	     	send_to_char(buf,ch);
	       return(FALSE);
	     }
	   } else {
	     if (isname(arg1,obj->name)) {
	       if (!EgoBladeSave(ch)) {

	     sprintf(buf,"%s laughs at your attempt to remove it!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	sprintf(buf,"%s gives you a little warning...\n\r",
		obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"%s twists around and smacks you!\n\r",
        	obj->short_description);
	send_to_char(buf,ch);
	sprintf(buf,"Wow! $n's %s just whipped around and smacked $m one!",
	obj->short_description);
	act(buf,FALSE, ch, 0, 0, TO_ROOM);

		 GET_HIT(ch) -= 10;
		 if (GET_HIT(ch) < 0) {
		   GET_HIT(ch) = 0;
		   GET_POS(ch) = POSITION_STUNNED;
		 }
		 return(TRUE);
	       } else {
	       sprintf(buf,"You can feel %s attempt to stay wielded!\n\r",obj->short_description);
	       send_to_char(buf,ch);
		 return(FALSE);
	       }
	     }
	   }
	 }
	 for (joe = real_roomp(holder->in_room)->people; joe ;
	      joe = joe->next_in_room) {
	   if ((GET_ALIGNMENT(joe) <=-350) &&
	       (IS_MOB(joe)) && (CAN_SEE(holder,joe)) && (holder != joe)) {
	     if (lowjoe) {
	       if (GET_ALIGNMENT(joe) < GET_ALIGNMENT(lowjoe)){
		 lowjoe = joe;
	       }
	     } else lowjoe = joe;
	   }
	 }
	 if (lowjoe) {
	   if (!EgoBladeSave(holder)) {
	     if (GET_POS(holder) != POSITION_STANDING) {
	     sprintf(buf,"%s yanks you yo your feet!\n\r",
	       obj->short_description);
	       send_to_char(buf,ch);
	       GET_POS(holder) = POSITION_STANDING;
	     }
	     sprintf(buf,"%s leaps out of control!!\n\r",
	       obj->short_description);
	       send_to_char(buf,holder);
	sprintf(buf,"%s howls out for $n's neck!",obj->short_description);
	     act(buf,FALSE, lowjoe, 0, 0, TO_ROOM);
	     do_hit(holder,lowjoe->player.name, 0);
	     return(TRUE);
	   } else {
	     return(FALSE);
	   }
	 }
	 if ((cmd == 70) && (holder == ch)) {
	 sprintf(buf,"%s almost sings in your hands!!\n\r",
	 	obj->short_description);
	 	send_to_char(buf,ch);
	sprintf(buf,"You can hear $n's %s almost sing with joy!",
		obj->short_description);

	   act(buf,FALSE, ch, 0, 0, TO_ROOM);
	   return(FALSE);
	 }
       }
     }
   }

   return(FALSE);
 }

int NeutralBlade(struct char_data *ch, int cmd, char *arg,struct obj_data *tobj, int type)
{
 return(FALSE);
}

#endif

int FireBreather(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *tar_char;

 if (cmd)
    return(FALSE);

 if (ch->specials.fighting && number(0,2)) {
   act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
   act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
   for(tar_char=real_roomp(ch->in_room)->people;tar_char;tar_char=tar_char->next_in_room) {
   if (!IS_IMMORTAL(tar_char))
     spell_fire_breath(GetMaxLevel(ch),ch,tar_char,0);
   } /* end for */

   return(TRUE);
 }

 return(FALSE);
}

int FrostBreather(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *tar_char;
 if (cmd)
    return(FALSE);

 if (ch->specials.fighting && number(0,2)) {
   act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
   act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
   for(tar_char=real_roomp(ch->in_room)->people;tar_char;tar_char=tar_char->next_in_room) {
   if (!IS_IMMORTAL(tar_char))
   spell_frost_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
 } /* end for */
   return(TRUE);
 }

 return(FALSE);
}

int AcidBreather(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *tar_char;
 if (cmd)
    return(FALSE);

 if (ch->specials.fighting && number(0,2)) {
   act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
   act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
   for(tar_char=real_roomp(ch->in_room)->people;tar_char;tar_char=tar_char->next_in_room) {
   if (!IS_IMMORTAL(tar_char))
   spell_acid_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
  }
   return(TRUE);
 }

 return(FALSE);
}

int GasBreather(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *tar_char;

 if (cmd)
    return(FALSE);

 if (ch->specials.fighting && number(0,2)) {
   act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
   act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
   for(tar_char=real_roomp(ch->in_room)->people;tar_char;tar_char=tar_char->next_in_room) {
   if (!IS_IMMORTAL(tar_char))
   spell_gas_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
  }
   return(TRUE);
 }

 return(FALSE);
}


int LightningBreather(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
struct char_data *tar_char;

 if (cmd)
    return(FALSE);

 if (ch->specials.fighting && number(0,2)) {
   act("$n rears back and inhales",FALSE,ch,0,0,TO_ROOM);
   act("$n breaths...",FALSE,ch,0,0,TO_ROOM);
   for(tar_char=real_roomp(ch->in_room)->people;tar_char;tar_char=tar_char->next_in_room) {
   if (!IS_IMMORTAL(tar_char))
    spell_lightning_breath(GetMaxLevel(ch),ch,ch->specials.fighting,0);
   }
   return(TRUE);
 }

 return(FALSE);
}

int magic_user_imp(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;
  byte lspell;
  char buf[254];


  if (cmd || !AWAKE(ch) || IS_AFFECTED(ch, AFF_PARALYSIS))
    return(FALSE);

/* might move this somewhere else later... */

 SET_BIT(ch->player.class, CLASS_MAGIC_USER);
 ch->player.level[MAGE_LEVEL_IND] = GetMaxLevel(ch);

/* --- */

  if (!ch->specials.fighting && !IS_PC(ch)) {

     if (GetMaxLevel(ch) < 25)
        return FALSE;
     else {
       if (!ch->desc) {
          if (Summoner(ch, cmd, arg, mob, type))
	    return(TRUE);
	  else {
	    if (NumCharmedFollowersInRoom(ch) < 5 && IS_SET(ch->hatefield, HATE_CHAR)) {
	      act("$n utters the words 'Here boy!'.", 1, ch, 0, 0, TO_ROOM);
	      cast_mon_sum7(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	      do_order(ch, "followers guard on", 0);
	      return(TRUE);
	    }
	  }
       }
       return;
     }
  }

  if (!ch->specials.fighting)
    return;

  if (!IS_PC(ch)) {

    if ((GET_POS(ch) > POSITION_STUNNED) &&
	(GET_POS(ch) < POSITION_FIGHTING)) {
      if (GET_HIT(ch) > GET_HIT(ch->specials.fighting)/2)
	StandUp(ch);

         else {
        StandUp(ch);
        WAIT_STATE(ch, PULSE_VIOLENCE*3);
	do_flee(ch, "\0", 0);
      }
      WAIT_STATE(ch, PULSE_VIOLENCE*3);
      return(TRUE);
    } /* had to stand */

  }

  if (check_nomagic(ch, 0, 0))
    return(FALSE);

if (number(0,1))
 if (UseViolentHeldItem(ch))
     return(TRUE);

  if (check_soundproof(ch)) return(FALSE);

  /* Find a dude to to evil things upon ! */

  vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict) return(FALSE);

  lspell = number(0,GetMaxLevel(ch)); /* gen number from 0 to level */
  if (!IS_PC(ch)) {
    lspell+= GetMaxLevel(ch)/5;   /* weight it towards the upper levels of
				     the mages range */
  }
  lspell = MIN(GetMaxLevel(ch), lspell);

  /*
  **  check your own problems:
  */

  if (lspell < 1)
    lspell = 1;

/*
** only problem I can see with this new spell casting is if the mobs
** name is the same as the victim....
*/

  if (IS_AFFECTED(ch, AFF_BLIND) && (lspell > 15)) {
     sprintf(buf," 'remove blind' %s",GET_NAME(ch));
     do_cast(ch,buf,0);
    return TRUE;
  }

  if (IS_AFFECTED(ch, AFF_BLIND))
    return(FALSE);

  if ((IS_AFFECTED(vict, AFF_SANCTUARY)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict)))) {
    sprintf(buf," 'dispel magic' %s",GET_NAME(vict));
    do_cast(ch,buf,0);
    return(FALSE);
  }

  if ((IS_AFFECTED(vict, AFF_FIRESHIELD)) && (lspell > 10) &&
      (GetMaxLevel(ch) > (GetMaxLevel(vict)))) {
     sprintf(buf," 'dispel magic' %s",GET_NAME(vict));
     do_cast(ch,buf,0);
    return(FALSE);

  }

  if (!IS_PC(ch)) {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 28) &&
	!IS_SET(ch->specials.act, ACT_AGGRESSIVE)) {

#if 0
      vict = FindMobDiffZoneSameRace(ch);
      if (vict) {
	spell_teleport_wo_error(GetMaxLevel(ch), ch, vict, 0);
	return(TRUE);
      }
#endif
	sprintf(buf," 'teleport' %s",GET_NAME(ch));
	do_cast(ch,buf,0);
        return(FALSE);
    }
  }

  if (!IS_PC(ch)) {
    if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 15) &&
	(!IS_SET(ch->specials.act, ACT_AGGRESSIVE))) {
	sprintf(buf," 'teleport' %s",GET_NAME(ch));
	do_cast(ch,buf,0);
      return(FALSE);
    }
  }


  if  (GET_HIT(ch) > (GET_MAX_HIT(ch) / 2) &&
       !IS_SET(ch->specials.act, ACT_AGGRESSIVE) &&
       GetMaxLevel(vict) < GetMaxLevel(ch) && (number(0,1))) {

    /*
     **  Non-damaging case:
     */

    if (((lspell>8) && (lspell<50)) && (number(0,6)==0)) {
	sprintf(buf," 'web' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      return TRUE;
    }

    if (((lspell>5) && (lspell<10)) && (number(0,6)==0)) {
	sprintf(buf," 'weakness' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      return TRUE;
    }

    if (((lspell>5) && (lspell<10)) && (number(0,7)==0)) {
	sprintf(buf," 'armor' %s",GET_NAME(ch));
	do_cast(ch,buf,0);
      return TRUE;
    }

    if (((lspell>12) && (lspell<20)) && (number(0,7)==0))	{
	sprintf(buf," 'curse' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      return TRUE;
    }

    if (((lspell>10) && (lspell < 20)) && (number(0,5)==0)) {
	sprintf(buf," 'blind' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      return TRUE;
    }

    if (((lspell>8) && (lspell < 40)) && (number(0,5)==0) &&
	(vict->specials.fighting != ch)) {
	sprintf(buf," 'charm monster' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      if (IS_AFFECTED(vict, AFF_CHARM)) {
	if (!vict->specials.fighting) {
	  sprintf(buf, "%s kill %s",
		  GET_NAME(vict), GET_NAME(ch->specials.fighting));
	  do_order(ch, buf, 0);
	} else {
	  sprintf(buf, "%s remove all", GET_NAME(vict));
	  do_order(ch, buf, 0);
	}
      }
    }

    /*
    **  The really nifty case:
    */
      switch(lspell) {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
      case 10:
	sprintf(buf," 'monsum one'");
	do_cast(ch,buf,0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 11:
      case 12:
      case 13:
	sprintf(buf," 'monsum two'");
	do_cast(ch,buf,0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 14:
      case 15:
	sprintf(buf," 'monsum three'");
	do_cast(ch,buf,0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 16:
      case 17:
      case 18:
	sprintf(buf," 'monsum four'");
	do_cast(ch,buf,0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 19:
      case 20:
      case 21:
      case 22:
	sprintf(buf," 'monsum five'");
	do_cast(ch,buf,0);
        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 23:
      case 24:
      case 25:
	sprintf(buf," 'monsum six'");
	do_cast(ch,buf,0);

        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      case 26:
      default:
	sprintf(buf," 'monsum seven'");
	do_cast(ch,buf,0);

        do_order(ch, "followers guard on", 0);
        return(TRUE);
	break;
      }

  } else {

/*
*/

  switch (lspell) {
  case 1:
  case 2:
	sprintf(buf," 'magic missle' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
    break;
  case 3:
  case 4:
  case 5:
	sprintf(buf," 'shocking grasp' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
    break;
  case 6:
  case 7:
  case 8:
      if (ch->attackers <= 2) {
        sprintf(buf," 'web' %s",GET_NAME(vict));
        do_cast(ch,buf,0);
        break;
      } else {
	sprintf(buf," 'burning hands' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
	break;
      }
  case 9:
  case 10:
	sprintf(buf," 'acid blast' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
  case 11:
  case 12:
  case 13:
    if (ch->attackers <= 2) {
	sprintf(buf," 'lightning bolt' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
    } else {
	sprintf(buf," 'ice storm' %s", GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
    }
  case 14:
  case 15:
	sprintf(buf," 'teleport' %s",GET_NAME(ch));
	do_cast(ch,buf,0);
    break;
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
  case 24:
  case 25:
  case 26:
  case 27:
  case 28:
  case 29:
    if (ch->attackers <= 2) {
	sprintf(buf," 'colour spray' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
    } else {
	sprintf(buf," 'cone of cold' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
    }
  case 30:
  case 31:
  case 32:
  case 33:
  case 34:
  case 35:
  case 36:
  case 37:
	sprintf(buf," 'fireball' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
    break;
  case 38:
  case 39:
  case 40:
  case 41:
    if (IS_EVIL(ch))	{
	sprintf(buf," 'energy drain' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
      break;
    }
  default:
    if (ch->attackers <= 2) {
	sprintf(buf," 'meteor swarm' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
       break;
     } else {
	sprintf(buf," 'fireball' %s",GET_NAME(vict));
	do_cast(ch,buf,0);
       break;
     }
  }
}
  return TRUE;
}




int cleric_imp(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *vict;
  byte lspell, healperc=0;


  if (cmd || !AWAKE(ch))
    return(FALSE);

  if (GET_POS(ch)!=POSITION_FIGHTING) {
    if ((GET_POS(ch)<POSITION_STANDING) && (GET_POS(ch)>POSITION_STUNNED)) {
      StandUp(ch);
    }
    return FALSE;
  }

  if (check_soundproof(ch)) return(FALSE);

  if (check_nomagic(ch, 0, 0))
    return(FALSE);


  if (!ch->specials.fighting) {
    if (GET_HIT(ch) < GET_MAX_HIT(ch)-10) {
      if ((lspell = GetMaxLevel(ch)) >= 20) {
	act("$n utters the words 'What a Rush!'.", 1, ch,0,0,TO_ROOM);
	cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      } else if (lspell > 12) {
      act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch,0,0,TO_ROOM);
	cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      } else if (lspell > 8) {
      act("$n utters the words 'I feel much better now!'.", 1, ch,0,0,TO_ROOM);
        cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      } else {
	act("$n utters the words 'I feel good!'.", 1, ch,0,0,TO_ROOM);
	cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      }
    }
  }


  /* Find a dude to to evil things upon ! */

  if ((vict = FindAHatee(ch))==NULL)
     vict = FindVictim(ch);

  if (!vict)
    vict = ch->specials.fighting;

  if (!vict) return(FALSE);

  /*
    gen number from 0 to level
    */

  lspell = number(0,GetMaxLevel(ch));
  lspell+= GetMaxLevel(ch)/5;
  lspell = MIN(GetMaxLevel(ch), lspell);

  if (lspell < 1)
    lspell = 1;


  if ((GET_HIT(ch) < (GET_MAX_HIT(ch) / 4)) && (lspell > 31) &&
      (!IS_SET(ch->specials.act, ACT_AGGRESSIVE))) {
    act("$n utters the words 'Woah! I'm outta here!'",
	1, ch, 0, 0, TO_ROOM);
    vict = FindMobDiffZoneSameRace(ch);
    if (vict) {
      cast_astral_walk(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(TRUE);
    }
    cast_teleport(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
    return(FALSE);
  }


  /*
    first -- hit a foe, or help yourself?
    */

  if (ch->points.hit < (ch->points.max_hit / 2))
    healperc = 7;
  else if (ch->points.hit < (ch->points.max_hit / 4))
    healperc = 5;
  else if (ch->points.hit < (ch->points.max_hit / 8))
    healperc=3;

  if (number(1,healperc+2)>3) {
    /* do harm */

    /* call lightning */
    if (OUTSIDE(ch) && (weather_info.sky>=SKY_RAINING) && (lspell >= 15) &&
	(number(0,5)==0)) {
      act("$n whistles.",1,ch,0,0,TO_ROOM);
      act("$n utters the words 'Here Lightning!'.",1,ch,0,0,TO_ROOM);
      cast_call_lightning(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      return(TRUE);
    }

    switch(lspell) {
    case 1:
    case 2:
    case 3:
      act("$n utters the words 'Moo ha ha!'.",1,ch,0,0,TO_ROOM);
      cast_cause_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 4:
    case 5:
    case 6:
      act("$n utters the words 'Hocus Pocus!'.",1,ch,0,0,TO_ROOM);
      cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 7:
      act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
      cast_dispel_magic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 8:
      act("$n utters the words 'Urgle Blurg'.",1,ch,0,0,TO_ROOM);
      cast_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 9:
    case 10:
      act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
      cast_cause_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 11:
      act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
      cast_flamestrike(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    case 13:
    case 14:
    case 15:
    case 16:
      {
	  if (!IS_SET(vict->M_immune, IMM_FIRE)) {
	    act("$n utters the words 'Burn Baby Burn'.",1,ch,0,0,TO_ROOM);
	    cast_flamestrike(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
	  } else if (IS_AFFECTED(vict, AFF_SANCTUARY) &&
		     ( GetMaxLevel(ch) > GetMaxLevel(vict))) {
	    act("$n utters the words 'Va-Voom!'.",1,ch,0,0,TO_ROOM);
	    cast_dispel_magic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL,vict,0);
	  } else {
	    act("$n utters the words 'Take That!'.",1,ch,0,0,TO_ROOM);
	    cast_cause_critic(GetMaxLevel(ch),ch,"",SPELL_TYPE_SPELL, vict, 0);
	  }
       	break;
      }
    case 17:
    case 18:
    case 19:
    default:
      act("$n utters the words 'Hurts, doesn't it??'.",1,ch,0,0,TO_ROOM);
      cast_harm(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, vict, 0);
      break;
    }

    return(TRUE);

  } else {
    /* do heal */

    if (IS_AFFECTED(ch, AFF_BLIND) && (lspell >= 4) & (number(0,3)==0)) {
      act("$n utters the words 'Praise <Deity Name>, I can SEE!'.", 1, ch,0,0,TO_ROOM);
      cast_cure_blind( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

    if (IS_AFFECTED(ch, AFF_CURSE) && (lspell >= 6) && (number(0,6)==0)) {
      act("$n utters the words 'I'm rubber, you're glue.", 1, ch,0,0,TO_ROOM);
      cast_remove_curse(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }

    if (IS_AFFECTED(ch, AFF_POISON) && (lspell >= 5) && (number(0,6)==0)) {
      act("$n utters the words 'Praise <Deity Name> I don't feel sick no more!'.", 1, ch,0,0,TO_ROOM);
      cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
    }


    switch(lspell) {
    case 1:
    case 2:
      act("$n utters the words 'Abrazak'.",1,ch,0,0,TO_ROOM);
      cast_armor(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 3:
    case 4:
    case 5:
      act("$n utters the words 'I feel good!'.", 1, ch,0,0,TO_ROOM);
      cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
      act("$n utters the words 'I feel much better now!'.", 1, ch,0,0,TO_ROOM);
      cast_cure_serious(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
      act("$n utters the words 'Woah! I feel GOOD! Heh.'.", 1, ch,0,0,TO_ROOM);
      cast_cure_critic(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    case 17:
    case 18: /* heal */
      act("$n utters the words 'What a Rush!'.", 1, ch,0,0,TO_ROOM);
      cast_heal(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;
    default:
      act("$n utters the words 'Oooh, pretty!'.", 1, ch,0,0,TO_ROOM);
      cast_sanctuary(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      break;

    }

    return(TRUE);

  }
}

int lich_church(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
if (type == EVENT_DEATH) {
	 /* add spec procs here for when he dies */

   return(FALSE);
  }

 if (number(0,1))
   return(magic_user(ch,cmd,arg,mob,type));
    else
   return(shadow(ch,cmd,arg,mob,type));
}
int medusa(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *tar;
  int i;

  if (cmd || !AWAKE(ch))
    return(FALSE);

  for (i=0;i<8;i++) {
    if ((tar = FindAHatee(ch)) == NULL)
        tar = FindVictim(ch);

    if (tar && (tar->in_room == ch->in_room)) {
      if (HitOrMiss(ch, tar, CalcThaco(ch))) {
	act("$n glares at $N with an evil eye!", 1, ch, 0, tar, TO_NOTVICT);
	act("$n glares at you with and evil eye!", 1, ch, 0, tar, TO_VICT);
	if (!IS_AFFECTED(tar, AFF_PARALYSIS)) {
	  cast_paralyze( GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL,tar, 0);
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}





int goblin_sentry(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *sentrymob;

  if (!AWAKE(ch))
    return(FALSE);


  if (!cmd) {
    if (ch->specials.fighting) {
      fighter(ch, cmd, arg, mob, type);
    }
  } else {
   sentrymob = FindMobInRoomWithFunction(ch->in_room, goblin_sentry);
   if (cmd >= 1 && cmd <= 6) {
    if (cmd == 3) return(FALSE);  /* can always go south */
    		/* everything else gets ya attacked */
    if (ch->desc && !IS_IMMORTAL(ch)) {
    if (number(0,1) && GET_RACE(ch) != RACE_GOBLIN && CAN_SEE(sentrymob,ch)) {
	act("$N blocks your path and attacks!",FALSE,ch,0,sentrymob, TO_CHAR);
        act("$N growls at $n and attacks!",TRUE, ch, 0, sentrymob, TO_ROOM);
        hit(sentrymob,ch,0);
      return(TRUE);
     }
    }
   }
  }
  return(FALSE);
}


int PostMaster(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

/*****************************************************************
** Below is the spec_proc for a postmaster using the above       **
** routines.  Written by Jeremy Elson (jelson@server.cs.jhu.edu) **
*****************************************************************/

   if (!ch->desc)
      return(FALSE); /* so mobs don't get caught here */

   switch (cmd) {
   case CMD_MAIL: /* mail */
      postmaster_send_mail(ch, cmd, arg);
      return 1;
      break;
   case CMD_CHECK: /* check */
      postmaster_check_mail(ch, cmd, arg);
      return 1;
      break;
   case CMD_RECEIVE: /* receive */
      postmaster_receive_mail(ch, cmd, arg);
      return 1;
      break;
   default:
      return(FALSE);
      break;
   }

   return(FALSE);
}

int TreeThrowerMob(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
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
      switch(ch->in_room) {
      case 13912:  /*Forest of Rhowyn, Bridge Troll*/
	ThrowChar(ch, vict, 1);  /* throw chars to the east */
	return(FALSE);
        break;
      case 6224:   /*Rhyiana, Sentinel Tree*/
        ThrowChar(ch, vict, 2); /*throw chars to the south*/
        return(FALSE);
        break;
      default:
	return(FALSE);
      }
    }
  } else {
    switch(ch->in_room) {
    case 13912:  /*Forest of Rhowyn, Bridge Troll*/
      if (cmd == 1)
         {   /* north+1 */
          act("$n blocks your way!", FALSE, mob, 0, ch, TO_VICT);
          act("$n blocks $N's way.", FALSE, mob, 0, ch, TO_NOTVICT);
          do_say(mob,"In order to pass, you must first defeat me!",1);
          do_action(mob,NULL,97); /*evil grin ;) */
          return(TRUE);
         }
      break;
    case 6224:   /*Rhyiana, Sentinel Tree*/
      if ((cmd == 1) && (IS_EVIL(ch)))
         {
          act("The trees around you rustle loudly.",FALSE,mob,0,ch,TO_VICT);
          act("Suddenly, branches reach out, blocking your way!",
               FALSE,mob,0,ch,TO_VICT);
          act("The branches of a huge tree move suddenly, blocking $N's way!",
              FALSE,mob,0,ch,TO_NOTVICT);
          act("A low rumbling voice whispers in your ear, 'The way north is forbidden to those of evil.'",FALSE,mob,0,ch,TO_VICT);
          act("The trees seem to be whispering in the wind.",
              FALSE,mob,0,ch,TO_NOTVICT);
          return(TRUE);
         }
      break;
    default:
      return(FALSE);
    } /*end of switch*/
  }   /*end of else*/
  return(FALSE);
}




/*
 well, this paladin will give a tough battle..
 imho paladins just can't be stupid and do _random_ things on yourself
 and on opponents.. grin

*/

int Paladin(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type) {

struct char_data *vict, *tch;
char    buf[255];

if (!ch->skills || GET_LEVEL(ch,PALADIN_LEVEL_IND)<=0) {  /* init skills */
    SET_BIT(ch->player.class, CLASS_PALADIN);
    if(!ch->skills) SpaceForSkills(ch);
    GET_LEVEL(ch,PALADIN_LEVEL_IND) = GetMaxLevel(ch); /* needed in do_ */

        /* set skill levels */

       ch->skills[SKILL_RESCUE].learned = GetMaxLevel(ch)+40;
       ch->skills[SKILL_HOLY_WARCRY].learned = GetMaxLevel(ch)+40;
       ch->skills[SKILL_LAY_ON_HANDS].learned = GetMaxLevel(ch)+40;
       ch->skills[SKILL_BASH].learned = GetMaxLevel(ch)+40;
       ch->skills[SKILL_KICK].learned = GetMaxLevel(ch)+40;
       ch->skills[SKILL_BLESSING].learned = GetMaxLevel(ch)+40;

  }     /* done with setting skills */

if(cmd) {
   /* we will not give free blesses or lay on hands..
       we will just ignore all commands.. yah. */

   return(FALSE);
}

if (affected_by_spell(ch,SPELL_PARALYSIS))                /* poor guy.. */
        return(FALSE);

if (!AWAKE(mob) && !affected_by_spell(mob,SPELL_SLEEP)) {
     /* hey, why you sleeping guy?  STAND AND FIGHT! */
     command_interpreter(mob,"wak");
     command_interpreter(mob,"stand");
     return(TRUE);
 }

if (ch->specials.fighting && ch->specials.fighting != ch)       {

        if (GET_POS(ch) == POSITION_SITTING || GET_POS(ch) == POSITION_RESTING) {
                do_stand(ch,"",0);
                return(TRUE);
        }

        vict = ch->specials.fighting;

        if (!vict) {
             log("!vict in paladin");
             return(FALSE);
        }


        /* well, if we in battle, do some nice things on ourself..      */
        if (!affected_by_spell(ch,SKILL_BLESSING)) {
                sprintf(buf,"%s",GET_NAME(ch)); /* bless myself */
                do_blessing(ch,buf,0);
                return(TRUE);
        }

        if (!affected_by_spell(ch,SKILL_LAY_ON_HANDS) && (GET_HIT(ch) < GET_MAX_HIT(ch)/2)) {
                sprintf(buf,"%s",GET_NAME(ch));
                do_lay_on_hands(ch,buf,0);
                return(TRUE);
        }

        switch(number(1,6)) {
                case 1: /* intellegent kick/bash.. hmm  */
                        if( HasClass(vict, CLASS_SORCERER|CLASS_MAGIC_USER) ||
                            HasClass(vict, CLASS_CLERIC|CLASS_PSI)) {
                                do_bash(ch,"",0);
                                return(TRUE);
                                break;
                        } else {
                                do_kick(ch,"",0);
                                return(TRUE);
                                break;
                        }
			break;
                case 2: do_bash(ch,"",0);
                        return(TRUE);
                        break;
                case 3: do_kick(ch,"",0);
                        return(TRUE);
                        break;
                case 4: do_holy_warcry(ch,"",0);
                        return(TRUE);
                        break;
                default:
			return(fighter(ch,cmd,arg,mob,type));
                        break;
        } /* end switch */

 } /* end fighting */
 else {         /* not FIGHTING */

   /* check our hps */
   if (GET_HIT(ch) < GET_MAX_HIT(ch)/2 && !affected_by_spell(ch,SKILL_LAY_ON_HANDS)) {
      sprintf(buf,"%s",GET_NAME(ch));
      do_lay_on_hands(ch,buf,0);
      return(TRUE);
   }
   if (GET_HIT(ch) < GET_MAX_HIT(ch)/2 && GET_MANA(ch)>GET_MANA(ch)/2 && number(1,6)>4) {
      cast_cure_light(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	return(TRUE);
   }

   /* lets check some spells on us.. */
   if (IS_AFFECTED(ch, AFF_POISON) && (number(0,6)==0) && GET_LEVEL(ch,PALADIN_LEVEL_IND)>10) {
      act("$n asks $s deity to remove poison from $s blood!", 1, ch,0,0,TO_ROOM);
      if(GET_LEVEL(ch,PALADIN_LEVEL_IND)<40)
         cast_slow_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      else
         cast_remove_poison(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
      return(TRUE);
   }

   if (!IS_AFFECTED(ch, AFF_PROTECT_FROM_EVIL) && number(0,6)==0) {
      act("$n prayed to $s deity to protect $m from evil.", 1,ch,0,0,TO_ROOM);
      cast_protection_from_evil(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch, 0);
	return(TRUE);
   }

  /* i dont like GreetPeople behaivor.. lets make it this way */

  if (IS_SET(ch->specials.act, ACT_GREET)) {
     for (tch=real_roomp(ch->in_room)->people; tch; tch = tch->next_in_room) {
log("paladin trying greet someone..\n");
        if (!IS_NPC(tch) && !number(0,4) ) {
           if (GetMaxLevel(tch) > 5 && CAN_SEE(ch,tch)) {
              if(GET_ALIGNMENT(tch) >= 900) {
                 sprintf(buf, "bow %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 if(tch->player.sex == SEX_FEMALE)
                    do_say(ch, "Greetings, noble m'lady!",0);
                 else
                    do_say(ch, "Greetings, noble sir!",0);
              }
              else if(GET_ALIGNMENT(tch) >= 350) {
                 sprintf(buf, "smile %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 do_say(ch, "Greetings, adventurer",0);
              }
              else if(GET_ALIGNMENT(tch) >= -350) {
                 sprintf(buf, "wink %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 do_say(ch, "Your doing well on your path of Neutrality",0);
              }
              else if(GET_ALIGNMENT(tch) >= -750) {
                 sprintf(buf, "nod %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 do_say(ch, "May the prophet smile upon you",0);
              }
              else if(GET_ALIGNMENT(tch) >= -900) {
                 sprintf(buf, "wink %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 do_say(ch, "You falling in hands of evil, beware!", 0);
              }
	      else {
   	         /* hmm, not nice guy.. */
                 sprintf(buf, "glare %s", GET_NAME(tch));
                 command_interpreter(ch, buf);
                 do_say(ch, "I sense great evil here!", 0);
              }
              SET_BIT(ch->specials.act, ACT_GREET);
	      break;
           } /* endif of can_see */
        }
     }
  }
  else if (!number(0,50)) {
       REMOVE_BIT(ch->specials.act, ACT_GREET);
  }
}       /* end not fighting */
 return(FALSE);
}


/* psi stuff here */

/* PSI_CAN is a shorthand for me, use ONLY for PSIs*/
#define PSI_CAN(skill,level) (spell_info[(skill)].min_level_psi <= (level))

/* NOTAFF_N_LEARNED more shorthand, this one is general*/
#define NOTAFF_N_LEARNED(ch,spell,skill) (!affected_by_spell((ch),(spell))\
	&& ((ch)->skills[(skill)].learned))

/* SP(el)L_(and target's)N(a)ME :: puts spell name and target's name together*/
#define SPL_NME(tname,spell) (strcat( strcat( strcat( strcpy(buf,"'"), \
	spells[(spell)]), "' "), (tname)))

/* (Psychic)C(rush)_OR_B(last) if psi can do a crush he does, if not blast!*/
#define C_OR_B(ch,vict) ((ch)->skills[SKILL_PSYCHIC_CRUSH].learned ? \
        (mind_psychic_crush(PML,(ch),(vict),NULL)) : \
	(do_blast((ch),(vict)->player.name,1)) )

/* CAST_OR_BLAST checks to see if psi can cast special attack spell, if not
   it will either do a psyhic crush or it will do a psionic blast */
#define CAST_OR_BLAST(ch,vict,spell) ((ch)->skills[(spell)].learned ? \
	do_cast((ch), SPL_NME((vict)->player.name, (spell)-1),370) : \
        C_OR_B((ch),(vict)))


/* Bugs: if 2 mobs of same name in room, may cause problems.
         fix add macro to add those "-" between parts and check for number of
         mobs with same name in room and figure out which one you are!!!
       * add teleport, prob travel, and disguise */

int Psionist(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 int PML;          /*psi mob's level*/
 int Qmana;        /*psi mob's 1/4 mana*/
 int cmana;        /*psi mob's current mana level*/
 int hpcan;        /*safe to canibalize/summon*/
 char buf[100];    /*all purpose buffer*/
 struct char_data *targ;  /*who the psi_mob is fighting*/
 int group;        /* does targ have followers or is grouped? */

 if (cmd) return(FALSE);


 if (!AWAKE(mob) && !affected_by_spell(mob,SKILL_MEDITATE))
                  /*no sleeping on the job!!!!*/
    {		   /* NOTE: this also prevents mobs who are incap from casting*/
     command_interpreter(mob,"wake");
     command_interpreter(mob,"stand");
     return(TRUE);
    }

 if (!IS_SET(mob->player.class, CLASS_PSI))
    {
     SET_BIT(mob->player.class, CLASS_PSI);
     PML = GET_LEVEL(mob,PSI_LEVEL_IND) = GetMaxLevel(mob);
     SpaceForSkills(mob);
	  /* SetSkillLevels first defensive, general spells. */
          /* I set all of these so that gods running quests can use the
             psi mobs skills, unlike most other special procedure mobs*/
     if (PSI_CAN(SKILL_CANIBALIZE,PML))
  	mob->skills[SKILL_CANIBALIZE].learned= MIN(95,10+dice(10,(int)PML/2));
     if (PSI_CAN(SKILL_CELL_ADJUSTMENT,PML))
        mob->skills[SKILL_CELL_ADJUSTMENT].learned = MIN(95,10+dice(4,PML));
     if (PSI_CAN(SKILL_CHAMELEON,PML))
	mob->skills[SKILL_CHAMELEON].learned = MIN(95,50+number(2,PML));
     if (PSI_CAN(SKILL_FLAME_SHROUD,PML))
	mob->skills[SKILL_FLAME_SHROUD].learned = MIN(95,50+dice(5,PML));
     if (PSI_CAN(SKILL_GREAT_SIGHT,PML))
	mob->skills[SKILL_GREAT_SIGHT].learned = MIN(95,33+dice(4,PML));
     if (PSI_CAN(SKILL_INVIS,PML))
	mob->skills[SKILL_INVIS].learned = MIN(95,50+dice(3,PML));
     if (PSI_CAN(SKILL_MEDITATE,PML))
	mob->skills[SKILL_MEDITATE].learned = MIN(95,33+dice((int)PML/5,20));
     if (PSI_CAN(SKILL_MINDBLANK,PML))
	mob->skills[SKILL_MINDBLANK].learned = MIN(95,50+dice(4,PML));
     if (PSI_CAN(SKILL_PORTAL,PML))
        mob->skills[SKILL_PORTAL].learned = MIN(95,40+dice(4,PML));
     if (PSI_CAN(SKILL_PROBABILITY_TRAVEL,PML))
        mob->skills[SKILL_PROBABILITY_TRAVEL].learned=MIN(95,35+dice(3,PML));
     if (PSI_CAN(SKILL_PSI_SHIELD,PML))
        mob->skills[SKILL_PSI_SHIELD].learned = MIN(95,66+dice(10,(int) PML/3));
     if (PSI_CAN(SKILL_PSI_STRENGTH,PML))
        mob->skills[SKILL_PSI_STRENGTH].learned = MIN(95,dice((int)PML/5,20));
     if (PSI_CAN(SKILL_PSYCHIC_IMPERSONATION,PML))
        mob->skills[SKILL_PSYCHIC_IMPERSONATION].learned=MIN(95,66+dice(2,PML));
     if (PSI_CAN(SKILL_SUMMON,PML))
        mob->skills[SKILL_SUMMON].learned = MIN(95,66+PML);
     if (PSI_CAN(SKILL_TOWER_IRON_WILL,PML))
	mob->skills[SKILL_TOWER_IRON_WILL].learned = MIN(95,50+dice(4,PML));
/*ATTACK -- TYPE SPELLS BEGIN HERE*/
     if (PSI_CAN(SKILL_MIND_BURN,PML))
        mob->skills[SKILL_MIND_BURN].learned = MIN(95,33+dice(2,PML));
     if (PSI_CAN(SKILL_DISINTERGRATE,PML))
        mob->skills[SKILL_DISINTERGRATE].learned = MIN(95,33+dice(3,PML));
     if (PSI_CAN(SKILL_MIND_WIPE,PML))
	mob->skills[SKILL_MIND_WIPE].learned = MIN(95,45+dice(2,PML));
     if (PSI_CAN(SKILL_PSIONIC_BLAST,PML))
      mob->skills[SKILL_PSIONIC_BLAST].learned= MIN(99,33+dice((int)PML/10,30));
     if (PSI_CAN(SKILL_PSYCHIC_CRUSH,PML))
	mob->skills[SKILL_PSYCHIC_CRUSH].learned = MIN(99,66+dice(2,PML));
     if (PSI_CAN(SKILL_TELEKINESIS,PML))
	mob->skills[SKILL_TELEKINESIS].learned = MIN(95,dice((int)PML/10,30));
     if (PSI_CAN(SKILL_PSI_TELEPORT,PML))
	mob->skills[SKILL_PSI_TELEPORT].learned = MIN(95,dice((int)PML/10,30));
     if (PSI_CAN(SKILL_ULTRA_BLAST,PML))
	mob->skills[SKILL_ULTRA_BLAST].learned = MIN(99,66+dice(3,PML));
     mob->points.mana = 100;
    }

/* Aarcerak's little bug fix.. sitting mobs can't cast/mind/etc.. */
  if (mob->specials.fighting) {
    if((GET_POS(mob)<POSITION_FIGHTING) && (GET_POS(mob)>POSITION_STUNNED)) {
      StandUp(mob);
      return (TRUE);
    }
  }


 if (affected_by_spell(mob,SPELL_FEEBLEMIND))
    {
     if (!IS_AFFECTED(mob,AFF_HIDE))
        act("$n waits for $s death blow impatiently.",FALSE,mob,0,0,TO_ROOM);
     return(TRUE);
    }

  if (GET_POS(mob) == POSITION_SITTING ||
      GET_POS(mob) == POSITION_RESTING) {
      do_stand(mob,"",0);
      return(TRUE);
    }

 PML = GET_LEVEL(mob,PSI_LEVEL_IND);

 if (NOTAFF_N_LEARNED(mob,SPELL_FIRESHIELD,SKILL_FLAME_SHROUD))
    {
     do_flame_shroud(mob,mob->player.name,1);
     return(TRUE);
    }

 if (affected_by_spell(mob,SKILL_MEDITATE))
    {
    if (mob->points.mana <= .75*mob->points.max_mana)
        return(TRUE); /*regaining mana*/
      else
       {
        command_interpreter(mob,"stand");
        return(TRUE);
       }
    }
 if (IS_AFFECTED(mob,AFF_HIDE)) return (FALSE); /*hiding, break fer flame shrd*/

 Qmana = 51;
 hpcan = (int) (.75 * mob->points.max_hit);
 cmana = mob->points.mana;

 if (!mob->specials.fighting)
    {
     if (PSI_CAN(SKILL_CELL_ADJUSTMENT, PML) && (cmana > Qmana) &&
	  (mob->points.hit < hpcan) )
	{
         do_say(mob,"That was too close for comfort.",1);
         mind_teleport(PML,mob,mob,NULL);
	 mind_cell_adjustment(PML, mob, mob, NULL);
	 return(TRUE);
	}
     if (cmana <= Qmana)
       if ((mob->points.hit>(hpcan+1))&&(mob->skills[SKILL_CANIBALIZE].learned))
	  {
           if ((cmana + 2*(mob->points.hit - hpcan)) >= mob->points.max_mana)
              sprintf(buf,"24"); /*Qmana=51>=cm, cm+(2*24) <= 99 */
            else sprintf(buf,"%d",(mob->points.hit - hpcan -1));
	   do_canibalize(mob,buf,1);
 	   return(TRUE);
	  }
	else if (mob->skills[SKILL_MEDITATE].learned)
    	        {
		 do_meditate(mob,mob->player.name,1);
 		 return(TRUE);
	        }
     if (NOTAFF_N_LEARNED(mob,SKILL_PSI_SHIELD,SKILL_PSI_SHIELD))
        do_psi_shield(mob,mob->player.name,1);
     else if (NOTAFF_N_LEARNED(mob,SKILL_MINDBLANK,SKILL_MINDBLANK))
	      mind_mindblank(PML,mob,mob,NULL);
     else if (NOTAFF_N_LEARNED(mob,SKILL_TOWER_IRON_WILL,SKILL_TOWER_IRON_WILL))
	      mind_tower_iron_will(PML,mob,mob,NULL);
     else if (NOTAFF_N_LEARNED(mob,SPELL_SENSE_LIFE,SKILL_GREAT_SIGHT))
	      do_great_sight(mob,mob->player.name,1);
     else if (NOTAFF_N_LEARNED(mob,SKILL_PSI_STRENGTH,SKILL_PSI_STRENGTH))
              mind_psi_strength(PML, mob, mob, NULL);
     else if (IS_SET(mob->hatefield, HATE_CHAR) && (mob->points.hit>hpcan))
              {
               do_say(mob,"It's payback time!",1);
               mob->points.mana = 100;
               if (PSI_CAN(SKILL_PORTAL,PML) || PSI_CAN(SKILL_SUMMON,PML))
                  return(Summoner(mob,0,NULL,mob,0));
              }
     else if (NOTAFF_N_LEARNED(mob,SPELL_INVISIBLE,SKILL_INVIS))
	      do_invisibililty(mob,mob->player.name,1);
     else if (mob->skills[SKILL_CHAMELEON].learned)
              mind_chameleon(PML,mob,mob,NULL);
     return(TRUE);
  } /* end peace time castings */
  else
   { /*they are fighting someone, do something nasty to them!*/
    mob->points.mana = 100; /*some psi combat spells still cost mana,
                              set to max mana start of every round of combat*/
    targ = mob->specials.fighting;
    if ((mob->points.max_hit-hpcan) > (1.5 * mob->points.hit) )
      {
      if (!mob->skills[SKILL_PSI_TELEPORT].learned || (!IsOnPmp(mob->in_room)))
         {
          act("$n looks around frantically.",0,mob,0,0,TO_ROOM);
          command_interpreter(mob,"flee");
          return(TRUE);
         }
      act("$n screams defiantly, 'I'll get you yet, $N!'",0,mob,0,targ,TO_ROOM);
      mind_teleport(PML,mob,mob,0);
      return(TRUE);
      }
    group = ((targ->followers || targ->master) ? TRUE : FALSE);
    log(buf);
    if (group && (dice(1,2)-1)) group = FALSE;
    if (!group)
       { /*not fighting a group, or has selected person fighting, for spec*/
        if ((dice(1,2)-1)) /* do special attack 50% of time */
          {
           if (IS_SET(targ->player.class,CLASS_MAGIC_USER|CLASS_CLERIC))
              if ((dice(1,2)-1))
                 CAST_OR_BLAST(mob,targ,SKILL_TELEKINESIS);
                else CAST_OR_BLAST(mob,targ,SKILL_MIND_WIPE);
		/*special attack for psi & sorc opponents
           else if (IS_SET(targ->player.class,CLASS_PSI|CLASS_SORCERER))
                     if (affected_by_spell(targ,SPELL_FEEBLEMIND))
                        CAST_OR_BLAST(mob,targ,SKILL_DISINTEGRATE);
                      else CAST_OR_BLAST(mob,targ,SKILL_MIND_WIPE);
            	/*special attack for fighter subtypes & thieves*/
	   else if ((GetMaxLevel(targ) < 20) && (dice(1,2)-1))
                   CAST_OR_BLAST(mob,targ,SKILL_PSI_TELEPORT);
                 else CAST_OR_BLAST(mob,targ,SKILL_DISINTERGRATE);
          }
        else C_OR_B(mob,targ); /* norm attack, psychic crush or psionic blast*/
       }
     else if (mob->skills[SKILL_ULTRA_BLAST].learned)
             mind_ultra_blast(PML,mob,targ,NULL);
           else if (mob->skills[SKILL_MIND_BURN].learned)
                   mind_burn(PML,mob,targ,NULL);
             else do_blast(mob,targ->player.name,1);
     return(TRUE);
   } /* end of fighting stuff */
 return(TRUE);
}
/*-------------------------end Psionist-------------------------*/
/* church bell for ators zone in town */
#define PULL	224
int ChurchBell(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	if (cmd==PULL)
 {
	char buf[128];
  arg=one_argument(arg,buf); /* buf == object */

  if(!strcmp("rope",buf)) {
     send_to_all("The bells of Karsynia's church sound 'GONG! GONG! GONG! GONG!'\n");
	return(TRUE);
    } /* end strcmpi() */
   return(FALSE);
 } /* end pull */

 return(FALSE);
}


/* end church bell */

#define SLAV_DEAD_DONE_ROOM	3494		/* where to tel people when he is killed */
#define SLAV_LIVE_ROOM		3496	/* where he SHOULD live */
int Slavalis(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

  if (cmd || !AWAKE(mob))
    return(FALSE);

if (type == EVENT_DEATH && ch->in_room == SLAV_LIVE_ROOM) {
  struct char_data *v;
	act("$n screams at you horridly 'I will return again!'",FALSE,ch,0,0,TO_ROOM);
		/* move all pc's to different room */
	    for (v = real_roomp(ch->in_room)->people;v;v = v->next_in_room) {
	     if (v != ch && !IS_IMMORTAL(v)) {
		act("You get a quezzy feeling as you fall into a swirling mist.\nYou arrive back on your home plane!",FALSE,v,0,ch,TO_CHAR);
	        act("$n fades away blinking in curiousity.",FALSE,v,0,ch,TO_ROOM);
		char_from_room(v);
		char_to_room(v,SLAV_DEAD_DONE_ROOM);
		do_look(v,"\0",0);
	      }
	    } /* end for */


	/* ding bell... */

	send_to_all("The chruch bells of Karsynia sound off in a chorus of happiness!\n");

		return(TRUE);
	} /* end event dead! */


 if (ch->specials.fighting)
 {
      if ((GET_POS(ch) < POSITION_FIGHTING) &&
          (GET_POS(ch) > POSITION_STUNNED))
        StandUp(ch);
 switch(number(1,2)) {
	case 1: return(Demon(ch, cmd, arg, mob, type));
		break;
	case 2:return(Psionist(ch, cmd, arg, mob, type));
		break;
	default: return(Demon(ch, cmd, arg, mob, type));
		break;
 } /* end switch */

 } /* fighting */   else
   {
    return(magic_user(ch, cmd, arg, mob, type));
   } /* else not fighting */

}

/* berserker sword */
#define BERSERK 329
int BerserkerItem(struct char_data *ch, int cmd, char *arg, struct obj_data *obj, int type)
{

if (type != PULSE_COMMAND)
	    return(FALSE);

 if (ch->specials.fighting) {	/* fighting! berserk! */

 if (!ch->equipment[WIELD])  /* nothing wielded */
    return(FALSE);

if (ch->equipment[WIELD]!=obj)    	/* we are not wielded */
	return(FALSE);

  if (!IS_SET(ch->specials.affected_by2,AFF2_BERSERK)) {
log("trying to berserk because of item ");
  	/* not berserked go berserk! */
	do_berserk(ch,"",0);
	return(FALSE);
   } else {
      		/* already berserked */
    if (number(1,10)==1) {
	 act("$n seems enraged to the point of exploding!",FALSE,ch,0,0,TO_ROOM);
	 act("Your anger whells up inside, you fling yourself at your opponet!",FALSE,ch,0,0,TO_CHAR);
	}
	return(FALSE);

    }
  } else {
	/* was not fighting */

  return(FALSE);
 }

 return(FALSE);
}

/* end berserker sword */

int AntiSunItem(struct char_data *ch, int cmd, char *arg, struct obj_data *obj, int type)
{

 if (type != PULSE_COMMAND)
  return(FALSE);

  if (OUTSIDE(ch) && weather_info.sunlight == SUN_LIGHT
     && weather_info.sky<= SKY_CLOUDY &&
     !IS_AFFECTED2(ch,AFF2_DARKNESS)) {
  			/* frag the item! */
	act("The sun strikes $p, causing it to fall apart!",FALSE,ch,obj,0,TO_CHAR);
	act("The sun strikes $p worn by $n, causing it to fall apart!",FALSE,ch,obj,0,TO_ROOM);
	    MakeScrap(ch,0,obj);
	    return(TRUE); /* if not TRUE mud will CRASH! */
	  }

 return(FALSE);
}

int Beholder(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
int action=0;

if (cmd || !AWAKE(mob))
   return(FALSE);

 if (mob->specials.fighting) {

   if (GET_POS(mob) == POSITION_SITTING) {
       do_stand(mob,"",0);
       return(TRUE);
      }


  action=number(1,100);

  if (action>=90) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_fear(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
  if (action>=80) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_slow(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
  if (action>=75) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_disintegrate(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
  if (action>=50) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_dispel_magic(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
  if (action>=30) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_charm_monster(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
  if (action>=10) {
     act("$n turns one of $m eye stalks at $N!",FALSE,mob,0,mob->specials.fighting,TO_NOTVICT);
     act("$n turns a evil looking eye stalk at you!",FALSE,mob,0,mob->specials.fighting,TO_VICT);
     cast_harm(GetMaxLevel(mob),mob,"",SPELL_TYPE_SPELL,mob->specials.fighting,0);
	return(TRUE);
    } else
      return(magic_user(ch,cmd,arg,mob,type));
  } /* end he was fighting */

  return(magic_user(ch,cmd,arg,mob,type));
}

/* Generic Archer */

int range_estimate(struct char_data *ch, struct obj_data *o, int type)
{
   int r, sz, w = o->obj_flags.weight;

   /* Type 0 = fireweapon 1 = thrown */
   if (w>100) sz = 3;
   else if (w>25) sz = 2;
   else if (w>5) sz = 1;
   else sz = 0;
   r = (((GET_STR(ch)+GET_ADD(ch)/30)-3)/8)+2;
   r /= sz+1;
   if (!type) {
      r += ch->equipment[WIELD]->obj_flags.value[2];
   }
   r = r*2/3;
   return r;
}

int pick_archer_target(struct char_data *ch, int maxr, struct char_data **targ,    int *rn, int *dr)
{
   int i, r, rm, a, b;
   struct char_data *ptarg;

   if (maxr>5) maxr=5;
   for (i=0;i<6;i++) {
      rm = ch->in_room;
      r = 0;
      while (r<=maxr) {
         if (clearpath(ch,rm,i)) {
            r++;
            rm = real_roomp(rm)->dir_option[i]->to_room;
            for (ptarg=real_roomp(rm)->people;ptarg;ptarg=ptarg->next_in_room) {
/* find PC's to kill if we are aggressive */
   if (IS_PC(ptarg) && CAN_SEE(ch,ptarg) &&
          (IS_SET(ch->specials.act,ACT_AGGRESSIVE) || IS_SET(ch->specials.act,ACT_META_AGG)) ) {
                  *targ = ptarg;
                  *rn = r;
                  *dr = i;
                  return(TRUE);
               } else
/* find people we are hunting */
          if (CAN_SEE(ch,ptarg) && IS_SET(ch->specials.act,ACT_HUNTING) &&
             ((ch->specials.hunting) && ch->specials.hunting==ptarg ||
              (ch->specials.charging) && ch->specials.charging == ptarg) ) {
                  *targ = ptarg; *rn = r;  *dr = i;
                  return(TRUE);
		}  else
/* find hated peoples */
	if (CAN_SEE(ch,ptarg) && Hates(ch,ptarg)) {
                  *targ = ptarg; *rn = r;  *dr = i;
                  return(TRUE);
	   }
            }
         } else {
            r = maxr+1;
         }
      }
   }
   return FALSE;
}

/* added - changed */
int archer_sub(struct char_data *ch)
{
   struct obj_data *bow = NULL, *missile = NULL, *thrown = NULL,
                   *spid = NULL, *obj_object = NULL, *next_obj = NULL;
   int r, a, b, found;
   char target[MAX_STRING_LENGTH], buf[MAX_STRING_LENGTH];
   struct char_data *td;

   if ((ch->equipment[WIELD]) &&
       (ch->equipment[WIELD]->obj_flags.type_flag==ITEM_FIREWEAPON)) {
      bow = ch->equipment[WIELD];
      if (ch->equipment[LOADED_WEAPON]) {
            missile = ch->equipment[LOADED_WEAPON];
      } else {
         /* Search inventory for a missile */
         for (spid=ch->carrying;spid;spid=spid->next_content) {
            if ((spid->obj_flags.type_flag==ITEM_MISSILE) &&
                (spid->obj_flags.value[3]==bow->obj_flags.value[2])) {
               missile = spid;
            } else {
              /* see if they are carrying a quiver full of arrows, if so get an arrow */
               if (GET_ITEM_TYPE(spid)==ITEM_CONTAINER){
                 found=FALSE;
                 for(obj_object = spid->contains;
                     obj_object && !found;
                     obj_object = next_obj) {
                    next_obj = obj_object->next_content;
                    if ((obj_object->obj_flags.type_flag==ITEM_MISSILE) &&
                        (obj_object->obj_flags.value[3]==bow->obj_flags.value[2])) {
                      /* gets arrow out of quiver, next round they will load it */
                      get(ch,obj_object,spid);
                      found=TRUE;
                    }
                 }
               }
            }
         }
         /* If you found a missile, load it and return */
         if (missile) {
            do_weapon_load(ch,missile->name,0);
            return TRUE;
         }
      }
      if (missile) {
         /* Pick a target and fire */
         r = range_estimate(ch, missile, 0);
         if (pick_archer_target(ch, r, &td, &a, &b)) {
            strcpy(target, td->player.name);
            do_fire(ch, target,0);
            return TRUE;
         } else {
            return FALSE;
         }
      }
   }
   /* Try and find a missile weapon and wield it */
   if (!bow) {
      for (spid=ch->carrying;spid;spid=spid->next_content) {
         if (spid->obj_flags.type_flag==ITEM_FIREWEAPON) {
            bow = spid;
         }
               if (GET_ITEM_TYPE(spid)==ITEM_CONTAINER){
                 found=FALSE;
                 for(obj_object = spid->contains;
                     obj_object && !found;
                     obj_object = next_obj) {
                    next_obj = obj_object->next_content;
                    if (obj_object->obj_flags.type_flag==ITEM_FIREWEAPON) {
                      /* gets bow out of container */
                      get(ch,obj_object,spid);
                      found=TRUE;
                      bow=obj_object;
                    }
                 }
               }
      }
      if (bow) {
         if (ch->equipment[WIELD]) {
            do_remove(ch,ch->equipment[WIELD]->name,0);
            return TRUE;
         } else {
            do_wield(ch,bow->name,0);
            return TRUE;
         }
      }
   }
   /* No missile weapon or no ammo.  Try a thrown weapon */
   for (spid=ch->carrying;spid;spid=spid->next_content) {
      if (IS_SET(spid->obj_flags.wear_flags,ITEM_THROW)) {
         thrown = spid;
      }
   }
   if (!thrown) {
      /* Just can't do nothing. */
      return FALSE;
   }
   /* Got a throw weapon, throw it. */
   r = range_estimate(ch, thrown, 1);
   if (pick_archer_target(ch, r, &td, &a, &b)) {
      sprintf(buf,"%s %s",thrown->name,td->player.name);
      do_throw(ch, buf, 0);
      return TRUE;
   } else {
      return FALSE;
   }
}

int archer_hth(struct char_data *ch)
{
   struct obj_data *spid, *hth=NULL;

   /* What to do if you are an archer and find yourself in HTH combat */
   /* I. If you are wielding a bow ditch it */
   if ((ch->equipment[WIELD])&&(ch->equipment[WIELD]->obj_flags.type_flag==ITEM_FIREWEAPON)) {
      do_remove(ch,ch->equipment[WIELD]->name,66);
      return TRUE;
   } else {
      if (ch->equipment[WIELD]) return FALSE;
      for (spid=ch->carrying;spid;spid=spid->next_content) {
         if (spid->obj_flags.type_flag==ITEM_WEAPON) hth = spid;
      }
      if (hth) {
         do_wield(ch,hth->name,14);
         return TRUE;
      }
      return FALSE;
   }
}

int archer(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

   if (cmd || !AWAKE(ch)) {
      return(FALSE);
     }

  if (GET_POS(ch)==POSITION_FIGHTING) {
     return(archer_hth(ch));
   } else {
     return(archer_sub(ch));
   }

  return(archer_sub(ch)); /* should never get here */
}

int fighter_mage(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 if (number(1,100) >49)
    return(fighter(ch,cmd,arg,mob,type));else
    return(magic_user(ch,cmd,arg,mob,type));
}

int fighter_cleric(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 if (number(1,100) >49)
    return(fighter(ch,cmd,arg,mob,type));else
    return(cleric(ch,cmd,arg,mob,type));
}

int cleric_mage(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 if (number(1,100) >49)
    return(cleric(ch,cmd,arg,mob,type));else
    return(magic_user(ch,cmd,arg,mob,type));
}

int Ranger(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 return(fighter(ch,cmd,arg,mob,type));
}

int Barbarian(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
 return(fighter(ch,cmd,arg,mob,type));
}

/*
 *  From: sund_procs.c                          Part of Exile MUD
 *
 *  Special procedures for the mobs and objects of Sundhaven.
 *
 *  Exile MUD is based on CircleMUD, Copyright (C) 1993, 1994.
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.
 *
 */


/*  Mercy's Procs for the Town of Sundhaven  */

int sund_earl(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
  if (cmd)
    return(FALSE);
  switch (number(0, 20)) {
   case 0:
        do_say(ch, "Lovely weather today.", 0);
        return(TRUE);
   case 1:
                act("$n practices a lunge with an imaginary foe.", FALSE, ch, 0, 0,TO_ROOM);
        return(TRUE);
   case 2:
        do_say(ch, "Hot performance at the gallows tonight.", 0);
        act("$n winks suggestively.", FALSE, ch, 0, 0,TO_ROOM);
        return(TRUE);
   case 3:
        do_say(ch, "Must remember to up the taxes at my convenience.", 0);
        return(TRUE);
   case 4:
        do_say(ch, "Sundhaven is impermeable to the enemy!", 0);
        act("$n growls menacingly.", FALSE, ch, 0, 0,TO_ROOM);
        return(TRUE);
   case 5:
        do_say(ch, "Decadence is the credence of the abominable.", 0);
        return(TRUE);
   case 6:
            do_say(ch, "I look at you and get a wonderful sense of impending doom.", 0);
            act("$n chortles merrily.", FALSE, ch, 0, 0,TO_ROOM);
            return(TRUE);
   case 7:
        act("$n touches his goatee ponderously.", FALSE, ch, 0, 0,TO_ROOM);
        return(TRUE);
   case 8:
        do_say(ch, "It's Mexican Madness night at Maynards!", 0);
        act("$n bounces around.", FALSE, ch, 0, 0, TO_ROOM);
        return(TRUE);
   default:
                return(FALSE);
        break;
    return(FALSE);
 }

}

#if 0
int silktrader(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
        struct room_data *rp;

        if (cmd)
            return 0;

        rp = real_roomp(ch->in_room);

        if (rp->sector_type == SECT_CITY)
                switch (number(0, 30))
                {
                 case 0:
                act("$n eyes a passing woman.", FALSE, ch, 0, 0,TO_ROOM);
                do_say(ch, "Come, m'lady, and have a look at this precious silk!", 0);
                return(TRUE);
                 case 1:
                    act("$n says to you, 'Wouldn't you look lovely in this!'", FALSE, ch, 0, 0,TO_ROOM);
                act("$n shows you a gown of indigo silk.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 2:
                    act("$n holds a pair of silk gloves up for you to inspect.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                        case 3:
                act("$n cries out, 'Have at this fine silk from exotic corners of the world you will likely never see!", FALSE, ch, 0, 0,TO_ROOM);
                act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 4:
                do_say(ch, "Step forward, my pretty locals!", 0);
                return(TRUE);
             case 5:
                act("$n shades his eyes with his hand.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 6:
                do_say(ch, "Have you ever seen an ogre in a silken gown?", 0);
                do_say(ch, "I didn't *think* so!", 0);
                act("$n throws his head back and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 7:
                act("$n hands you a glass of wine.", FALSE, ch, 0, 0,TO_ROOM);
                do_say(ch, "Come, have a seat and view my wares.", 0);
                return(TRUE);
                 case 8:
                act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
                act("$n shakes his head sadly.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 9:
                act("$n fiddles with some maps.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                 case 10:
                do_say(ch, "Here here! Beggars and nobles alike come forward and make your bids!", 0);
                return(TRUE);
                 case 11:
                do_say(ch, "I am in this bourgeois hamlet for a limited time only!", 0);
                act("$n swirls some wine in a glass.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
  }

  if (rp->sector_type != SECT_CITY)
        switch (number(0, 20))
        {
                case 0:
                do_say(ch, "Ah! Fellow travellers! Come have a look at the finest silk this side of the infamous Ched Razimtheth!", 0);
                return(TRUE);
                case 1:
                    act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
                do_say(ch, "You are feebly attired for the danger that lies ahead.", 0);
                do_say(ch, "Silk is the way to go.", 0);
                act("$n smiles warmly.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                case 2:
                do_say(ch, "Worthy adventurers, hear my call!", 0);
                return(TRUE);
                case 3:
                act("$n adjusts his cloak.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                case 4:
                act("$n says to you, 'Certain doom awaits you, therefore shall you die in silk.'", FALSE, ch, 0, 0,TO_ROOM);
                act("$n bows respectfully.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                case 5:
                do_say(ch, "Can you direct me to the nearest tavern?", 0);
                return(TRUE);
                case 6:
                do_say(ch, "Heard the latest ogre joke?", 0);
                act("$n snickers to himself.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                case 7:
                do_say(ch, "What ho, traveller! Rest your legs here for a spell and peruse the latest in fashion!", 0);
                return(TRUE);
                case 8:
                do_say(ch, "Beware ye, traveller, lest ye come to live in Exile!", 0);
                act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
                case 9:
                act("$n touches your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
                do_say(ch, "A word of advice. Beware of any ale labled 'mushroom' or 'pumpkin'.", 0);
                act("$n shivers uncomfortably.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);

  }
  return(FALSE);

}


int athos(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
  if(cmd)
   return 0;
    switch (number(0, 20)) {
    case 0:
      act("$n gazes into his wine gloomily.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
    case 1:
      act("$n grimaces.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
    case 2:
      act("$n asks you, 'Have you seen the lady, pale and fair, with a heart of stone?'", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "That monster will be the death of us all.", 0);
      return(TRUE);
    case 3:
      do_say(ch, "God save the King!", 0);
      return(TRUE);
    case 4:
      do_say(ch, "All for one and .. one for...", 0);
      act("$n drowns himself in a swig of wine.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
    case 5:
      act("$n looks up with a philosophical air.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "Women - God's eternal punishment on man.", 0);
      return(TRUE);
    case 6:
      act("$n downs his glass and leans heavily on the oaken table.", FALSE, ch, 0, 0,TO_ROOM);
      do_say(ch, "You know, we would best band together and wrestle the monstrous woman from her lair and home!", 0);
      return(TRUE);
        default:
                return(FALSE);
                break;
                                }
    return(FALSE);

}



int hangman(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
        if(cmd) return 0;
                switch (number(0, 15)) {
        case 0:
            act("$n whirls his noose like a lasso and it lands neatly around your neck.", FALSE, ch, 0, 0,TO_ROOM);
        do_say(ch, "You're next, you ugly rogue!", 0);
            do_say(ch, "Just kidding.", 0);
            act("$n pats you on your head.", FALSE, ch, 0, 0,TO_ROOM);
            return(TRUE);
    case 1:
            do_say(ch, "I was conceived in Exile and have been integrated into society!", 0);
        do_say(ch, "Muahaha!", 0);
            return(TRUE);
        case 2:
            do_say(ch, "Anyone have a butterknife I can borrow?", 0);
            return(TRUE);
        case 3:
            act("$n suddenly pulls a lever.", FALSE, ch, 0, 0,TO_ROOM);
            act("With the flash of light on metal a giant guillotine comes crashing down!", FALSE, ch, 0, 0,TO_ROOM);
        act("A head drops to the ground from the platform.", FALSE, ch, 0, 0,TO_ROOM);
        act("$n looks up and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
            act("$n shouts, 'Next!'", FALSE, ch, 0, 0, TO_ROOM);
            return(TRUE);
    case 4:
                act("$n whistles a local tune.", FALSE, ch, 0, 0,TO_ROOM);
                return(TRUE);
        default:
        return(FALSE);
   break;
  }
  return(FALSE);

}



int butcher(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
if(cmd) return 0;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "I need a Union.", 0);
      act("$n glares angrily.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n rummages about for an axe.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 1:
      act("$n gnaws on a toothpick.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 2:
      act("$n runs a finger along the edge of a giant meat cleaver.", FALSE, ch,
0, 0,TO_ROOM);
      act("$n grins evilly.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 3:
      do_say(ch, "Pork for sale!", 0);
      return(TRUE);
   case 4:
      act("$n whispers to you, 'I've got some great damage eq in the back room. Wanna see?'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n throws back his head and cackles with insane glee!", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 5:
      act("$n yawns.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 6:
      act("$n throws an arm around the headless body of an ogre and asks to have his picture taken.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 7:
      act("$n listlessly grabs a cleaver and hurls it into the wall behind your head.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 8:
      act("$n juggles some fingers.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 9:
      act("$n eyes your limbs.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n chuckles.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 10:
      do_say(ch, "Hi, Alice.", 0);
      return(TRUE);
   case 11:
      do_say(ch, "Everyone looks like food to me these days.", 0);
      act("$n sighs loudly.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 12:
      act("$n throws up his head and shouts wildly.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n shouts, 'Bring out your dead!'", FALSE, ch, 0, 0, TO_ROOM);
      return(TRUE);
   case 13:
      do_say(ch, "The worms crawl in, the worms crawl out..", 0);
      return(TRUE);
   case 14:
      act("$n sings 'Brave, brave Sir Patton...'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles a tune.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n smirks.", FALSE, ch, 0, 0,TO_ROOM);
      return(TRUE);
   case 15:
      do_say(ch, "Get Lurch to bring me over a case and I'll sport you a year's supply of grilled ogre.", 0);
      return(TRUE);
   default:
          return(FALSE);

        break; }
    return(FALSE);

}



int stu(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int
type)

{
  if(cmd)
    return 0;

  switch (number(0, 60)) {
    case 0:
      do_say(ch, "I'm so damn cool, I'm too cool to hang out with myself!", 0);
      break;
    case 1:
      do_say(ch, "I'm really the NICEST guy you ever MEET!", 0);
      break;
    case 2:
      do_say(ch, "Follow me for exp, gold and lessons in ADVANCED C!", 0);
      break;
    case 3:
      do_say(ch, "Mind if I upload 200 megs of pregnant XXX gifs with no descriptions to your bbs?", 0);
      break;
    case 4:
      do_say(ch, "Sex? No way! I'd rather jog 20 miles!", 0);
      break;
    case 5:
      do_say(ch, "I'll take you OUT!!   ...tomorrow", 0);
      break;
    case 6:
      do_say(ch, "I invented Mud you know...", 0);
      break;
    case 7:
      do_say(ch, "Can I have a cup of water?", 0);
      break;
    case 8:
      do_say(ch, "I'll be jogging down ventnor ave in 10 minutes if you want some!", 0);
      break;
    case 9:
      do_say(ch, "Just let me pull a few strings and I'll get ya a site, they love me! - doesnt everyone?", 0);
      break;
    case 10:
      do_say(ch, "Pssst! Someone tell Manwe to sport me some levels.", 0);
      act("$n nudges you with his elbow.", FALSE, ch, 0, 0,TO_ROOM);
      break;
    case 11:
      do_say(ch, "Edgar! Buddy! Let's group and hack some ogres to tiny quivering bits!", 0);
      break;
    case 12:
      act("$n tells you, 'Xenon has bad taste in women!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n screams in terror!", FALSE, ch, 0, 0,TO_ROOM);
      do_flee(ch, "", 0);
      break;
    case 13:
      if (number(0, 32767)<10){
      act("$n whispers to you, 'Dude! If you say 'argle bargle' to the glowing fido he'll raise you a level!'", FALSE, ch, 0, 0,TO_ROOM);
      act("$n flexes.", FALSE, ch, 0, 0,TO_ROOM);}
      return(TRUE);
    default:
      return(FALSE);
      break;
   return(TRUE);
  }
  return 0;

}




int blinder(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
  if (cmd)
    return FALSE;

  if (GET_POS(ch) != POSITION_FIGHTING)
    return FALSE;

  if (ch->specials.fighting && (ch->specials.fighting->in_room == ch->in_room) && (number(0, 100)+GET_LEVEL(ch, BestClassIND(ch)) >= 50))
  {
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n whispers, 'So, $N! You wouldst share my affliction!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    act("$n's frayed cloak blows as he points at $N.", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
    act("$n's frayed cloak blows as he aims a bony finger at you.", 1, ch, 0, ch->specials.fighting, TO_VICT);
    act("A flash of pale fire explodes in $N's face!", 1, ch, 0, ch->specials.fighting, TO_NOTVICT);
        act("A flash of pale fire explodes in your face!", 1, ch, 0, ch->specials.fighting, TO_VICT);
    cast_blindness(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, ch->specials.fighting, 0);

        return TRUE;
  }
  return FALSE;

}


int idiot(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
if(cmd) return FALSE;
  switch (number(0, 40)) {
   case 0:
      do_say(ch, "even if idiot = god", 0);
      do_say(ch, "and Stu = idiot", 0);
      do_say(ch, "Stu could still not = god.", 0);
      act("$n smiles.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 1:
      act("$n balances a newbie sword on his head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 2:
      act("$n doesn't think you could stand up to him in a duel.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 3:
      do_say(ch, "Rome really was built in a day.", 0);
      act("$n snickers.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 4:
      act("$n flips over and walks around on his hands.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 5:
      act("$n cartwheels around the room.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 6:
      do_say(ch, "How many ogres does it take to screw in a light bulb?", 0);
      act("$n stops and whaps himself upside the head.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 7:
      do_say(ch, "Uh huh. Uh huh huh.", 0);
      return TRUE;
   case 8:
      act("$n looks at you.", FALSE, ch, 0, 0,TO_ROOM);
      act("$n whistles quietly.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 9:
      act("$n taps out a tune on your forehead.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 10:
      act("$n has a battle of wits with himself and comes out unharmed.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 11:
      do_say(ch, "All this and I am just a number.", 0);
      act("$n cries on your shoulder.", FALSE, ch, 0, 0,TO_ROOM);
      return TRUE;
   case 12:
      do_say(ch, "A certain hunchback I know dresses very similar to you, very similar...", 0);
      return TRUE;
   default:
      return FALSE;
  }
 return FALSE;

}


int marbles(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)

{
  struct obj_data *tobj;

  if (tobj->in_room == NOWHERE)
    return 0;

  if ((cmd >= 1) && (cmd <= 6) &&(GET_POS(ch) == POSITION_STANDING) &&(!IS_NPC(ch)))
  {
    if (number(1, 100) + GET_DEX(ch) > 50)
        {
      act("You slip on $p and fall.", FALSE, ch, tobj, 0, TO_CHAR);
      act("$n slips on $p and falls.", FALSE, ch, tobj, 0, TO_ROOM);
      GET_POS(ch) = POSITION_SITTING;
      return 1;
    }
    else
        {
      act("You slip on $p, but manage to retain your balance.", FALSE, ch, tobj, 0, TO_CHAR);
      act("$n slips on $p, but manages to retain $s balance.", FALSE, ch, tobj, 0, TO_ROOM);
    }
  }
  return 0;

}

#endif



/* (GH)2001STill under Construction...*/
int QPSalesman(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type) {
  struct obj_data *obj;
  char buf[256];
  int x,temp=0;

  int questitems[13][3] = {
    {871,CLASS_CLERIC,     6},
    {872,CLASS_MAGIC_USER, 6},
    {873,CLASS_DRUID,      6},
    {874,CLASS_WARRIOR,    6},
    {875,CLASS_RANGER,     6},
	{876,CLASS_CLERIC,     6},
    {877,CLASS_MAGIC_USER, 6},
    {878,CLASS_DRUID,      6},
    {879,CLASS_WARRIOR,    6},
    {880,CLASS_RANGER,     6},
    {881,CLASS_BARBARIAN,  7},
	{882,CLASS_MAGIC_USER, 6},
	{883,CLASS_DRUID,      6}

  };
  //*pc_class_types[]
  if (cmd==59) { //list
    send_to_char("Quest Point Items:\n\r", ch);

    sprintf(buf,"$c0011%-25s %-15s %-10s\n\r","Name","Class","QPs");
    send_to_char(buf,ch);

    sprintf(buf,"%-25s %-15s %-10s\n\r","----","-----","---");
    send_to_char(buf,ch);

    for(x = 0;x < 13;x ++) {

      if(HasClass(ch,questitems[x][1])) {
	temp = questitems[x][0];
	obj = read_object(temp, VIRTUAL);

	if(obj) {
	  sprintf(buf,"$c0012%-25s $c0014%-16s $c0015%-10d$c0007\n\r"
		  ,obj->short_description
		  ,pc_class_types[pc_class_num(questitems[x][1])]
		  ,questitems[x][2]);
	  send_to_char(buf,ch);
	}
      }

    }
    return(TRUE);
  } else

    if(cmd == 56) {


      temp = 13;//ch->player.q_points;
      //lets search for item..
      for (x = 0;x < 13;x++) {
	temp = questitems[x][0];
	obj = read_object(temp, VIRTUAL);
	if (obj) {

     /*crashes Here (GH)*/
	if(isname(arg,obj->name)) {
	  temp = questitems[x][2];
	  if (temp <= ch->player.q_points) {
	    ch->player.q_points = ch->player.q_points - temp;
	    act("$c0013[$c0015The QuestPoint Salesman$c0013] tells you"
		" 'I hope you enjoy.'",FALSE,ch,0,0,TO_CHAR);
	    sprintf(buf,"The QPSalesman gives you %s\n\r",obj->short_description);
	    send_to_char(buf,ch);
	    obj_to_char(obj,ch);
	  	sprintf(buf,"$s just bought %s\n\r",GET_NAME(ch), obj->short_description);
	  	qlog(buf);
	  } else
	    act("$c0013[$c0015The QuestPoint Salesman$c0013] tells you"
		" 'You don't have enought QPoints for that item'"
		,FALSE,ch,0,0,TO_CHAR);

	  return TRUE;
	}
}
   }
      act("$c0013[$c0015The QuestPoint Salesman$c0013] tells you"
	  " 'I don't have that item.'",FALSE,ch,0,0,TO_CHAR);
      return TRUE;
    }
  return FALSE;
}
/* This procs allows a mob to dispel and incernary cloud someone after they miss a bash
 * By Greg Hovey
 * Feb 25, 2001
 */

int DispellerIncMob(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *targ;
  struct room_data *rp;
  int i;
  char buf[256];

  if (cmd && cmd != 156) return(FALSE);

  if (cmd == 156) {
    send_to_char("You're much too afraid to steal anything!\n\r", ch);
    return(TRUE);
  }

  if (AWAKE(ch)) {
    if ((targ = FindAnAttacker(ch))!='\0') {
      act("$n slowly raises $s hands.", TRUE, ch, 0, 0, TO_ROOM);

      if (!saves_spell(targ, SAVING_PARA)) {
	act("Suddently $n utters some strange words!",
	    TRUE, ch, 0, targ, TO_ROOM);
	cast_dispel_magic(60, ch, "", SPELL_TYPE_SPELL, targ,0);
	cast_incendiary_cloud(60, ch, "", SPELL_TYPE_SPELL, targ,0);
      }
    }
  }
}

/* Ash's Corsair ship proc.
 * By Greg Hovey
 * May 5, 2001
 */
#if 1

int CorsairPush(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *targ, *target;
  struct room_data *rp;
  int i, location;
  char buf[256];

  if (cmd && cmd != 156) return(FALSE);


  if (AWAKE(ch)) {
    if ((targ = FindAnAttacker(ch))!='\0') {
      act("$n pushes you off the ship.", TRUE, ch, 0, 0, TO_ROOM);

		//char_from_room(mob);
		if ( !(target=get_char_vis_world(ch,"ship",NULL)) )   {
		  send_to_char ("Where did that darn ship go??.\n\r",ch);
		  return;
	    }

	    location = target->in_room;

		//char_from_room(mob);
	//	char_to_room(mob,location);  /*Find the room that the corsair ship is in*/

	//	char_from_room(ch);
		//char_to_room(ch,location);  /*Find the room that the corsair ship is in*/
		char_from_room(targ);
		char_to_room(targ,location);  /*Find the room that the corsair ship is in*/
		act("$n suddently falls from above.", TRUE, targ, 0, 0, TO_ROOM);
    	act("You fall helplessly downward off the ship.", FALSE, ch, 0, 0, TO_CHAR);
    }
  }
}
#endif

/*Thunder Mountain Keep*/

#define NADIA_PILL 45431
#define PEN_MIGHT 45445
#define DRAGON_SCEPTRE_ONE 45470
#define DRAGON_SCEPTRE_TWO 45481
#define MARBLES 45475
#define JESTER_KEY 45480
#define NADIA_KEY 45489
#define EYE_DRAGON 45488
#define GATEKEEPER_KEY 45491
#define BLACK_PILL 45492
#define BLUE_PILL 45493
#define GRAYSWANDIR 45494
#define BAHAMUT_SKIN 45495
#define WEST_WING_KEY 45500

#define BAHAMUT 45400
#define TMK_GUARD_ONE 45401
#define TMK_GUARD_TWO 45402
#define BRAXIS 45406
#define NADIA 45409
#define MIME_JERRY 45410
#define ZORK 45413
#define ELAMIN 45417
#define STARVING_MAN 45440
#define GOBLIN_CHUIRGEON 45443

#define ZORK_ROOM 45496
#define CALM_BEFORE_STORM 45517

int Thunder_Fountain(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

  char buf[MAX_INPUT_LENGTH];

  extern int drink_aff[][3];

  extern struct weather_data weather_info;
        void name_to_drinkcon(struct obj_data *obj,int type);
        void name_from_drinkcon(struct obj_data *obj);


  if (cmd==11) { /* drink */

    only_argument(arg,buf);

    /*if (str_cmp(buf, "fountain") && str_cmp(buf, "water")) {
      return(FALSE);
    }*/

    send_to_char("You drink from the fountain\n\r", ch);
    act("$n drinks from the fountain", FALSE, ch, 0, 0, TO_ROOM);


    if(GET_COND(ch,THIRST)>20) {
           act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);
	   return(TRUE);
	}
    if(GET_COND(ch,FULL)>20) {
           act("You are full.",FALSE,ch,0,0,TO_CHAR);
	   return(TRUE);
	}

    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL)+=1;

    switch (number(0, 5)) {

	/*If you are lucky, you will find the secret*/
	case 1:
		send_to_char("You see something glistening in the water.\n\r",ch);
		send_to_char("As you reach to touch it, some magical force pulls you under.\n\r",ch);
		act("$n is suddenly sucked into the fountain!",FALSE, ch, 0, 0, TO_ROOM);
		char_from_room(ch);
		char_to_room(ch,46201);
		do_look(ch, "", 0);
	break;
	/*And if you aren't, you won't*/
	default:
		send_to_char("The water tastes so good, you are tempted to try some more.\n\r", ch);
	break;
	}
	  return(TRUE);
  }

  /* All commands except fill and drink */
  return(FALSE);
}

/* Braxis the Swamp Dragon */
int braxis_swamp_dragon(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	char tbuf[80];
	struct char_data *vict;
	struct obj_data *obj;
	int test=0, r_num=0;
	int has_marbles = 0;
        int marblesrnum = 0;
	struct obj_data *i;

	if(!AWAKE(ch)) return(FALSE);

	/*TALK TO ME!!!*/

	if (cmd == 531)	/*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);
	      if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}
	      if (mob_index[vict->nr].virtual != BRAXIS) return(FALSE);

		marblesrnum = real_object(MARBLES);
		for (i = vict->carrying; i; i = i->next_content) {
			if (has_marbles != 1) {
				if (i->item_number == marblesrnum) has_marbles = 1;
				else has_marbles = 0;
	                }
		}

		/*Quest Text*/
		if (has_marbles == 0)
		{
			act("$n says, 'Kind warriors, I seem to have misplaced my marbles.'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'How I so long to play with them again. If you can'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'return my marbles, I will tell you of a great land'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'of riches and powerful items.'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n shivers in anticipation.", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'Ignatius will fall to my master marble skills if I'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'can ever find them.'", FALSE, vict, 0, 0, TO_ROOM);
			return(TRUE);
		}

		else
		{
			act("$n says, 'Please leave me alone while I practice my game.'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'Ignatius will fall to my wrath.'", FALSE, vict, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}

	else if (cmd == 72)
	{
		arg=one_argument(arg, obj_name);
		if ((!*obj_name) || (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))) return(FALSE);

		arg=one_argument(arg, vict_name);
		if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))) || (IS_PC(vict))) return(FALSE);

		if (vict->specials.fighting)
		{
			send_to_char("Not while they are fighting!\n\r", ch);
			return(TRUE);
		}

		/*If mob is not Braxis*/
		if (mob_index[vict->nr].virtual != BRAXIS) return(FALSE);

		/*If object is not Marbles*/
        	if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
      		if ((obj_index[obj->item_number].virtual != MARBLES))
			{
      		sprintf(buf, "%s That is not the item I seek.",GET_NAME(ch));
    			do_tell(vict,buf,19);
         		return(TRUE);
			}
		}
   		else
		{
			sprintf(buf,"%s %s",obj_name,vict_name);
      		do_give(ch,buf,0);
			if(obj_index[obj->item_number].virtual == MARBLES) test=1;
			else return(TRUE);
		}
 		if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
			test=1;
	  		do_give(ch,"bag-marbles braxis-swamp-dragon",0);
 		}
	}

	else return(FALSE);

	if (test==1) /* It is the Braxis, and the marbles */
	{
		act("$n says, 'There is a mighty power in the sceptre that rests in'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'the Great King's throne room. However, this power has'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'been split into three separate pieces that must be united.'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'You must reunite the head of the sceptre with its severed'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'eyes.  Once you do so you will be transported to a wondrous'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'world of monsters and danger.  You must be brave as well as'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'strong to survive, but if you do, the rewards will be great.'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n hands over a couple of items.", FALSE, vict, 0, 0, TO_ROOM);
		if ((r_num = real_object(EYE_DRAGON)) >= 0) {
			obj = read_object(r_num, REAL);
			obj_to_char(obj, ch);
		}
		if ((r_num = real_object(GATEKEEPER_KEY)) >= 0) {
			obj = read_object(r_num, REAL);
			obj_to_char(obj, ch);
		}
		return(TRUE);
	}

	return(FALSE);

}

int mime_jerry(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *i, *tmp_ch;
  struct obj_data *target_obj;
  char buf[80];
  int inroom,r_num=FALSE;
  static int mimenum=0;

  void do_emote(struct char_data *ch, char *argument, int cmd);

  if(IS_PC(ch))
    return(0);

  inroom = ch->in_room;

	if(type == EVENT_DEATH) {
			sprintf(buf,"nadia thunder mountain", 0);
			if (tmp_ch = get_char_vis_world(ch, buf, 0)) {
			if ((r_num = real_object(NADIA_KEY)) >= 0) {
				target_obj = read_object(r_num, REAL);
				obj_to_char(target_obj, tmp_ch);
			}
			if ((r_num = real_object(NADIA_PILL)) >= 0) {
				target_obj = read_object(r_num, REAL);
				obj_to_char(target_obj, tmp_ch);
			}
			}
	}




  switch (number(0, 60))
    {
    case 0:
		do_emote(ch, "turns himself into a box.", 0);
		return(1);
	break;
	case 1:
		do_emote(ch, "starts to climb an invisible rope.", 0);
		return(1);
	break;
	case 2:
		do_emote(ch, "is stuck inside a box that you can't see. Wierd!", 0);
		return(1);
	break;
	case 3:
		do_emote(ch, "is crawling around on the floor like a dog.", 0);
		return(1);
	break;
	case 4:
		do_emote(ch, "is lying on the floor playing dead.", 0);
		return(1);
	break;
	case 5:
		do_emote(ch, "pretends like he's riding a big stallion.", 0);
		return(1);
	break;
	case 6:
		do_emote(ch, "is stuck under an enormous rock....or so he thinks.", 0);
		return(1);
	break;
	case 7:
		do_emote(ch, "mimics your every move.", 0);
		return(1);
	break;
	case 8:
		do_emote(ch, "begins to eat some imaginary cheese.", 0);
		return(1);
	break;
	case 9:
		do_emote(ch, "sniffs your behind.", 0);
		return(1);
	break;
	case 10:
		do_emote(ch, "would like a kiss from you.", 0);
		return(1);
	break;
	case 11:
		do_emote(ch, "flexes his muscles and pretends like he's the great Lord Ignatius.", 0);
		return(1);
	break;
	case 12:
		do_emote(ch, "sticks his nose in your crotch and won't go away.", 0);
		return(1);
	break;
	case 13:
		do_emote(ch, "rolls up in a tiny ball for you to kick him.", 0);
		return(1);
	break;
	case 14:
		do_emote(ch, "takes a black magic marker and begins to give you a cute mustache.", 0);
		return(1);
	break;
	case 15:
		do_emote(ch, "worships Abagor as his patron saint of stupidity.", 0);
		return(1);
	break;
	case 16:
		do_emote(ch, "wishes he could be just like you.", 0);
		return(1);
	break;
	case 17:
		do_emote(ch, "pees his pants every time that meanie Tsaron shows up.", 0);
		return(1);
	break;
	case 18:
		do_emote(ch, "begins falling and falling and falling and falling and....", 0);
		return(1);
	break;
	case 19:
		do_emote(ch, "is head over heels in love....no, he's just head over heels.", 0);
		return(1);
	break;
	case 20:
		do_emote(ch, "moshes wildly with you and tries to steal your wallet.", 0);
		return(1);
	break;
	case 21:
		do_emote(ch, "begs for a couple of cents with those poor puppy dog eyes.", 0);
		return(1);
	break;
	case 22:
		do_emote(ch, "is sitting with no chair...Amazing!", 0);
		return(1);
	break;
	case 23:
		do_emote(ch, "shows you his wallet and the pictures of all his kids.", 0);
		return(1);
	break;
	case 24:
		do_emote(ch, "would like it if you would give him a massage.", 0);
		return(1);
	break;
	case 25:
		do_emote(ch, "would shave his butt and learn to walk backwards if he looked like you!", 0);
		return(1);
	break;
	case 26:
		do_emote(ch, "is gradually getting squished tighter and tighter into a magical force field!", 0);
		return(1);
	break;
	case 27:
		do_emote(ch, "is climbing a ladder, but going nowhere.", 0);
		return(1);
	break;
	case 28:
		do_emote(ch, "writhes on the floor doing strange undulating motions.", 0);
		return(1);
	break;
	case 29:
		do_emote(ch, "begins to do laps around the room on his hands.", 0);
		return(1);
	break;
	case 30:
		do_emote(ch, "sits down at his computer and starts to MUD.", 0);
		return(1);
	break;
	default:
    return(0);
	}
}

/* Nadia */
int nadia(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	char tbuf[80];
	struct char_data *vict;
	struct obj_data *obj;
	int test=0;
	int has_key = 0;
	int has_pill = 0;
        int keyrnum = 0;
	int pillrnum = 0;
	struct obj_data *i;

	if(!AWAKE(ch)) return(FALSE);

	/*TALK TO ME!!!*/

	if (cmd == 531)	/*Talk*/
	{

		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);

		if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}

		if (mob_index[vict->nr].virtual != NADIA) return(FALSE);

		keyrnum = real_object(NADIA_KEY);
		for (i = vict->carrying; i; i = i->next_content) {
			if (has_key != 1) {
				if (i->item_number == keyrnum) has_key = 1;
				else has_key = 0;
			}
		}

		/*Quest Text*/
		if (has_key == 0)
		{
			pillrnum = real_object(NADIA_PILL);
			for (i = vict->carrying; i; i = i->next_content) {
				if (has_pill != 1) {
					if (i->item_number == pillrnum) has_pill = 1;
					else has_pill = 0;
				}
			}

			if (has_pill == 0)
			{
				act("$n says, 'I don't know anything about a sceptre or any dragons, but I'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'do know who might be able to help you.  Elamin, a great magician'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'used to work for the king is imprisoned in the keep tower.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'If you do me a favor and kill that dastardly Mime Jerry, I will'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'give you the key to Elamin's prison.  That fool king will learn'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'to pay the consequences when he doesn't come to see me for weeks!'", FALSE, vict, 0, 0, TO_ROOM);
				if (vict->equipment[HOLD]) {
					do_remove(vict,vict->equipment[HOLD]->name,0);
	      				sprintf(buf,"green-key %s",GET_NAME(ch));
      					do_give(vict,buf,0);
					act("$n says, 'Here is the key to the west wing.  You'll need it.'", FALSE, vict, 0, 0, TO_ROOM);
					return(TRUE);
				}

			}
		}

		else if (has_key == 1)
		{
			act("$n says, 'Here is the key.  I will tell you no more.  Go, before I change my mind.'", FALSE, vict, 0, 0, TO_ROOM);
      			sprintf(buf,"key-elamin %s",GET_NAME(ch));
      			do_give(vict,buf,0);
			return(TRUE);
		}
	}
	return(FALSE);
}

/* Elamin */
int elamin(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	char tbuf[80];
	struct char_data *vict;
	struct obj_data *obj;
	int test=0, r_num=0;
	int has_pen = 0, has_key = 0;
        int penrnum = 0, keyrnum = 0;
	struct obj_data *i;

	if(!AWAKE(ch)) return(FALSE);

	/*TALK TO ME!!!*/

	if (cmd == 531)	/*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);
	      if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}
	      if (mob_index[vict->nr].virtual != ELAMIN) return(FALSE);

		penrnum = real_object(PEN_MIGHT);
		for (i = vict->carrying; i; i = i->next_content) {
			if (has_pen != 1) {
				if (i->item_number == penrnum) has_pen = 1;
				else has_pen = 0;
	                }
		}

		/*Quest Text*/
		if (has_pen == 0)
		{
			act("$n says, 'Hello there.  How did you manage to get so far into'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'the keep?  Nevermind, it is of no importance.  So'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'you are looking for the infamous realm of dragons eh?'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'Very well.  I'll help you since that bugger of a king'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'locked me up here.  I will give you the sceptre head'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'if you do me a favor.  This pain in the neck jester'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'took a very powerful magical item from me a short time'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'back.  If you will go to the Jester and obtain my pen'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'and bring it back to me, the sceptre will be yours.'", FALSE, vict, 0, 0, TO_ROOM);

			keyrnum = real_object(JESTER_KEY);
			for (i = vict->carrying; i; i = i->next_content) {
				if (has_key != 1) {
				if (i->item_number == keyrnum) has_key = 1;
				else has_key = 0;
	                }
			if (has_key == 1) {
  	    			sprintf(buf,"skeleton-key %s",GET_NAME(ch));
      				do_give(vict,buf,0);
			}

		}


			return(TRUE);
		}

		else
		{
			act("$n says, 'Please depart at your convenience.  I have work to do'", FALSE, vict, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}

	else if (cmd == 72)
	{
		arg=one_argument(arg, obj_name);
		if ((!*obj_name) || (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))) return(FALSE);

		arg=one_argument(arg, vict_name);
		if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))) || (IS_PC(vict))) return(FALSE);

		if (vict->specials.fighting)
		{
			send_to_char("Not while they are fighting!\n\r", ch);
			return(TRUE);
		}

		if (mob_index[vict->nr].virtual != ELAMIN) return(FALSE);

        	if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
      		if ((obj_index[obj->item_number].virtual != PEN_MIGHT))
			{
      		sprintf(buf, "%s That is not the item I seek.",GET_NAME(ch));
    			do_tell(vict,buf,19);
         		return(TRUE);
			}
		}
   		else
		{
			sprintf(buf,"%s %s",obj_name,vict_name);
      		do_give(ch,buf,0);
			if(obj_index[obj->item_number].virtual == PEN_MIGHT) test=1;
			else return(TRUE);
		}
 		if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
			test=1;
	  		do_give(ch,"pen-might elamin",0);
 		}
	}

	else return(FALSE);

	if (test==1)
	{
		act("$n says, 'Thank you so much.  Here is the sceptre.  Please be'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'careful.  I wouldn't want anything bad to happen to'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'you in your journeys.'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n hands over the dragon sceptre.", FALSE, vict, 0, 0, TO_ROOM);
                if ((r_num = real_object(DRAGON_SCEPTRE_ONE)) >= 0) {
                        obj = read_object(r_num, REAL);
                        obj_to_char(obj, ch);
                }
		return(TRUE);
	}

	return(FALSE);

}

int goblin_chuirgeon (struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	char tbuf[80];
	struct char_data *vict;
	struct obj_data *obj;
	int test=0, r_num=0;
	struct obj_data *i;

	if(!AWAKE(ch)) return(FALSE);

	if (cmd == 531)	/*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);
		if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}
		if (mob_index[vict->nr].virtual != GOBLIN_CHUIRGEON) return(FALSE);

		if (GET_LEVEL(ch, BestClassIND(ch)) > 50) {
			send_to_char("Get real.  You shouldn't be playing around with him.\n\r", ch);
			return (TRUE);
		}

		act("$n says, 'Lo.  Yoo be lookin for abit o doctrin?  I offa my'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'ade fo a wee tiny gold pyle.  Yoo be stronga, I'", FALSE, vict, 0, 0, TO_ROOM);
		act("$n says, 'swayr.  Jus type 'buy' and I get right ta work.'", FALSE, vict, 0, 0, TO_ROOM);

	return(TRUE);
	}

	else if (cmd == 56) /*Buy*/
	{
		if (GET_LEVEL(ch, BestClassIND(ch)) > 50) {
			send_to_char("Get real.  You shouldn't be playing around with him.\n\r", ch);
			return (TRUE);
		}


		test = GET_CON(ch)+1;
		test = test*100000;

		if (GET_GOLD(ch) < test) {
			act("$n says, 'Wat I look lyk?  Freebee?  Yoo need mo money.'", FALSE, mob, 0, 0, TO_ROOM);
			return (TRUE);
		}
		else if (GET_EXP(ch) < 100000000) {
			act("$n says, 'Yoo need mo expuriense befo I can help yoo.'", FALSE, mob, 0, 0, TO_ROOM);
			return (TRUE);
		}
		else if (GET_CON(ch) > 17) {
			act("$n says, 'Yoo too strong fo mee to help.'", FALSE, mob, 0, 0, TO_ROOM);
			return (TRUE);
		}
		else {

		act("$n says, 'Vewy good.  I begin now.'", FALSE, mob, 0, 0, TO_ROOM);
		act("$n pushes you to the floor.", FALSE, mob, 0, 0, TO_ROOM);
		act("$n walks over to one of the cages and pulls a small bunny out.", FALSE, mob, 0, 0, TO_ROOM);
		act("$n slowly returns, gutting the bunny then quickly slicing your stomach open.", FALSE, mob, 0, 0, TO_ROOM);
		WAIT_STATE(ch,PULSE_VIOLENCE*12);
		GET_POS(ch) = POSITION_STUNNED;
       		GET_MANA(ch) = 1;
		GET_MOVE(ch) = 1;
		GET_HIT(ch) = 1;
		GET_CON(ch) += 1;
		GET_GOLD(ch) -= test;
		GET_EXP(ch) -= GET_CON(ch)*5000000;
		return (TRUE);
		}
	}
	else {
		return (FALSE);
	}

	return (FALSE);
}

int zork(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
  struct char_data *temp_ch;
  struct obj_data *i;
  char argument[150];
  char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
  char tbuf[80];
  struct char_data *vict;
  struct obj_data *obj;
  int test=0, r_num=0;

	if(!AWAKE(ch)) return(FALSE);

	if (cmd == 531) /*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);
	      	if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}
	      	if (mob_index[vict->nr].virtual != ZORK) return(FALSE);

		if (get_obj_vis_world(ch, "black pill thunder mountain", NULL))
		{
			test = 1;
		}
		if (get_obj_vis_world(ch, "blue pill thunder mountain", NULL))
		{
			test = 1;
		}
		if (get_obj_vis_world(ch, "elamin pen might", NULL))
		{
			test = 1;
		}

		if (test == 0)
		{
			act("$n says, 'So, Elamin has finally found a courier to aid him.'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'How quaint.  Well, I'll tell you what I'll do.  If'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'you can figure out a simple riddle, I will grant'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'poor Elamin's pen.  Eat the correct pill and the'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'pen will be yours.  Eat the wrong pill and you will'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'face an instant death.'", FALSE, vict, 0, 0, TO_ROOM);
			if ((r_num = real_object(BLACK_PILL)) >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj, ch);
			}
			if ((r_num = real_object(BLUE_PILL)) >= 0) {
				obj = read_object(r_num, REAL);
				obj_to_char(obj, ch);
			}
			act("$n hands over 2 pills: one black and one blue.", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'The riddle is this:'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'When one stands on solid ground'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'They look up and cannot deny'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'That the law of nature's demand'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'Determines the shade of this cloudless sky'", FALSE, vict, 0, 0, TO_ROOM);

			return(TRUE);
		}
		if (test == 1)
		{
			act("$n says, 'I can only tell you the riddle:'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'When one stands on solid ground'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'They look up and cannot deny'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'That the law of nature's demand'", FALSE, vict, 0, 0, TO_ROOM);
			act("$n says, 'Determines the shade of this cloudless sky'", FALSE, vict, 0, 0, TO_ROOM);

			return(TRUE);
		}
	}

  if(IS_PC(ch))
    return(0);

  temp_ch = FindAnyVictim(ch);
	if (temp_ch && IS_PC(temp_ch)) {

  switch (number(0, 30))
	{
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
		strcpy(argument, "has $c0009'LOSER' $c0007stamped across their forehead.");
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
		strcpy(argument, "has fallen off of the stupid tree and hit every branch on the way down.");
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
		strcpy(argument, "says, 'Do not listen to a word I say, I'm incompetent'");
	break;
	case 20:
		strcpy(argument, "wants you now!  Need only apply!");
	break;
	case 21:
		strcpy(argument, "is having passionate thoughts about...everyone...all at once...");
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

    return(0);
	}

	if (temp_ch->player.title)
          free(temp_ch->player.title);
    temp_ch->player.title = strdup(argument);
    return(1);
	}



return(0);
}

int thunder_blue_pill(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

  char buf[MAX_INPUT_LENGTH];
  struct obj_data *i;
  struct obj_data *obj;
  int pillrnum = 0;
  int has_pill = 0;
  int r_num = 0;
  extern int gLightLevel;

  	if (cmd == 12) /*Eat*/
	{
		only_argument(arg,buf);

        	if(!(obj = get_obj_in_list_vis(ch,buf,ch->carrying)))  {
	                act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        	        return (TRUE);
        	}

		pillrnum = real_object(BLUE_PILL);
		for (i = ch->carrying; i; i = i->next_content) {
			if (has_pill != 1) {
				if (i->item_number == pillrnum) has_pill = 1;
				else has_pill = 0;
	                }
		}

		if (has_pill == 1)
		{
			if (obj->item_number != pillrnum) return(FALSE);

	        	act("$n eats $p",TRUE,ch,obj,0,TO_ROOM);
			act("You eat the $o.",FALSE,ch,obj,0,TO_CHAR);
			extract_obj(obj);

			if (time_info.hours > 6)
			{
				if (time_info.hours < 20)
				{
				if ((r_num = real_object(PEN_MIGHT)) >= 0) {
					obj = read_object(r_num, REAL);
					obj_to_char(obj, ch);
					send_to_char("Elamin's Pen of Might bursts out of thin air and lands in your hands.\n\r", ch);
					act("Elamin's Pen of Might bursts out of thin air and lands in $n's hands.", FALSE, ch, 0, 0, TO_ROOM);
				}
				}
				else
				{
				send_to_char("You realize too late that you chose the wrong pill.\n\r", ch);
				act("$n realizes too late that they chose the wrong pill as they fall to the ground.", FALSE, ch, 0, 0, TO_ROOM);
				GET_HIT(ch) = -1;
				die(ch, '\0');
				}
			}
			else
			{
				send_to_char("You realize too late that you chose the wrong pill.\n\r", ch);
				act("$n realizes too late that they chose the wrong pill as they fall to the ground.", FALSE, ch, 0, 0, TO_ROOM);
				GET_HIT(ch) = -1;
				die(ch, '\0');
			}
		return(TRUE);
		}

	}

	return(FALSE);
}

int thunder_black_pill(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

  char buf[MAX_INPUT_LENGTH];
  struct obj_data *i;
  struct obj_data *obj;
  int pillrnum = 0;
  int has_pill = 0;
  int r_num = 0;
  extern int gLightLevel;

  	if (cmd == 12) /*Eat*/
	{
		only_argument(arg,buf);

        	if(!(obj = get_obj_in_list_vis(ch,buf,ch->carrying)))  {
	                act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
        	        return (TRUE);
        	}

		pillrnum = real_object(BLACK_PILL);
		for (i = ch->carrying; i; i = i->next_content) {
			if (has_pill != 1) {
				if (i->item_number == pillrnum) has_pill = 1;
				else has_pill = 0;
	                }
		}

		if (has_pill == 1)
		{
			if (obj->item_number != pillrnum) return(FALSE);

	        	act("$n eats $p",TRUE,ch,obj,0,TO_ROOM);
			act("You eat the $o.",FALSE,ch,obj,0,TO_CHAR);
			extract_obj(obj);

			if (time_info.hours < 7)
			{
				if ((r_num = real_object(PEN_MIGHT)) >= 0) {
					obj = read_object(r_num, REAL);
					obj_to_char(obj, ch);
					send_to_char("Elamin's Pen of Might bursts out of thin air and lands in your hands.\n\r", ch);
					act("Elamin's Pen of Might bursts out of thin air and lands in $n's hands.", FALSE, ch, 0, 0, TO_ROOM);
				}

			}
			else
			{
				if (time_info.hours > 19)
				{
				if ((r_num = real_object(PEN_MIGHT)) >= 0) {
					obj = read_object(r_num, REAL);
					obj_to_char(obj, ch);
					send_to_char("Elamin's Pen of Might bursts out of thin air and lands in your hands.\n\r", ch);
					act("Elamin's Pen of Might bursts out of thin air and lands in $n's hands.", FALSE, ch, 0, 0, TO_ROOM);
				}
				}
				else
				{
				send_to_char("You realize too late that you chose the wrong pill.\n\r", ch);
				act("$n realizes too late that they chose the wrong pill as they fall to the ground.", FALSE, ch, 0, 0, TO_ROOM);
				GET_HIT(ch) = -1;
				die(ch,'\0');
				}
			}
			return(TRUE);
		}

	}

	return(FALSE);
}

int thunder_sceptre_one(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
  char buf[256], obj_name1[180], obj_name2[180];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj;
  struct obj_data *obj1;
  struct obj_data *obj2;
  struct char_data *tmp_char;
  int r_num = 0;

	if (cmd == 67) /*put*/
	{
	      	argument_interpreter(arg, arg1, arg2);

		if ((!*arg1) || (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying)))) return(FALSE);
		if ((!*arg2) || (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)))) return(FALSE);

      		if ((obj_index[obj1->item_number].virtual != EYE_DRAGON)) return(FALSE);
		if ((obj_index[obj2->item_number].virtual != DRAGON_SCEPTRE_ONE)) return(FALSE);

		act("The sceptre rumbles slightly as it unites with the eye of the dragon.", FALSE, ch, 0, 0, TO_ROOM);
		act("The sceptre rumbles slightly as it unites with the eye of the dragon.", FALSE, ch, 0, 0, TO_CHAR);
		extract_obj(obj1);
		extract_obj(obj2);
		if ((r_num = real_object(DRAGON_SCEPTRE_TWO)) >= 0) {
			obj = read_object(r_num, REAL);
			obj_to_char(obj, ch);
		}

		return(TRUE);


	}

	return(FALSE);
}

int thunder_sceptre_two(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
  char buf[256], obj_name1[180], obj_name2[180];
  char arg1[128];
  char arg2[128];
  struct obj_data *obj;
  struct obj_data *obj1;
  struct obj_data *obj2;
  struct char_data *tmp_char;
  int r_num = 0;
  struct char_data *tmp, *tmp2;
  struct room_data *room;

	if (cmd == 67) /*put*/
	{
	      	argument_interpreter(arg, arg1, arg2);

		if ((!*obj_name1) || (!(obj1 = get_obj_in_list_vis(ch, arg1, ch->carrying)))) return(FALSE);
		if ((!*obj_name2) || (!(obj2 = get_obj_in_list_vis(ch, arg2, ch->carrying)))) return(FALSE);

      		if ((obj_index[obj1->item_number].virtual != EYE_DRAGON)) return(FALSE);
		if ((obj_index[obj2->item_number].virtual != DRAGON_SCEPTRE_TWO)) return(FALSE);

		act("The sceptre rumbles slightly as it unites with the eye of the dragon.", FALSE, ch, 0, 0, TO_ROOM);
		act("The sceptre rumbles slightly as it unites with the eye of the dragon.", FALSE, ch, 0, 0, TO_CHAR);
		act("The sceptre flies out of $n's grasp and spins rapidly in the air.", FALSE, ch, 0, 0, TO_ROOM);
		act("The sceptre flies out of your grasp and spins rapidly in the air.", FALSE, ch, 0, 0, TO_CHAR);
		act("A blinding white light fills the room.  You blink and find yourself in a new room.", FALSE, ch, 0, 0, TO_ROOM);
		act("A blinding white light fills the room.  You blink and find yourself in a new room.", FALSE, ch, 0, 0, TO_CHAR);
		extract_obj(obj1);
		extract_obj(obj2);

		room = real_roomp(ch->in_room);

  		for (tmp = room->people;tmp;tmp=tmp2) {
    			tmp2 = tmp->next_in_room;
			if ( in_group(ch, tmp) && (!tmp->specials.fighting) &&
			(IS_PC(tmp) || IS_SET(tmp->specials.act,ACT_POLYSELF))
			&& IS_AFFECTED(tmp,AFF_GROUP)) {
      				char_from_room(tmp);
      				char_to_room(tmp, CALM_BEFORE_STORM);
      				do_look(tmp, "\0", 0);
    			}
  		}

		return(TRUE);


	}

	return(FALSE);
}

int bahamut_prayer(struct char_data *ch, struct char_data *vict)
{
  char buf[256];
  int hitp = 0;
  int test = 0;
  struct char_data *tmp_ch;
  struct char_data *i;

  if(IS_PC(ch))
    return(0);

	if (GET_HIT(ch) <= 2000)
	{
		if (ch->mult_att < 4)
		{
			ch->mult_att = 4;
		}

		for (i=real_roomp(ch->in_room)->people; i; i = i->next_in_room)
		{
			GET_HIT(i) -= 25;
			send_to_char("$c0011A blinding holy light engulfs the room and seers your life away!\n\r", i);
			if (mob_index[i->nr].virtual == BAHAMUT) {
				GET_HIT(i) += 25;
			}
		}

	return(FALSE);
	}

	if (GET_HIT(ch) <= 3000)
	{
		if (ch->mult_att >= 4)
		{
			ch->mult_att = 1;
		}
		else if (ch->mult_att == 1)
		{
			act("$c0011$n bows down and prays to the dragon lord.", FALSE, ch, 0, 0, TO_ROOM);
		}
		else
		{
			ch->mult_att = 4;
		}

	return(FALSE);
	}

return(FALSE);
}

int tmk_guard(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
	char tbuf[80];
	struct char_data *vict;
	struct obj_data *obj;

	if(!AWAKE(ch)) return(FALSE);

	/*TALK TO ME!!!*/

	if (cmd == 531)	/*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);
	      	if (vict->specials.fighting)
		{
		   send_to_char("Not while they are fighting!\n\r", ch);
		   return(TRUE);
		}
	      	if (mob_index[vict->nr].virtual == TMK_GUARD_ONE || mob_index[vict->nr].virtual == TMK_GUARD_TWO)
		{
			switch(number(1, 10)) {

			case 1:
				act("$n says, 'That Bahamut scares me to no end.  I know he's supposed to be good'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'and all, but anything with that much power is just too dangerous!'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'I've heard rumors that his skin is magically empowered to make him'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'resistant to attacks from clubs and such weapons.  Heh.  What I'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'wouldn't give to get some armor made of that skin.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n grins slyly, but turns away quickly to prevent you from seeing.", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 2:
				act("$n says, 'Have you heard the legend of Bahamut, The Platinum Dragon?  According'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'to the stories, when Bahamut feels that he may be in danger, he pauses'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'to pray to the great dragon lord.  And his prayers are answered by an'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'enormous blazing light that virtually melts away the skin.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 3:
				act("$n says, 'Hello!  Hmmm.  Dragons?  From what I understand, there is a great'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'power in the keep that can get you to a mysterious realm of dragons'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'This realm is a gigantic, shifting maze where danger lurks around'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'every corner.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 4:
				act("$n says, 'Watch out for that fountain.  Wierd things happen around that thing.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 5:
				act("$n says, 'I had this wierd dream last night.  Wonder if it means anything.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'These colors kept flashing over and over again in the same pattern:'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'White, Prismatic, Crystaline, Dark Gray, Blue, Black, Gold, Platinum.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'I wonder what it means.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n frowns thoughtfully", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 6:
				act("$n says, 'Stories are spreading about some strange creature in the surounding'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'mountains that can increase a person's stamina.  Wild stories of'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'animal sacrifice and the lord only knows what else.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 7:
				act("$n says, 'A long time ago, the king's magician made a great staff that, when held,'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'could knock over even the largests of beasts.  The king bestowed this'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'staff to Bahamut as a gift for his protection of the keep in a great'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'battle against Tiamat.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 8:
				act("$n rubs his chin in deep thought", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'Tales of fantasy...  I seem to remember a story from when I was a child.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'Crazy stuff really.  But there was this one that really intriqued me.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'Supposedly, there is a spring hidden somewhere in the realm who's waters'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'can restore a person's age if they give some ancient staff to a nymph'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'that guards the it.  Probably rediculous fancy.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			case 9:
				act("$n says, 'Ever hear the legend of the Staff of the Ages?'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'Some say that it's mysteries contain the'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'only possibility for the defeat of Bahamut.'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'Though, I bet thats crazy.  There's *nothing*'", FALSE, vict, 0, 0, TO_ROOM);
				act("$n says, 'that can take out Bahamut, The Ancient Platinum.'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			default:
				act("$n says, 'Leave me alone.  I've got work to do!'", FALSE, vict, 0, 0, TO_ROOM);
			break;
			}
		}
	return(TRUE);
	}
return(FALSE);
}

int starving_man(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
   char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
   char tbuf[80];
   struct char_data *vict;
   struct obj_data *obj;
   int test=0;
   int has_danish = 0;
   struct obj_data *i;

   if(!AWAKE(ch)) return(FALSE);

   	if(!cmd)
   	{
   		if(number(0,10) == 0)
		{
   			act("$n says, 'Pardon me sire, might you spare a poor man some food?'",FALSE, mob, 0, 0, TO_ROOM);
	      		return(TRUE);
   		}
   	}

	/*TALK TO ME!!!*/

	if (cmd == 531)	/*Talk*/
	{
		arg=one_argument(arg, vict_name);

		if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name)))
			|| (IS_PC(vict)) || (vict == ch)) return(FALSE);

      		if (vict->specials.fighting)
		{
		   	send_to_char("Not while they are fighting!\n\r", ch);
		   	return(TRUE);
		}

 	      	if (mob_index[vict->nr].virtual != STARVING_MAN) return(FALSE);

		for (i = vict->carrying; i; i = i->next_content)
			if (has_danish == 0) {
				if(i->obj_flags.type_flag == ITEM_FOOD) has_danish = 1;
				else has_danish = 0;
			}

	      	/*Quest Text*/
		if (has_danish == 0)
		{
   			act("$n says, 'Pardon me sire, might you spare a poor man some food?'",FALSE, vict, 0, 0, TO_ROOM);
			return(TRUE);
		}

		else
		{
   			act("$n says, 'I'm too busy eating to talk!  Go Away!'", FALSE, vict, 0, 0, TO_ROOM);
			return(TRUE);
		}
	}

	else if (cmd == 72) /*give*/
	{
		arg=one_argument(arg,obj_name);

	    	if ((!*obj_name) || (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))) return(FALSE);

   		arg=one_argument(arg, vict_name);
	    	if ((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))) || (IS_PC(vict))) return(FALSE);

		if (vict->specials.fighting)
		{
 			send_to_char("Not while they are fighting!\n\r",ch);
	        	return(TRUE);
		}

		/*Mob is not Starving Man*/
		if (mob_index[vict->nr].virtual != STARVING_MAN) return(FALSE);

		if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
      			if(obj->obj_flags.type_flag != ITEM_FOOD)
			{
      				sprintf(buf, "%s Thank you, but that is not what I desire.",GET_NAME(ch));
    				do_tell(vict,buf,19);
         			return(TRUE);
      			}
			else test = 1;
   		}

		else
		{
			sprintf(buf,"%s %s",obj_name,vict_name);
      			do_give(ch,buf,0);
     			if(obj->obj_flags.type_flag == ITEM_FOOD) test=1;
		}
   		if (GetMaxLevel(ch)<LOW_IMMORTAL)
		{
			sprintf(buf,"%s %s",obj_name,vict_name);
      			do_give(ch,buf,0);
   		}
	}

	else return(FALSE);

	if(test==1)
	{
		if (vict->equipment[WIELD])
		{
      			sprintf(buf, "%s Thank you mighty hero.  Take this as a token of my appreciation.",GET_NAME(ch));
    			do_tell(vict,buf,19);
   			do_remove(vict,vict->equipment[WIELD]->name,0);
      			sprintf(buf,"Staff %s",GET_NAME(ch));
      			do_give(vict,buf,0);
			return(TRUE);
   		}
		else
		{
	      		sprintf(buf, "%s You are indeed a generous person, but I have nothing to offer you in return.", GET_NAME(ch));
			do_tell(vict,buf,19);
			return(TRUE);
   		}
	}

return(TRUE);

}

int grayswandir(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
    char buf[256], name[256];
    struct char_data *victim;
    struct obj_data *object;
    int percent = 0;
    int r_num = 0;
    int v_num1 = 0;
    int v_num2 = 0;
    extern struct dex_app_type dex_app[];


	if (cmd == 157) /* Bash */
	{
		if ((r_num = real_object(GRAYSWANDIR)) >= 0) {
			object = read_object(r_num, REAL);
			v_num1 = obj_index[object->item_number].virtual;
		}

		if (!ch->equipment[HOLD]) return(FALSE);

		object =  ch->equipment[HOLD];

		v_num2 = obj_index[object->item_number].virtual;

		if (v_num1 != v_num2) return(FALSE);

/*		if (ch->equipment[HOLD] != object) return (FALSE);*/

  		if (!ch->skills) return(TRUE);

   		if (check_peaceful(ch,"You feel too peaceful to contemplate violence.\n\r")) return(TRUE);

		if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
 			if (!HasClass(ch, CLASS_WARRIOR|CLASS_PALADIN|CLASS_RANGER|CLASS_BARBARIAN)) {
     				send_to_char("You're no warrior!\n\r", ch);
     				return(TRUE);
    			}
		}

  		only_argument(arg, name);

  		if (!(victim = get_char_room_vis(ch, name))) {
    			if (ch->specials.fighting) {
      				victim = ch->specials.fighting;
    			}
			else {
     		 		send_to_char("Bash who?\n\r", ch);
      				return(TRUE);
    			}
		}

  		if (!(victim = get_char_room_vis(ch, name))) {
    			if (ch->specials.fighting) {
      				victim = ch->specials.fighting;
    			}
			else {
      				send_to_char("Bash who?\n\r", ch);
      				return(TRUE);
    			}
  		}

		if (victim == ch) {
    			send_to_char("Aren't we funny today...\n\r", ch);
    			return(TRUE);
  		}

		if (MOUNTED(victim)) {
    			send_to_char("You can't bash a mounted target!\n\r", ch);
    			return(TRUE);
  		}

		if (MOUNTED(ch)) {
    			send_to_char("You can't bash while mounted!\n\r", ch);
    			return(TRUE);
  		}

		if (ch->attackers > 3) {
    			send_to_char("There's no room to bash!\n\r",ch);
    			return(TRUE);
  		}

  		if (victim->attackers >= 6) {
    			send_to_char("You can't get close enough to them to bash!\n\r", ch);
    			return(TRUE);
  		}

     		SetVictFighting(ch, victim);
     		SetCharFighting(ch, victim);

		percent = number(1,101); /* 101% is a complete failure */
  		percent -= dex_app[GET_DEX(ch)].reaction*10;
  		percent += dex_app[GET_DEX(victim)].reaction*10;

  		if (percent > ch->skills[SKILL_BASH].learned) {
    			if (GET_POS(victim) > POSITION_DEAD) {
		            	act("$c0009You wildly swings $p at $N and fall over.", FALSE, ch, ch->equipment[HOLD], victim, TO_CHAR);
        			act("$c0009$n wildly swings $p at you and falls over.", FALSE, ch, ch->equipment[HOLD], victim, TO_VICT);
	            		act("$c0009$n wildly swings $p at $N and falls over.", FALSE, ch, ch->equipment[HOLD], victim, TO_NOTVICT);
      				GET_POS(ch) = POSITION_SITTING;
    			}
    			LearnFromMistake(ch, SKILL_BASH, 0, 90);
    			WAIT_STATE(ch, PULSE_VIOLENCE*3);
  		}

		else {
    			if (GET_POS(victim) > POSITION_DEAD) {
				act("$c0009You give $N a solid hit with $p that knocks them to the ground.", FALSE, ch, ch->equipment[HOLD], victim, TO_CHAR);
				act("$c0009$n gives you a solid hit with $p that knocks you to the ground.", FALSE, ch, ch->equipment[HOLD], victim, TO_VICT);
				act("$c0009$n gives $N a solid hit with $p that knocks them to the ground.", FALSE, ch, ch->equipment[HOLD], victim, TO_NOTVICT);
      				GET_POS(victim) = POSITION_SITTING;
				WAIT_STATE(victim, PULSE_VIOLENCE*2);
         			GET_POS(victim) = POSITION_SITTING;
    			}
  		}
	  WAIT_STATE(ch, PULSE_VIOLENCE*2);
	}
	else return(FALSE);

	return(TRUE);
}

int bahamut_home(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
  int r_num = 0, test = 0, percent = 0;
  struct obj_data *object, *bahamut;
  char itemname[256],itemtype[80],hidetype[80],buf[256];

	if (cmd == 330)
	{
		if (MOUNTED(ch)) {
        		send_to_char("Not from this mount you cannot!\n\r",ch);
        		return(TRUE);
			}

		if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
			if (!HasClass(ch,CLASS_BARBARIAN|CLASS_WARRIOR|CLASS_RANGER)) {
        			send_to_char("What do you think you are, A tanner?\n\r",ch);
        			return(TRUE);
			}
		}

		bahamut = get_obj_vis(ch, "corpse corpse of Bahamut, The Ancient Platinum Dragon is");

	      	argument_interpreter(arg, itemname, itemtype);

		if (!*itemname) {
 			send_to_char("Tan what?\n\r",ch);
 			return(TRUE);
		}

		if (!*itemtype) {
 			send_to_char("I see that, but what do you wanna make?\n\r",ch);
 			return(TRUE);
		}

  		if (!(object= get_obj_in_list_vis(ch, itemname,real_roomp(ch->in_room)->contents)))
    		{
      			send_to_char("Where did that carcass go?\n\r",ch);
      			return(TRUE);
		}

		if (bahamut != object) return(FALSE);

		if (strcmp(itemtype,"jacket")) return(FALSE);

        	if (object->affected[0].modifier != 0 && object->affected[1].modifier != 0)
		{

 			percent = number(1,101); /* 101% is a complete failure */

			if (ch->skills && ch->skills[SKILL_TAN].learned && GET_POS(ch) > POSITION_SLEEPING) {
 				if (percent > ch->skills[SKILL_TAN].learned) {
   					object->affected[1].modifier=0; /* make corpse unusable for another tan */
					sprintf(buf,"You hack at %s but manage to only destroy the hide.\n\r", object->short_description);
   					send_to_char(buf,ch);
					sprintf(buf,"%s tries to skins %s for it's hide, but destroys it.", GET_NAME(ch),object->short_description);
   					act(buf,TRUE, ch, 0, 0, TO_ROOM);
   					LearnFromMistake(ch, SKILL_TAN, 0, 95);
   					WAIT_STATE(ch, PULSE_VIOLENCE*3);
  					return(TRUE);
 				}
				else
				{
           				if ((r_num = real_object(BAHAMUT_SKIN)) >= 0)
           				{
	   					object->affected[1].modifier=0; /* make corpse unusable for another tan */
						sprintf(buf, "It seems you are able to make some very stylish body armor.");
						send_to_char(buf, ch);
             					sprintf(buf, "%s fashions some very fine body armor out of the %s.", GET_NAME(ch), object->short_description);
						act(buf,TRUE, ch, 0, 0, TO_ROOM);
						object = read_object(r_num, REAL);
             					obj_to_char(object,ch);
						return(TRUE);
           				}
				}
			}
			else return(FALSE);
		}
		else
		{
      			send_to_char("Sorry, nothing left of the carcass to make an item with.\n\r",ch);
      			return(TRUE);
		}
	}

return(FALSE);
}

//{ 44129,  Magic_Pool}  in proc_assign and add to protos

int Magic_Pool(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
  char buf[MAX_INPUT_LENGTH];

  //send_to_char("Start proc",ch);
  if (cmd==11) { /*11 = drink */
    only_argument(arg,buf);
	 send_to_char("pool?",ch);
    if (!str_cmp(buf, "pool")) {
       send_to_char("not pool",ch);
      return(FALSE);
    }

  	if(GET_COND(ch,THIRST)>21 || GET_COND(ch,FULL)>21) {
		act("Ohhh.. Your stomach can't handle anymore.",FALSE,ch,0,0,TO_CHAR);
		return(TRUE);
	}
    send_to_char("You drink from the pool\n\r", ch);
    act("$n drinks from the pool", FALSE, ch, 0, 0, TO_ROOM);

    GET_COND(ch,THIRST) = 24;
    GET_COND(ch,FULL)+=1;

    act("You suddently feel much better.",FALSE,ch,0,0,TO_CHAR);
    cast_heal(50, ch, "", SPELL_TYPE_SPELL, ch, 0);
    return(TRUE);
  }
  return(FALSE);
}


//{ 44115,  Read_Room} in proc_assign and add to protos..

int Read_Room(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
  int key_room;
  char buf[MAX_INPUT_LENGTH];
  struct obj_data *obj;
	//send_to_char("entering proc",ch);
  if (cmd!=63)  /*63 = read */
    return FALSE;
  if (!HasClass(ch, CLASS_PSI))
	return FALSE;

  only_argument(arg,buf);
  if (str_cmp(buf, "book")) {
   	 send_to_char("cmp book and buf .. not same",ch);
     return(FALSE);
  }

  obj = get_obj_in_list_vis(ch,buf,ch->carrying);
	if (!obj)
		return(FALSE);
  if(obj_index[obj->item_number].virtual == real_roomp(ch->in_room))  {
  	key_room = 1+ch->in_room;
  	act("$n reads the book then disappears.", FALSE, ch, 0, 0, TO_ROOM);
  	char_from_room(ch);
  	char_to_room(ch, key_room);
  	act("$n suddently appears.", FALSE, ch, 0, 0, TO_ROOM);
  	return TRUE;
  }

  return FALSE;
}


char questwinner[50];
int questwon = 0;

int questNumber2 = -1;
int questNumber = -1;
/* Questor GOD */
int QuestorGOD(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{

	//extern struct QuestItem QuestList[4][IMMORTAL];
   char obj_name[80], vict_name[80], buf[MAX_INPUT_LENGTH];
   char tbuf[80];
   struct char_data *vict;
   struct obj_data *obj;
   int test;
   int itemgranted = 0;
   static time_t time_diff = 50;
   static time_t last_time = 0;
	if (questNumber == -1) {
	   questNumber = number(0,49);
		questNumber2 = number(0,3);
	}

	//Ask what the quest is?
	if (cmd == 86)
	{ /* ask */

      arg=one_argument(arg, vict_name);
      if((!*vict_name) || (!(vict = get_char_room_vis(ch, vict_name))))
		printf("ReturnFALSE");
		//return(FALSE);

      if(!(strcmp(lower(arg)," what is the quest?"))) {
		send_to_char("$c0006You ask him 'What is the quest?",ch);
		do_say(vict, "Solve my quest and you shall be rewarded!",0);
	   do_say(vict, QuestList[questNumber2][questNumber].where,0);
		return (TRUE);
		}
	  return(FALSE);
    }



   if(!cmd)
   {
      if((time_diff > 150))
      {
         time_diff = 0;  /* reset */
         last_time = time(NULL);
         do_say(ch, "Solve my quest and you shall be rewarded!",0);
	     do_say(ch, QuestList[questNumber2][questNumber].where,0);

         return(TRUE);
      }
      else
      {
         time_diff = time(NULL) - last_time;
         //printf("Questor Time?? - %d.\n", time_diff);
      }
      return(FALSE);
   }

   if(!AWAKE(ch)) return(FALSE);

   if (cmd == 72) { /* give */
   	/* determine the correct obj */
   	  arg=one_argument(arg,obj_name);
      if (!*obj_name) {
     	send_to_char("Give what?\n\r",ch);
      	return(FALSE);
      }
      if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying))) {
      	send_to_char("Give what?\n\r",ch);
      	return(TRUE);
      }
   	  arg=one_argument(arg, vict_name);
      if(!*vict_name)	{
      	send_to_char("To who?\n\r",ch);
      	return(FALSE);
      }
      if (!(vict = get_char_room_vis(ch, vict_name)))	{
        send_to_char("To who?\n\r",ch);
        return(FALSE);
      }
      if (vict->specials.fighting) {
 	 	send_to_char("Not while they are fighting!\n\r",ch);
       	return(TRUE);
      }

      if(IS_PC(vict)) return(FALSE);

      if (GetMaxLevel(ch)<99) {
      	if ((obj_index[obj->item_number].virtual != QuestList[questNumber2][questNumber].item)) {
      		sprintf(buf, "%s That is not the item I seek.",GET_NAME(ch));
    			do_tell(vict,buf,19);
         	return(TRUE);
      	} else {


			if(!(strcmp(questwinner,GET_NAME(ch)))) {
				questwon++;
			} else {
				sprintf(questwinner,"%s",GET_NAME(ch));
				questwon = 0;
			}


			do_say(vict, "Thanks-you!!  Just what i needed!! Here ya go",0);


		     switch(number(0,10)+questwon) {
				 case 0:
		       		itemgranted = 9611;
		       		break;
		       	 case 1:
		       	 	itemgranted = 4050;
		       	 	break;
		       	 default:
		       	 	itemgranted = 4050;

		     }
		       obj = read_object(itemgranted, VIRTUAL);
    		   obj_to_char(obj, ch);

		 	   act("$N gives you $p.",TRUE,ch,obj,vict,TO_CHAR);
    		   act("$N gives $p to $n.",TRUE,ch,obj,vict,TO_ROOM);

				do_say(vict, "Hey.. I got something else for you to get too.",0);
				do_junk(vict, obj->name,0);


			/*pick new quest */
			questNumber=number(0,49);
			questNumber2=number(0,3);

			//do_say(vict, "Hey.. I got something else for you to get too.",0);
			time_diff = 201;

		}

   	}



  }
	return(FALSE);

}



/* Toy for Banon.. a new say for him (GH) April 2002 */
int godsay(struct char_data *ch, int cmd, char *argument, struct obj_data *obj, int type)
{


   int i;
     char buf[MAX_INPUT_LENGTH+80];

  if(cmd != 17)
    return(FALSE);

   dlog("in godsay");

     if (apply_soundproof(ch))
       return;

     for (i = 0; *(argument + i) == ' '; i++);

     if (!*(argument + i))
       send_to_char("Yes, but WHAT do you want to say?\n\r", ch);
     else	{
       sprintf(buf,"$c0012-=$c0015$n$c0012=-$c0011 says '$c0014%s$c0011'", argument + i);
       act(buf,FALSE,ch,0,0,TO_ROOM);
       if (IS_NPC(ch)||(IS_SET(ch->specials.act, PLR_ECHO))) {
         sprintf(buf,"$c0015You say '$c0014%s$c0015'\n\r", argument + i);
         send_to_char(buf, ch);
       }
  }

}
