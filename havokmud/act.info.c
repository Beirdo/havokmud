
/************************************************************************
 *  Usage : Informative commands.                                          *
 *  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
 *************************************************************************
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "protos.h"
#define NEWHELP_FILE      "ADD_HELP"      /* New help to add (In db.h too) */

/* extern variables */
extern struct zone_data *zone_table;
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif
extern struct descriptor_data *descriptor_list;
extern struct char_data *character_list;
extern struct obj_data *object_list;

extern int  top_of_world;
extern int  top_of_zone_table;
extern int  top_of_mobt;
extern int  top_of_objt;
extern int  top_of_p_table;

extern char *exits[];
extern char credits[MAX_STRING_LENGTH];
extern char news[MAX_STRING_LENGTH];
extern char info[MAX_STRING_LENGTH];

extern char wizlist[MAX_STRING_LENGTH];
extern char iwizlist[MAX_STRING_LENGTH];
extern char *dirs[];
extern char *where[];
extern char *color_liquid[];
extern char *fullness[];
extern char *RaceName[];
extern int RacialMax[][MAX_CLASS];
extern char *spell_desc[];
extern char *spells[];
extern struct spell_info_type spell_info[MAX_SPL_LIST];
extern char *system_flag_types[];
extern char *exits[];
  extern char *listexits[];
  extern struct index_data *mob_index;
  extern char *item_types[];
  extern char *extra_bits[];
  extern char *apply_types[];
  extern char *affected_bits[];
  extern char *affected_bits2[];
  extern char *immunity_names[];
  extern long Uptime;
  extern long room_count;
  extern long mob_count;
  extern long obj_count;
  extern long total_connections;
  extern long total_max_players;
extern long SystemFlags;
  extern char *spells[];
  extern int spell_status[];
  extern const struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
  extern const char *connected_types[];
 extern struct radix_list radix_head[];
  extern int top_of_wizhelpt;
  extern struct help_index_element *wizhelp_index;
  extern FILE *wizhelp_fl;
 extern struct radix_list radix_head[];
  extern int top_of_helpt;
  extern struct help_index_element *help_index;
  extern FILE *help_fl;
  extern char help[MAX_STRING_LENGTH];
  extern struct weather_data weather_info;
  extern struct time_info_data time_info;
  extern const char *weekdays[];
  extern const char *month_name[];
  extern const struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
  extern char *RaceNames[];
  extern const char *fight_styles[];
  extern struct skillset warriorskills[];
  extern struct skillset scwarskills[];
  extern struct skillset thiefskills[];
  extern struct skillset barbskills[];
  extern struct skillset bardskills[];
  extern struct skillset monkskills[];
  extern struct skillset mageskills[];
  extern struct skillset sorcskills[];
  extern struct skillset clericskills[];
  extern struct skillset druidskills[];
  extern struct skillset paladinskills[];
  extern struct skillset rangerskills[];
  extern struct skillset psiskills[];
  extern struct skillset warninjaskills[];
  extern struct skillset thfninjaskills[];
  extern struct skillset allninjaskills[];
  extern struct skillset loreskills[];

/* extern functions */

void log (char *s);
int GET_RADIUS(struct char_data *ch);
int IS_UNDERGROUND(struct char_data *ch);
int MobLevBonus(struct char_data *ch);
int bitvector_num(int temp);
int clearpath(struct char_data *ch, long room, int direc);
int atoi (const char *s);
int CAN_SEE_OBJ( struct char_data *ch, struct obj_data *obj);
void ch_printf(struct char_data *ch, char *fmt, ...);
char *formatNum(int foo);
char *DescAge(int age,int race);
int pc_num_class(int clss);
int color_strlen(struct char_data *ch, char *arg, int cmd);

// void *malloc (size_t size);

     void free (void *ptr);

//     void *realloc (void *ptr, size_t size);

     void *calloc (size_t nelem, size_t	elsize);

//     void *memalign (size_t alignment, size_t size);

//     void *valloc (size_t size);



struct time_info_data age(struct char_data *ch);
void page_string(struct descriptor_data *d, char *str, int keep_internal);
int track( struct char_data *ch, struct char_data *vict);
int GetApprox(int num, int perc);
int SpyCheck(struct char_data *ch);
int remove_trap( struct char_data *ch, struct obj_data *trap);
void do_actual_wiz_help(struct char_data *ch, char *argument, int cmd);

struct obj_data *get_obj_in_list_vis(struct char_data *ch, char *name,
				     struct obj_data *list);


/* intern functions */

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show);
char *DescDamage(float dam);
char *DescRatio(float f);  /* theirs / yours */
char *DamRollDesc(int a);
char *HitRollDesc(int a);
char *ArmorDesc(int a);
char *AlignDesc(int a);
char *DescAttacks(float a);

long CalcPowerLevel(struct char_data *ch);
char *PowerLevelDesc(long a);

int singular( struct obj_data *o)
{

  if (IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_HANDS) ||
      IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_FEET) ||
      IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_LEGS) ||
      IS_SET(o->obj_flags.wear_flags, ITEM_WEAR_ARMS))
    return(FALSE);
  return(TRUE);
}

/* Procedures related to 'look' */

void argument_split_2(char *argument, char *first_arg, char *second_arg) {
  int look_at, begin;
  begin = 0;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of first word */
  for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to first_arg */
    *(first_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(first_arg + look_at) = '\0';
  begin += look_at;

  /* Find first non blank */
  for ( ;*(argument + begin ) == ' ' ; begin++);

  /* Find length of second word */
  for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

    /* Make all letters lower case, AND copy them to second_arg */
    *(second_arg + look_at) = LOWER(*(argument + begin + look_at));
  *(second_arg + look_at)='\0';
  begin += look_at;
}

struct obj_data *get_object_in_equip_vis(struct char_data *ch,
		 char *arg, struct obj_data *equipment[], int *j) {

  for ((*j) = 0; (*j) < MAX_WEAR ; (*j)++)
    if (equipment[(*j)])
      if (CAN_SEE_OBJ(ch,equipment[(*j)]))
	if (isname(arg, equipment[(*j)]->name))
	  return(equipment[(*j)]);

  return (0);
}

char *find_ex_description(char *word, struct extra_descr_data *list)
{
  struct extra_descr_data *i;

  for (i = list; i; i = i->next)
if (*word && i->keyword)
    if (isname(word,i->keyword))
      return(i->description);

  return(0);
}


void show_obj_to_char(struct obj_data *object, struct char_data *ch, int mode)
{

    char buffer[MAX_STRING_LENGTH];
  buffer[0] = 0;
  if ((mode == 0) && object->description)
    strcpy(buffer, object->description);
  else if (object->short_description && ((mode == 1) ||
					 (mode == 2) || (mode==3) || (mode == 4)))
    strcpy(buffer,object->short_description);
  else if (mode == 5)   {
    if (object->obj_flags.type_flag == ITEM_NOTE)    {
      if (object->action_description)          {
	strcpy(buffer, "There is something written upon it:\n\r\n\r");
	strcat(buffer, object->action_description);
	page_string(ch->desc, buffer, 1);
      }  else       {
	act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      }
       return;  /* mail fix, thanks brett */

    } else
    if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }

  if (mode != 3) {
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"(invisible)");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD) && IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
      if (singular(object))
			strcat(buffer,"..It glows red");
      else
			strcat(buffer,"..They glow red");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_EVIL) && IS_AFFECTED2(ch,AFF2_DETECT_GOOD)) {
      if (singular(object))
			strcat(buffer,"..It glows white");
      else
			strcat(buffer,"..They glow white");
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      if (singular(object))
	strcat(buffer,"..It glows blue");
      else
	strcat(buffer,"..They glow blue");
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      if (singular(object))
	strcat(buffer,"..It glows softly");
      else
	strcat(buffer,"..They glow softly");
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      if (singular(object))
      strcat(buffer,"..It hums powerfully");
      else
	strcat(buffer,"..They hum with power");
    }
/* Commented out for now.. no more container fullness -Lennya 20030408*/
//	if (singular(object) && object->obj_flags.type_flag == ITEM_CONTAINER) {
//		if (!object->obj_flags.value[2] && !object->obj_flags.value[3]) { /* check for ShowFull, IsCorpse */
//			for(i=object->contains;i;i=i->next_content) {
//				weight +=(float)i->obj_flags.weight;
//			}
//			/*calculate how much stuff is in this bag*/
//			fullperc = ((float) weight / ((float) object->obj_flags.value[0] -((float)object->obj_flags.weight - weight)-1));
//			/* 0% <= fullperc < 5  Empty*/
//			if(fullperc < .05)
//				strcat(buffer, "..It is empty");
//			/* 5 <= fullperc < 20 Almost Empty*/
//	else 	if(fullperc < .2)
//				strcat(buffer, "..It is almost empty");
//			/* 20 <= fullperc < 40 Less than Half*/
//	else 	if(fullperc < .4)
//				strcat(buffer, "..Its less than half full");
//			/* 40 <= fullperc < 60 About Half*/
//	else 	if(fullperc < .6)
//				strcat(buffer, "..Its about half full");
//			/* 60 <= fullperc < 80 More than half*/
//	else 	if(fullperc < .8)
//				strcat(buffer, "..Its more than half full");
//			/* 80 <= fullperc < 95 Almost full*/
//	else 	if(fullperc < .95)
//				strcat(buffer, "..It is almost full");
//			/* 95 <= fullperc < 100 Full*/
//	else 	if(fullperc >= .95)
//				strcat(buffer, "..It is full");
//		}
//	}
    if (object->obj_flags.type_flag == ITEM_ARMOR) {
      if (object->obj_flags.value[0] <
	  (object->obj_flags.value[1] / 4)) {
	if (singular(object))
	  strcat(buffer, "..It is falling apart");
	else
	  strcat(buffer,"..They are falling apart");
      } else if (object->obj_flags.value[0] <
		 (object->obj_flags.value[1] / 3)) {
	if (singular(object))
	  strcat(buffer, "..It is need of much repair.");
	else
	  strcat(buffer,"..They are in need of much repair");
      } else if (object->obj_flags.value[0] <
		 (object->obj_flags.value[1] / 2)) {
	if (singular(object))
	  strcat(buffer, "..It is in fair condition");
	else
	  strcat(buffer,"..They are in fair condition");
      } else if  (object->obj_flags.value[0] <
		  object->obj_flags.value[1]) {
	if (singular(object))
	  strcat(buffer, "..It is in good condition");
	else
	  strcat(buffer,"..They are in good condition");
      } else {
	if (singular(object))
	  strcat(buffer, "..It is in excellent condition");
	else
	  strcat(buffer,"..They are in excellent condition");
      }
    }
  }


  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);

}

void show_mult_obj_to_char(struct obj_data *object, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];

  buffer[0] = 0;
  tmp[0] = 0;

  if ((mode == 0) && object->description)
    strcpy(buffer,object->description);
  else  if (object->short_description && ((mode == 1) ||
					  (mode == 2) || (mode==3) || (mode == 4)))
    strcpy(buffer,object->short_description);
  else if (mode == 5) {
    if (object->obj_flags.type_flag == ITEM_NOTE)       {
      if (object->action_description)    {
	strcpy(buffer, "There is something written upon it:\n\r\n\r");
	strcat(buffer, object->action_description);
	page_string(ch->desc, buffer, 1);
      }  else
	act("It's blank.", FALSE, ch,0,0,TO_CHAR);
      return;
    } else if((object->obj_flags.type_flag != ITEM_DRINKCON)) {
      strcpy(buffer,"You see nothing special..");
    }  else  { /* ITEM_TYPE == ITEM_DRINKCON */
      strcpy(buffer, "It looks like a drink container.");
    }
  }

  if (mode != 3) {
    if (IS_OBJ_STAT(object,ITEM_INVISIBLE)) {
      strcat(buffer,"(invisible)");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_GOOD) && IS_AFFECTED(ch,AFF_DETECT_EVIL)) {
      strcat(buffer,"..It glows red!");
    }
    if (IS_OBJ_STAT(object,ITEM_ANTI_EVIL) && IS_AFFECTED2(ch,AFF2_DETECT_GOOD)) {
      strcat(buffer,"..It glows white!");
    }
    if (IS_OBJ_STAT(object,ITEM_MAGIC) && IS_AFFECTED(ch,AFF_DETECT_MAGIC)) {
      strcat(buffer,"..It glows blue!");
    }
    if (IS_OBJ_STAT(object,ITEM_GLOW)) {
      strcat(buffer,"..It has a soft glowing aura!");
    }
    if (IS_OBJ_STAT(object,ITEM_HUM)) {
      strcat(buffer,"..It emits a faint humming sound!");
    }
  }

  if (num>1) {
    sprintf(tmp,"[%d]", num);
    strcat(buffer, tmp);
  }
  strcat(buffer, "\n\r");
  page_string(ch->desc, buffer, 1);
}

void list_obj_in_room(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int Inventory_Num = 1;
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];

  cond_top = 0;

  for (i=list; i; i = i->next_content) {
    if (CAN_SEE_OBJ(ch, i)) {
      if (cond_top< 50) {
	found = FALSE;
	for (k=0;(k<cond_top&& !found);k++) {
	  if (cond_top>0) {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(i->description && cond_ptr[k]->description &&
		 !strcmp(i->description,cond_ptr[k]->description))){
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	if ((ITEM_TYPE(i) == ITEM_TRAP) || (GET_TRAP_CHARGES(i) > 0))
	{
	 if (CAN_SEE_OBJ(ch,i))
	   {
	     show_obj_to_char(i,ch,0);
	   }
	} /* not a trap */
	 else
	{
	  show_obj_to_char(i,ch,0);
	}
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if ((ITEM_TYPE(cond_ptr[k]) == ITEM_TRAP) && (GET_TRAP_CHARGES(cond_ptr[k]) > 0))  {
		if (CAN_SEE_OBJ(ch,cond_ptr[k])){
	  		if (cond_tot[k] > 1) {
	  			sprintf(buf,"[%2d] ",Inventory_Num++);
	    		send_to_char(buf,ch);
	    		show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
	  		} else {
	    		show_obj_to_char(cond_ptr[k],ch,0);
	  		}
	  	 }
      	} else {
			if (cond_tot[k] > 1) {
				sprintf(buf,"[%2d] ",Inventory_Num++);
	  			send_to_char(buf,ch);
	  			show_mult_obj_to_char(cond_ptr[k],ch,0,cond_tot[k]);
			} else {
	  			show_obj_to_char(cond_ptr[k],ch,0);
			}
      	}
    }
  }
}


void list_obj_in_heap(struct obj_data *list, struct char_data *ch)
{
  struct obj_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;
  char buf[MAX_STRING_LENGTH];

  int Num_Inventory = 1;
  cond_top = 0;

  for (i=list; i; i = i->next_content)
  {
    if (CAN_SEE_OBJ(ch, i))
    {
      if (cond_top< 50)
      {
	found = FALSE;
	for (k=0;(k<cond_top&& !found);k++)
	{
	  if (cond_top>0)
	  {
	    if ((i->item_number == cond_ptr[k]->item_number) &&
		(i->short_description && cond_ptr[k]->short_description &&
		 (!strcmp(i->short_description,cond_ptr[k]->short_description))))
		 {
	      cond_tot[k] += 1;
	      found=TRUE;
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	show_obj_to_char(i,ch,2);
      }
    } /* else can't see */
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      sprintf(buf,"[%2d] ",Num_Inventory++);
      send_to_char(buf,ch);
      if (cond_tot[k] > 1) {
	Num_Inventory += cond_tot[k] - 1;
	show_mult_obj_to_char(cond_ptr[k],ch,2,cond_tot[k]);
      } else {
	show_obj_to_char(cond_ptr[k],ch,2);
      }
    }
  }
}

void list_obj_to_char(struct obj_data *list,struct char_data *ch, int mode,
		      bool show) {
  char buf[MAX_STRING_LENGTH];
  int Num_In_Bag = 1;
  struct obj_data *i;
  bool found;

  found = FALSE;
  for ( i = list ; i ; i = i->next_content ) {
    if (CAN_SEE_OBJ(ch,i)) {
      sprintf(buf,"[%2d] ",Num_In_Bag++);
      send_to_char(buf,ch);
      show_obj_to_char(i, ch, mode);
      found = TRUE;
    }
  }
  if ((! found) && (show)) send_to_char("Nothing\n\r", ch);
}



void show_char_to_char(struct char_data *i, struct char_data *ch, int mode)
{

  char buffer[MAX_STRING_LENGTH];
  int j, found, percent, otype;
  struct obj_data *tmp_obj;
  struct affected_type *aff;

if (!ch || !i) {
   log("!ch || !i in act.info.c show_char_to_char");
   return;
  }

  if (mode == 0) {

	/* Don't show linkdead imms to mortals, messy stuff   -Lennya */
	if(!IS_NPC(i) && IS_IMMORTAL(i) && IS_LINKDEAD(i) && !IS_IMMORTAL(ch))
		return;

    if ((IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) && !IS_IMMORTAL(ch)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i)) {
		send_to_char("You sense a hidden life form in the room.\n\r", ch);
       return;
      } else {
      /* no see nothing */
       return;
      }
    }
	//sprintf(buffer,""); lets see if below is more effective.
	*buffer = '\0';
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos.*/
      if (!IS_NPC(i)) {
		//if (!GET_TITLE(i)) {
		  //strcpy(buffer,GET_NAME(i));
			//strcat(buffer," ");
		//}
		if (GET_TITLE(i))
		  strcat(buffer,GET_TITLE(i));
    	  } else {
				strcpy(buffer, i->player.short_descr);
				sprintf(buffer,"%s",CAP(buffer));
    	    }

	if(IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch))
	  strcat(buffer," (Hiding)");
	if ( IS_AFFECTED(i,AFF_INVISIBLE) || i->invis_level == LOW_IMMORTAL)
	  strcat(buffer," (invisible)");
    if ( IS_AFFECTED(i,AFF_CHARM))
	  strcat(buffer," (pet)$c0007");

      switch(GET_POS(i)) {
      case POSITION_STUNNED  :
	strcat(buffer," is lying here, stunned."); break;
      case POSITION_INCAP    :
	strcat(buffer," is lying here, incapacitated."); break;
      case POSITION_MORTALLYW:
	strcat(buffer," is lying here, mortally wounded."); break;
      case POSITION_DEAD     :
	strcat(buffer," is lying here, dead."); break;
      case POSITION_MOUNTED:
	if (MOUNTED(i)) {
	  strcat(buffer, " is here, riding ");
	  strcat(buffer, MOUNTED(i)->player.short_descr);
	} else {

	  strcat(buffer, " is standing here.");
	}
	break;
      case POSITION_STANDING :
	if (!IS_AFFECTED(i, AFF_FLYING) && !affected_by_spell(i,SKILL_LEVITATION)) {
	  if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	    strcat(buffer, "is floating here.");
	  else
	    strcat(buffer," is standing here.");
	} else {
	  strcat(buffer," is flying about.");
	}
	break;
      case POSITION_SITTING  :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is floating here.");
	else
	  strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is resting here in the water.");
	else
	strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is sleeping here in the water.");
	else
	strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {

	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air.");
	break;
	default : strcat(buffer," is floating here."); break;
      }

	if (IS_AFFECTED2(i,AFF2_AFK))
		strcat(buffer,"$c0006 (AFK)$c0007");

//  this may lag a bit, commented out for now  -Lennya
//	if (IS_AFFECTED2(i,AFF2_QUEST))
//		strcat(buffer,"$c0008 ($c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008)$c0007");

	if (IS_LINKDEAD(i))
		strcat(buffer,"$c0015 (Linkdead)$c0007");

	if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_EVIL(i))
	  		strcat(buffer, "$c0009 (Red Aura)");
   }

	if (IS_AFFECTED2(ch, AFF2_DETECT_GOOD)) {
		if (IS_GOOD(i))
   		strcat(buffer, "$c0015 (White Aura)");
   }

	act(buffer,FALSE, ch,0,0,TO_CHAR);
    } else {  /* npc with long */
		sprintf(buffer,"%s",(i->player.long_descr));
// lennyatest
	while ((buffer[strlen(buffer)-1]=='\r') ||
	       (buffer[strlen(buffer)-1]=='\n') ||
	       (buffer[strlen(buffer)-1]==' ')) {
	  buffer[strlen(buffer)-1] = '\0';
	}
		if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch))
			strcat(buffer," (Hiding)");
		if (IS_AFFECTED(i,AFF_INVISIBLE) || i->invis_level == LOW_IMMORTAL)
			strcat(buffer," (invisible)");
		if (IS_AFFECTED(i,AFF_CHARM))
			strcat(buffer," (pet)$c0007");
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			if (IS_EVIL(i))
	  			strcat(buffer, "$c0009 (Red Aura)$c0007");
		}
		if (IS_AFFECTED2(ch, AFF2_DETECT_GOOD)) {
			if (IS_GOOD(i))
				strcat(buffer, "$c0015 (White Aura)$c0007");
		}
		if (IS_AFFECTED2(i,AFF2_AFK))
			strcat(buffer,"$c0006 (AFK)$c0007");
		if (IS_LINKDEAD(i))
			strcat(buffer,"$c0015 (Linkdead)$c0007");
		/* strip \n\r's off */
	while ((buffer[strlen(buffer)-1]=='\r') ||
	       (buffer[strlen(buffer)-1]=='\n') ||
	       (buffer[strlen(buffer)-1]==' ')) {
	  buffer[strlen(buffer)-1] = '\0';
	}
      act(buffer,FALSE, ch,0,0,TO_CHAR);
    }

 if (IS_AFFECTED(i,AFF_SANCTUARY)) {
   if (!IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0015$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
   }


    if (IS_AFFECTED(i,AFF_GROWTH))
      act("$c0003$n is extremely large!", FALSE, i, 0, ch, TO_VICT);

    if (IS_AFFECTED(i, AFF_FIRESHIELD)) {
   if (!IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0001$n is surrounded by burning flames!", FALSE, i, 0, ch, TO_VICT);
   }

   if  (IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0008$n is surrounded by darkness!", FALSE, i, 0, ch, TO_VICT);

  } else if (mode == 1) {

    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

/*
  personal descriptions.
*/

    if (IS_PC(i)) {
	sprintf(buffer, "$n is %s", RaceName[GET_RACE(i)]);
	act(buffer, FALSE, i, 0, ch, TO_VICT);
    }

    if (MOUNTED(i)) {
      sprintf(buffer,"$n is mounted on %s", MOUNTED(i)->player.short_descr);
      act(buffer, FALSE, i, 0, ch, TO_VICT);
    }

    if (RIDDEN(i)) {
      sprintf(buffer,"$n is ridden by %s", IS_NPC(RIDDEN(i))?RIDDEN(i)->player.short_descr:GET_NAME(RIDDEN(i)));
      act(buffer, FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and many bruises.");
    else if (percent >= 50)
      strcat(buffer, " is wounded, and bleeding.");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.");
    else if (percent >= 15)
      strcat(buffer, " is badly wounded");
    else if (percent >= 0)
      strcat(buffer, " $c0001is in an awful condition.");
    else
      strcat(buffer, " $c0009is bleeding badly from large, gaping wounds.");

    act(buffer,FALSE, ch,0,0,TO_CHAR);


/*
  spell_descriptions, etc.
*/
    for (aff = i->affected; aff; aff = aff->next) {
      if (aff->type < MAX_EXIST_SPELL) {
	otype = -1;
	if (spell_desc[aff->type] && *spell_desc[aff->type])
	  if (aff->type != otype) {
	    act(spell_desc[aff->type], FALSE, i, 0, ch, TO_VICT);
	    otype = aff->type;
	  }
      }
    }


    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      if (IS_SET(i->player.user_flags, CLOAKED) && i->equipment[WEAR_ABOUT]) {
	act("$p covers much of $n's body.",FALSE,i,i->equipment[WEAR_ABOUT],ch,TO_VICT);
      }
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);

      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (IS_SET(i->player.user_flags, CLOAKED) && i->equipment[WEAR_ABOUT] &&
	      !(j==WEAR_LIGHT || j==WEAR_HEAD || j==WEAR_HANDS ||
		j==WEAR_SHIELD || j==WEAR_ABOUT || j==WEAR_EAR_R || j==WEAR_EAR_L ||
		j==WEAR_EYES) && !IS_OBJ_STAT(i->equipment[WEAR_ABOUT],ITEM_INVISIBLE) /* see through cloak  -Lennya */
	      && !IS_IMMORTAL(ch) && (i!=ch) ) {
	    /* Do nothing */
	  } else
	    if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	      send_to_char(where[j],ch);
	      show_obj_to_char(i->equipment[j],ch,1);
	    }
	}
      }
    }
    if (HasClass(ch, CLASS_THIEF) && (ch != i) &&
	(!IS_IMMORTAL(ch))){
      found = FALSE;
      send_to_char
	("\n\rYou attempt to peek at the inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj;
	  tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch, tmp_obj) &&
	    (number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n\r", ch);
    } else if (IS_IMMORTAL(ch)) {
      send_to_char("Inventory:\n\r",ch);
      for(tmp_obj = i->carrying; tmp_obj;
	  tmp_obj = tmp_obj->next_content) {
	show_obj_to_char(tmp_obj, ch, 1);
	found = TRUE;
      }
      if (!found) {
	send_to_char("Nothing\n\r",ch);
      }
    }

  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_in_heap(i->carrying,ch);
  }
}

void glance_at_char(struct char_data *i, struct char_data *ch)
{
  char buffer[MAX_STRING_LENGTH];
  int otype, percent ;
  struct affected_type *aff;

if (!ch || !i) {
   log("!ch || !i in act.info.c show_char_to_char");
   return;
  }
    /* Show a character to another */

if (GET_MAX_HIT(i) > 0)
  percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
else
  percent = -1; /* How could MAX_HIT be < 1?? */

if (IS_NPC(i))
  strcpy(buffer, i->player.short_descr);
else
  strcpy(buffer, GET_NAME(i));

if (percent >= 100)
  strcat(buffer, " is in an excellent condition.");
else if (percent >= 90)
  strcat(buffer, " has a few scratches.");
else if (percent >= 75)
  strcat(buffer, " has some small wounds and many bruises.");
else if (percent >= 50)
  strcat(buffer, " is wounded, and bleeding.");
else if (percent >= 30)
  strcat(buffer, " has some big nasty wounds and scratches.");
else if (percent >= 15)
  strcat(buffer, " is badly wounded");
else if (percent >= 0)
  strcat(buffer, " $c0001is in an awful condition.");
else
  strcat(buffer, " $c0009is bleeding badly from large, gaping wounds.");

act(buffer,FALSE, ch,0,0,TO_CHAR);


/*
  spell_descriptions, etc.
*/

for (aff = i->affected; aff; aff = aff->next) {
  if (aff->type < MAX_EXIST_SPELL) {
     otype = -1;
        if (spell_desc[aff->type] && *spell_desc[aff->type])
	  if (aff->type != otype) {
	    act(spell_desc[aff->type], FALSE, i, 0, ch, TO_VICT);
	    otype = aff->type;
	  }
      }
   }
}

void show_mult_char_to_char(struct char_data *i, struct char_data *ch, int mode, int num)
{
  char buffer[MAX_STRING_LENGTH];
  char tmp[10];
  int j, found, percent;
  struct obj_data *tmp_obj;

	/* Don't show linkdead imms to mortals. Should never be multiple, but hey..   -Lennya */
	if(!IS_NPC(i) && IS_IMMORTAL(i) && IS_LINKDEAD(i) && !IS_IMMORTAL(ch))
		return;

  if (mode == 0) {
    if ((IS_AFFECTED(i, AFF_HIDE) || !CAN_SEE(ch,i)) && !IS_IMMORTAL(ch)) {
      if (IS_AFFECTED(ch, AFF_SENSE_LIFE) && !IS_IMMORTAL(i)) {
	    if (num==1) /* yer alone, what you doin in this func boy?  -Lennya */
	      act("You sense a hidden life form in the room.",FALSE, ch,0,0,TO_CHAR);
	    else
	      act("$c0002You sense a hidden life form in the room.",FALSE, ch,0,0,TO_CHAR);
        return;
      } else {
        /* no see nothing */
        return;
      }
    }
	//lets see if below is more effective...sprintf(buffer,"");
	*buffer = '\0';
    if (!(i->player.long_descr)||(GET_POS(i) != i->specials.default_pos)){
      /* A player char or a mobile without long descr, or not in default pos. */
      if (!IS_NPC(i)) {
	//strcpy(buffer,GET_NAME(i));
	//strcat(buffer," ");
	if (GET_TITLE(i))
	  strcat(buffer,GET_TITLE(i));
      } else {
	strcpy(buffer, i->player.short_descr);
	sprintf(buffer,"%s",CAP(buffer));
      }

	if(IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch))
	  strcat(buffer," (Hiding)");
	if ( IS_AFFECTED(i,AFF_INVISIBLE) || i->invis_level == LOW_IMMORTAL)
	  strcat(buffer," (invisible)");
    if ( IS_AFFECTED(i,AFF_CHARM))
	  strcat(buffer," (pet)$c0007");

      switch(GET_POS(i)) {
      case POSITION_STUNNED  :
	strcat(buffer,"$c0005 is lying here, stunned."); break;
      case POSITION_INCAP    :
	strcat(buffer,"$c0006 is lying here, incapacitated."); break;
      case POSITION_MORTALLYW:
	strcat(buffer,"$c0009 is lying here, mortally wounded."); break;
      case POSITION_DEAD     :
	strcat(buffer," is lying here, dead."); break;
      case POSITION_STANDING :
	if (!IS_AFFECTED(i, AFF_FLYING) && !affected_by_spell(i,SKILL_LEVITATION)) {
	  if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	    strcat(buffer, "is floating here.");
	  else
	    strcat(buffer," is standing here.");
	} else {
	  strcat(buffer," is flying about.");
	}
	break;
      case POSITION_SITTING  :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is floating here.");
	else
	  strcat(buffer," is sitting here.");  break;
      case POSITION_RESTING  :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is resting here in the water");
	else
	  strcat(buffer," is resting here.");  break;
      case POSITION_SLEEPING :
	if (real_roomp(i->in_room)->sector_type == SECT_WATER_NOSWIM)
	  strcat(buffer, "is sleeping here in the water");
	else
	  strcat(buffer," is sleeping here."); break;
      case POSITION_FIGHTING :
	if (i->specials.fighting) {

	  strcat(buffer," is here, fighting ");
	  if (i->specials.fighting == ch)
	    strcat(buffer," YOU!");
	  else {
	    if (i->in_room == i->specials.fighting->in_room)
	      if (IS_NPC(i->specials.fighting))
		strcat(buffer, i->specials.fighting->player.short_descr);
	      else
		strcat(buffer, GET_NAME(i->specials.fighting));
	    else
	      strcat(buffer, "someone who has already left.");
	  }
	} else /* NIL fighting pointer */
	  strcat(buffer," is here struggling with thin air.");
	break;
	default : strcat(buffer,"$c0006 is floating here."); break;
      }
   if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
		if (IS_EVIL(i))
	  		strcat(buffer, "$c0009 (Red Aura)");
   }

   if (IS_AFFECTED2(ch, AFF2_DETECT_GOOD)) {
   	if (IS_GOOD(i))
      	strcat(buffer, "$c0015 (White Aura)");
   }

if (IS_AFFECTED2(i,AFF2_AFK))
	strcat(buffer,"$c0006 (AFK)$c0007");

//  this may lag a bit, commented out for now  -Lennya
//	if (IS_AFFECTED2(i,AFF2_QUEST))
//		strcat(buffer,"$c0008 ($c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008)$c0007");

if (IS_LINKDEAD(i))
	strcat(buffer,"$c0015 (Linkdead)$c0007");

      if (num > 1) {
	sprintf(tmp," [%d]", num);
	strcat(buffer, tmp);
      }
      act(buffer,FALSE, ch,0,0,TO_CHAR);
    } else {  /* npc with long */
		sprintf(buffer,"%s",(i->player.long_descr));
// lennyatest
	while ((buffer[strlen(buffer)-1]=='\r') ||
	       (buffer[strlen(buffer)-1]=='\n') ||
	       (buffer[strlen(buffer)-1]==' ')) {
	  buffer[strlen(buffer)-1] = '\0';
	}
		if (IS_AFFECTED(i, AFF_HIDE) && IS_IMMORTAL(ch))
			strcat(buffer," (Hiding)");
		if (IS_AFFECTED(i,AFF_INVISIBLE) || i->invis_level == LOW_IMMORTAL)
			strcat(buffer," (invisible)");
		if (IS_AFFECTED(i,AFF_CHARM))
			strcat(buffer," (pet)$c0007");
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)) {
			if (IS_EVIL(i))
	  			strcat(buffer, "$c0009 (Red Aura)$c0007");
		}
		if (IS_AFFECTED2(ch, AFF2_DETECT_GOOD)) {
			if (IS_GOOD(i))
				strcat(buffer, "$c0015 (White Aura)$c0007");
		}
		if (IS_AFFECTED2(i,AFF2_AFK))
			strcat(buffer,"$c0006 (AFK)$c0007");
//  this may lag a bit, commented out for now  -Lennya
//	if (IS_AFFECTED2(i,AFF2_QUEST))
//		strcat(buffer,"$c0008 ($c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008)$c0007");
		if (IS_LINKDEAD(i))
			strcat(buffer,"$c0015 (Linkdead)$c0007");
      /* this gets a little annoying */

      if (num > 1) {
	while ((buffer[strlen(buffer)-1]=='\r') ||
	       (buffer[strlen(buffer)-1]=='\n') ||
	       (buffer[strlen(buffer)-1]==' ')) {
	  buffer[strlen(buffer)-1] = '\0';
	}
	sprintf(tmp," [%d]", num);
	strcat(buffer, tmp);
      }

      act(buffer,FALSE, ch,0,0,TO_CHAR);
    }

 if (IS_AFFECTED(i,AFF_SANCTUARY)) {
   if (!IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0015$n glows with a bright light!", FALSE, i, 0, ch, TO_VICT);
   }

    if (IS_AFFECTED(i,AFF_GROWTH))
      act("$c0003$n is extremely large!", FALSE, i, 0, ch, TO_VICT);

    if (IS_AFFECTED(i, AFF_FIRESHIELD)) {
   if (!IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0001$n is surrounded by burning flames!", FALSE, i, 0, ch, TO_VICT);
   }

    if (IS_AFFECTED2(i,AFF2_DARKNESS))
      act("$c0008$n is surround by darkness!", FALSE, i, 0, ch, TO_VICT);

  } else if (mode == 1) {

    if (i->player.description)
      send_to_char(i->player.description, ch);
    else {
      act("You see nothing special about $m.", FALSE, i, 0, ch, TO_VICT);
    }

    /* Show a character to another */

    if (GET_MAX_HIT(i) > 0)
      percent = (100*GET_HIT(i))/GET_MAX_HIT(i);
    else
      percent = -1; /* How could MAX_HIT be < 1?? */

    if (IS_NPC(i))
      strcpy(buffer, i->player.short_descr);
    else
      strcpy(buffer, GET_NAME(i));

    if (percent >= 100)
      strcat(buffer, " is in an excellent condition.\n\r");
    else if (percent >= 90)
      strcat(buffer, " has a few scratches.\n\r");
    else if (percent >= 75)
      strcat(buffer, " has some small wounds and bruises.\n\r");
    else if (percent >= 50)
      strcat(buffer, " has quite a few wounds.\n\r");
    else if (percent >= 30)
      strcat(buffer, " has some big nasty wounds and scratches.\n\r");
    else if (percent >= 15)
      strcat(buffer, " looks pretty hurt.\n\r");
    else if (percent >= 0)
      strcat(buffer, " $c0001is in an awful condition.\n\r");
    else
      strcat(buffer, " $c0009is bleeding awfully from big wounds.\n\r");

    act(buffer,FALSE, ch,0,0,TO_CHAR);

    found = FALSE;
    for (j=0; j< MAX_WEAR; j++) {
      if (i->equipment[j]) {
	if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	  found = TRUE;
	}
      }
    }
    if (found) {
      act("\n\r$n is using:", FALSE, i, 0, ch, TO_VICT);

      for (j=0; j< MAX_WEAR; j++) {
	if (i->equipment[j]) {
	  if (CAN_SEE_OBJ(ch,i->equipment[j])) {
	    send_to_char(where[j],ch);
	    show_obj_to_char(i->equipment[j],ch,1);
	  }
	}
      }
    }
    if ((HasClass(ch, CLASS_THIEF)) && (ch != i)) {
      found = FALSE;
      send_to_char("\n\rYou attempt to peek at the inventory:\n\r", ch);
      for(tmp_obj = i->carrying; tmp_obj; tmp_obj = tmp_obj->next_content) {
	if (CAN_SEE_OBJ(ch,tmp_obj)&&(number(0,MAX_MORT) < GetMaxLevel(ch))) {
	  show_obj_to_char(tmp_obj, ch, 1);
	  found = TRUE;
	}
      }
      if (!found)
	send_to_char("You can't see anything.\n\r", ch);
    }

  } else if (mode == 2) {

    /* Lists inventory */
    act("$n is carrying:", FALSE, i, 0, ch, TO_VICT);
    list_obj_in_heap(i->carrying,ch);
  }
}


void list_char_in_room(struct char_data *list, struct char_data *ch)
{
  struct char_data *i, *cond_ptr[50];
  int k, cond_top, cond_tot[50], found=FALSE;

  cond_top = 0;

  for (i=list; i; i = i->next_in_room) {
    if ( (ch!=i) && (!RIDDEN(i)) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		     (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) ) {
      if ((cond_top< 50) && !MOUNTED(i)) {
	found = FALSE;
	if (IS_NPC(i)) {
	  for (k=0;(k<cond_top&& !found);k++) {
	    if (cond_top>0) {
	      if (i->nr == cond_ptr[k]->nr &&
		  (GET_POS(i) == GET_POS(cond_ptr[k])) &&
		  (i->specials.affected_by==cond_ptr[k]->specials.affected_by) &&
		  (i->specials.fighting == cond_ptr[k]->specials.fighting) &&
		  (i->player.short_descr && cond_ptr[k]->player.short_descr &&
		   0==strcmp(i->player.short_descr,cond_ptr[k]->player.short_descr))) {
		cond_tot[k] += 1;
		found=TRUE;
	      }
	    }
	  }
	}
	if (!found) {
	  cond_ptr[cond_top] = i;
	  cond_tot[cond_top] = 1;
	  cond_top+=1;
	}
      } else {
	show_char_to_char(i,ch,0);
      }
    }
  }

  if (cond_top) {
    for (k=0; k<cond_top; k++) {
      if (cond_tot[k] > 1) {
	show_mult_char_to_char(cond_ptr[k],ch,0,cond_tot[k]);
      } else {
	show_char_to_char(cond_ptr[k],ch,0);
      }
    }
  }
}


void list_char_to_char(struct char_data *list, struct char_data *ch,
		       int mode) {
  struct char_data *i;

  for (i = list; i ; i = i->next_in_room) {
    if ( (ch!=i) && (IS_AFFECTED(ch, AFF_SENSE_LIFE) ||
		     (CAN_SEE(ch,i) && !IS_AFFECTED(i, AFF_HIDE))) )
      show_char_to_char(i,ch,0);
  }
}

/* Added by Mike Wilson 9/23/93 */

void list_exits_in_room(struct char_data *ch)
{
  int door,seeit=FALSE;
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  struct room_direction_data    *exitdata;

  *buf = '\0';

  for (door = 0; door <= 5; door++)
  {
    exitdata = EXIT(ch,door);
    if (exitdata)
    {
      if (real_roomp(exitdata->to_room))
  {
  if (GET_RACE(ch) == RACE_MOON_ELF ||
      GET_RACE(ch) == RACE_GOLD_ELF ||
      GET_RACE(ch) == RACE_WILD_ELF ||
      GET_RACE(ch) == RACE_SEA_ELF  ||
      GET_RACE(ch) == RACE_AVARIEL  )   /* elves can see secret doors 1-2 on d6 */
      seeit=(number(1,6)<=2);
  else if (GET_RACE(ch)==RACE_HALF_ELF) /* half-elves can see at 1 */
	seeit=(number(1,6)<=1);
		else seeit=FALSE;

if (exitdata->to_room != NOWHERE || IS_IMMORTAL(ch))
  {
  if ((!IS_SET(exitdata->exit_info, EX_SECRET) || IS_IMMORTAL(ch)) ||
	 (IS_SET(exitdata->exit_info, EX_SECRET) && seeit) )
       {
	  sprintf(buf2," %s",listexits[door]);
	  strcat(buf,buf2);
	if (IS_SET(exitdata->exit_info, EX_CLOSED) /*&& IS_IMMORTAL(ch)*/)
	  strcat(buf, "$c0015 (closed)"); /*in white*/
	if (IS_SET(exitdata->exit_info, EX_SECRET) && (seeit ||
            IS_IMMORTAL(ch)))
	  strcat(buf, " $c0009(secret)$c0007"); /* in red */
       } /* exit */

  } /* ! = NOWHERE */

   }    /* real_roomp */
  } /* exitdata */
  } /* for */

  send_to_char("Exits:", ch);

  if (*buf)
      act(buf,FALSE, ch,0,0,TO_CHAR);
    else
      send_to_char("None!\n\r", ch);
}


void do_look(struct char_data *ch, char *argument, int cmd)
{

	void generate_map(struct char_data *ch, int size, int x, int y);
	void print_map(struct char_data *ch);
  char buffer[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH+80];
  char arg2[MAX_INPUT_LENGTH+80];
  int keyword_no, res;
  float fullperc;
  float weight = 0;
  int j, bits, temp;
  bool found;
  struct obj_data *tmp_object, *found_object, *i;
  struct char_data *tmp_char;
  char *tmp_desc;
  static char *keywords[]= {
    "north",
    "east",
    "south",
    "west",
    "up",
    "down",
    "in",
    "at",
    "",  /* Look at '' case */
    "room",
    "\n" };

dlog("in do_look");

  if (!ch->desc) {
    return;
    }

  if (GET_POS(ch) < POSITION_SLEEPING)
    send_to_char("You can't see anything but stars!\n\r", ch);
  else if (GET_POS(ch) == POSITION_SLEEPING)
    send_to_char("You can't see anything, you're sleeping!\n\r", ch);
  else if ( IS_AFFECTED(ch, AFF_BLIND) )
    send_to_char("You can't see a damn thing, you're blinded!\n\r", ch);
  else if  ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) &&
	    (!IS_AFFECTED(ch, AFF_TRUE_SIGHT))) {
    send_to_char("It is very dark in here.. Find a lightsource to see.\n\r", ch);
    if (IS_AFFECTED(ch, AFF_INFRAVISION)) {
      list_char_in_room(real_roomp(ch->in_room)->people, ch);
    }
  } else {

    only_argument(argument, arg1);

    if (0==strn_cmp(arg1,"at",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 7;
    } else if (0==strn_cmp(arg1,"in",2) && isspace(arg1[2])) {
      only_argument(argument+3, arg2);
      keyword_no = 6;
    } else {
      keyword_no = search_block(arg1, keywords, FALSE);
    }

    if ((keyword_no == -1) && *arg1) {
      keyword_no = 7;
      only_argument(argument, arg2);
    }


    found = FALSE;
    tmp_object = 0;
    tmp_char     = 0;
    tmp_desc     = 0;

    switch(keyword_no) {
      /* look <dir> */
    case 0 :
    case 1 :
    case 2 :
    case 3 :
    case 4 :
    case 5 : {
      struct room_direction_data        *exitp;
      exitp = EXIT(ch, keyword_no);
      if (exitp) {
	if (exitp->general_description) {
	  send_to_char(exitp-> general_description, ch);
	} else {
	  send_to_char("You see nothing special.\n\r", ch);
	}

	if (affected_by_spell(ch,SKILL_DANGER_SENSE)) {
	  struct room_data      *tmprp;
	  tmprp = real_roomp(exitp->to_room);
	    if (tmprp && IS_SET(tmprp->room_flags,DEATH))
	    send_to_char("You sense great dangers in that direction.\n\r",ch);
	 }

	if (IS_SET(exitp->exit_info, EX_CLOSED) &&
	    (exitp->keyword)) {
	   if ((strcmp(fname(exitp->keyword), "secret")) &&
	       (!IS_SET(exitp->exit_info, EX_SECRET))) {
	      sprintf(buffer, "The %s is closed.\n\r",
		    fname(exitp->keyword));
	      send_to_char(buffer, ch);
	    }
	 } else {
	   if (IS_SET(exitp->exit_info, EX_ISDOOR) &&
	      exitp->keyword) {
	      sprintf(buffer, "The %s is open.\n\r",
		      fname(exitp->keyword));
	      send_to_char(buffer, ch);
	    }
	 }
      } else {
	send_to_char("You see nothing special.\n\r", ch);
      }

		if (exitp && exitp->to_room && (!IS_SET(exitp->exit_info, EX_ISDOOR) ||
					(!IS_SET(exitp->exit_info, EX_CLOSED)))) {

			if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch)) {
				struct room_data      *rp;
				sprintf(buffer,"You look %swards.\n\r", dirs[keyword_no]);
				send_to_char(buffer, ch);

				sprintf(buffer,"$n looks %swards.", dirs[keyword_no]);
				act(buffer, FALSE, ch, 0, 0, TO_ROOM);

				rp = real_roomp(exitp->to_room);
				if (!rp) {
					send_to_char("You see swirling chaos.\n\r", ch);
				} else if(exitp) {
					/* NO_SPY flag on rooms, Lennya 20030602 */
					if (IS_SET(real_roomp(exitp->to_room)->room_flags, NO_SPY)) {
						int immcheck = 1;
						sprintf(buffer, "A strange magic blurs your vision as you attempt to look into %s.\n\r", rp->name);
						send_to_char(buffer,ch);
						/* imms should be able to see through the blur: */
						if (IS_IMMORTAL(ch))
							immcheck = 0;
						if (immcheck)
							return;
					}
					sprintf(buffer, "%ld look", exitp->to_room);
					do_at(ch, buffer, 0);
				} else {
					send_to_char("You see nothing special.\n\r", ch);
				}
			}
		}
	}
	break;

      /* look 'in'      */
	case 6: {
		if (*arg2) {
			/* Item carried */
			bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM | FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);
			if (bits) { /* Found something */
				if (GET_ITEM_TYPE(tmp_object)== ITEM_DRINKCON) {
					if (tmp_object->obj_flags.value[1] <= 0) {
						act("It is empty.", FALSE, ch, 0, 0, TO_CHAR);
					} else {
						temp=((tmp_object->obj_flags.value[1]*3)/tmp_object->obj_flags.value[0]);
						sprintf(buffer,"It's %sfull of a %s liquid.\n\r",fullness[temp],
											color_liquid[tmp_object->obj_flags.value[2]]);
						send_to_char(buffer, ch);
					}
				} else if (GET_ITEM_TYPE(tmp_object) == ITEM_CONTAINER) {
					if (!IS_SET(tmp_object->obj_flags.value[1],CONT_CLOSED)) {
						if (!IS_CORPSE(tmp_object)) {
							/* If it's not a corpse, calculate how much stuff is in this container */
							for (i=tmp_object->contains;i;i=i->next_content) {
								weight +=(float)i->obj_flags.weight;
							}
							fullperc = (((float) weight / ((float) tmp_object->obj_flags.value[0] -
										((float)tmp_object->obj_flags.weight - weight)-1)) * 100.0);
							sprintf(buffer, "%s %.0f%s full", fname(tmp_object->name),fullperc, "%");
						} else { /* it's a corpse - Lennya 20030320 */
							sprintf(buffer, "%s ", fname(tmp_object->name));
						}
						send_to_char(buffer, ch);
						switch (bits) {
							case FIND_OBJ_INV :
								send_to_char(" (carried) : \n\r", ch);
								break;
							case FIND_OBJ_ROOM :
								send_to_char(" (here) : \n\r", ch);
								break;
							case FIND_OBJ_EQUIP :
								send_to_char(" (used) : \n\r", ch);
								break;
						}
						list_obj_in_heap(tmp_object->contains, ch);
					} else {
						send_to_char("It is closed.\n\r", ch);
					}
				} else {
					send_to_char("That is not a container.\n\r", ch);
				}
			} else { /* wrong argument */
				send_to_char("You do not see that item here.\n\r", ch);
			}
		} else { /* no argument */
			send_to_char("Look in what?!\n\r", ch);
		}
	}
	break;

      /* look 'at'      */
    case 7 : {
      if (*arg2) {
	bits = generic_find(arg2, FIND_OBJ_INV | FIND_OBJ_ROOM |
			    FIND_OBJ_EQUIP | FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  show_char_to_char(tmp_char, ch, 1);
	  if (ch != tmp_char) {
	    act("$n looks at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n looks at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	}
	/*
	  Search for Extra Descriptions in room and items
	  */

	/* Extra description in room?? */
	if (!found) {
	  tmp_desc = find_ex_description(arg2,
					 real_roomp(ch->in_room)->ex_description);
	  if (tmp_desc) {
	    page_string(ch->desc, tmp_desc, 0);
	    return;
	  }
	}

	/* extra descriptions in items */

	/* Equipment Used */
	if (!found) {
	  for (j = 0; j< MAX_WEAR && !found; j++) {
	    if (ch->equipment[j]) {
	      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
		tmp_desc = find_ex_description(arg2, ch->equipment[j]->ex_description);

		if (tmp_desc) {
		  page_string(ch->desc, tmp_desc, 1);
		  found = TRUE;
		}

	      }
	    }
	  }
	}
	/* In inventory */
	if (!found) {
	  for(tmp_object = ch->carrying;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if (CAN_SEE_OBJ(ch, tmp_object)) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* Object In room */

	if (!found) {
	  for(tmp_object = real_roomp(ch->in_room)->contents;
	      tmp_object && !found;
	      tmp_object = tmp_object->next_content) {
	    if (CAN_SEE_OBJ(ch, tmp_object)) {
	      tmp_desc = find_ex_description(arg2,
					     tmp_object->ex_description);
	      if (tmp_desc) {
		page_string(ch->desc, tmp_desc, 1);
		found = TRUE;
	      }
	    }
	  }
	}
	/* wrong argument */
	if (bits) { /* If an object was found */
	  if (!found)
	    show_obj_to_char(found_object, ch, 5);
	  /* Show no-description */
	  else
	    show_obj_to_char(found_object, ch, 6);
	  /* Find hum, glow etc */
	} else if (!found) {
	  send_to_char("You do not see that here.\n\r", ch);
	}
      } else {
	/* no argument */
	send_to_char("Look at what?\n\r", ch);
      }
    }
      break;

      /* look ''                */
    case 8 : {
      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      if (!IS_SET(ch->specials.act, PLR_BRIEF))
	send_to_char(real_roomp(ch->in_room)->description, ch);

      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }

	if (ValidRoom(ch) == TRUE && !IS_SET(ch->specials.act, PLR_BRIEF)) {

		if(IS_SET(real_roomp(ch->in_room)->room_flags, ROOM_WILDERNESS)) {

		 	generate_map(ch, GET_RADIUS(ch), 3, 3); //char, radius, x, y
		 	print_map(ch);
		 }
	}
	list_exits_in_room(ch);
	list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
	list_char_in_room(real_roomp(ch->in_room)->people, ch);

    }
      break;

      /* wrong arg      */
    case -1 :
      send_to_char("Sorry, I didn't understand that!\n\r", ch);
      break;

      /* look 'room' */
    case 9 : {

      send_to_char(real_roomp(ch->in_room)->name, ch);
      send_to_char("\n\r", ch);
      send_to_char(real_roomp(ch->in_room)->description, ch);


      if (!IS_NPC(ch)) {
	if (IS_SET(ch->specials.act, PLR_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, PLR_HUNTING);
	  }
	}
      } else {
	if (IS_SET(ch->specials.act, ACT_HUNTING)) {
	  if (ch->specials.hunting) {
	    res = track(ch, ch->specials.hunting);
	    if (!res) {
	      ch->specials.hunting = 0;
	      ch->hunt_dist = 0;
	      REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	    }
	  } else {
	    ch->hunt_dist = 0;
	    REMOVE_BIT(ch->specials.act, ACT_HUNTING);
	  }
	}
      }
      list_exits_in_room(ch);
      list_obj_in_room(real_roomp(ch->in_room)->contents, ch);
      list_char_in_room(real_roomp(ch->in_room)->people, ch);

    }
      break;
    }
  }
}

int GET_RADIUS(struct char_data *ch) {
	int radius = 2;

	if ((IS_DARK(ch->in_room)) && (!IS_IMMORTAL(ch)) && (!IS_AFFECTED(ch, AFF_TRUE_SIGHT)))
         radius--;


	if (IS_AFFECTED(ch, AFF_SCRYING) || IS_IMMORTAL(ch))
		radius++;


	if ( IS_AFFECTED(ch, AFF_BLIND))
		radius=0;

	return radius;
}

/* end of look */
extern int map[7][7];

void print_map(struct char_data *ch) {
   extern const char *sector_char[];
   char buf[MAX_STRING_LENGTH];
 	int x,y;
 *buf=0;

strcat(buf," _______\n()______)\n");

	for(x=0;x<7;x++) {
		for(y=0;y<7;y++){
			if(y==0)
				strcat(buf,"$c000w|");
			if(x==3 && y==3)
				sprintf(buf + strlen(buf),"$c000RX");
			else
				sprintf(buf + strlen(buf),"%s", map[x][y]-1 < 0? "$c000w:":sector_char[map[x][y]-1]);

		}
		  strcat(buf, "$c000w|\n\r");
	}
	strcat(buf, "@______/\n\r");
	send_to_char(buf,ch);
}

/*

   ________
  ()_______)
  |::::::::|
  |::::::::|
  |::::::::|
  |::::::::|
  |::::::::|   n
  |::::::::| w-+-e
  @:::::::/    s
   ~~~~~~



*/

void generate_map(struct char_data *ch, int size, int x, int y) {

    void recurse_map(struct room_data *rp, int size, int x, int y);
	int a,b;
    struct room_data *rm;

	if(!IS_PC(ch))
		return;

	for(a=0;a<7; a++)
	  for(b=0;b<7; b++)
	    map[a][b]=0;

	rm=real_roomp(ch->in_room);
	if(!rm) {
		send_to_char("Error??",ch);
		return;

	}
	//if(ValidRoom(rm)==TRUE)
		recurse_map(rm, size, x, y);
	//else
	//	send_to_char("Error somewhere in generate map",ch);

}
void recurse_map(struct room_data *rp, int size, int x, int y) {
	struct room_data *rm;

//	if(map[x][y]!=0) //May wasn't printing out completely.
//		return;
	if(!rp) {
		map[x][y]=22;
		return;
	}

	map[x][y]=rp->sector_type+1;

	if(size==0) {
		return;
	}


	if(rp->dir_option[0]) {
		rm=real_roomp(rp->dir_option[0]->to_room);
	  	if(!rm)
	  	  return;
	  	recurse_map(rm , size-1, x-1, y);//north
	}

	if(rp->dir_option[1]) {
		rm=real_roomp(rp->dir_option[1]->to_room);
	  	if(!rm)
	  	  return;
	  	recurse_map(rm , size-1, x, y+1);//east
	}

	if(rp->dir_option[2]) {
		rm=real_roomp(rp->dir_option[2]->to_room);
	  	if(!rm)
	  	  return;
	  	recurse_map(rm , size-1, x+1, y);//south
	}

	if(rp->dir_option[3]) {
		rm=real_roomp(rp->dir_option[3]->to_room);
	  	if(!rm)
	  	  return;
	  	recurse_map(rm , size-1, x, y-1);//west
	}


}

 void do_read(struct char_data *ch, char *argument, int cmd)
   {
     char buf[100];
     dlog("in do_read");

  /* This is just for now - To be changed later.! */
  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);
}



 void do_examine(struct char_data *ch, char *argument, int cmd)
{
  char name[1000], buf[1000];
  struct char_data *tmp_char;
  struct obj_data *tmp_object;
dlog("in do_examine");

  sprintf(buf,"at %s",argument);
  do_look(ch,buf,15);

  one_argument(argument, name);

  if (!*name) {
    send_to_char("Examine what?\n\r", ch);
    return;
  }

  generic_find(name, FIND_OBJ_INV | FIND_OBJ_ROOM |
		      FIND_OBJ_EQUIP, ch, &tmp_char, &tmp_object);

  if (tmp_object) {
    if ((GET_ITEM_TYPE(tmp_object)==ITEM_DRINKCON) ||
	(GET_ITEM_TYPE(tmp_object)==ITEM_CONTAINER)) {
      send_to_char("When you look inside, you see:\n\r", ch);
      sprintf(buf,"in %s",argument);
      do_look(ch,buf,15);
    }
  }
}

#if 0
void do_exits(struct char_data *ch, char *argument, int cmd)
{
  int door;
  char buf[1000];
  struct room_direction_data    *exitdata;
dlog("in do_exits");
  *buf = '\0';

  for (door = 0; door <= 5; door++) {
    exitdata = EXIT(ch,door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room)) {
	/* don't print unless immortal */
	if (IS_IMMORTAL(ch)) {
	  sprintf(buf + strlen(buf), "%s - swirling chaos of #%d\n\r",
		  exits[door], exitdata->to_room);
	}
      } else if (exitdata->to_room != NOWHERE &&
		 (!IS_SET(exitdata->exit_info, EX_CLOSED) ||
		  IS_IMMORTAL(ch))) {
	if (IS_DARK(exitdata->to_room))
	  sprintf(buf + strlen(buf), "%s - Too dark to tell", exits[door]);
	else
	  sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	if (IS_SET(exitdata->exit_info, EX_CLOSED))
	  strcat(buf, " (closed)");
	strcat(buf, "\n\r");
      }
    }
  }

  send_to_char("Obvious exits:\n\r", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n\r", ch);
}
#else

/*  Gecko's spiffy enhancement to do_exits() from act.info.c */

void do_exits(struct char_data *ch, char *argument, int cmd)
{
  /* NOTE: Input var 'cmd' is not used. */
  int door;
  char buf[1000];
  struct room_direction_data *exitdata;

dlog("in do_exits2");

  *buf = '\0';

  for (door = 0; door <= 5; door++) {
    exitdata = EXIT(ch,door);
    if (exitdata) {
      if (!real_roomp(exitdata->to_room))
      {
	/* don't print unless immortal */
	if (IS_IMMORTAL(ch))
	{
	  sprintf(buf + strlen(buf), "%s - swirling chaos of #%ld\n\r",
		  exits[door], exitdata->to_room);
	}
      }
      else if (exitdata->to_room != NOWHERE)
      {
	if (IS_IMMORTAL(ch))
	{
	  sprintf(buf + strlen(buf), "%s - %s", exits[door],
		  real_roomp(exitdata->to_room)->name);
	 if(IS_SET(exitdata->exit_info, EX_SECRET))
	   strcat(buf," (secret)");
	  if(IS_SET(exitdata->exit_info, EX_CLOSED))
	    strcat(buf, " (closed)");
	  if(IS_DARK(exitdata->to_room))
	    strcat(buf, " (dark)");
	  sprintf(buf + strlen(buf), " #%ld\n\r", exitdata->to_room);
	}
	else if (!IS_SET(exitdata->exit_info, EX_CLOSED) &&
		 !IS_SET(exitdata->exit_info, EX_SECRET)) /* msw 10/93 */
	{
	  if (IS_DARK(exitdata->to_room))
	    sprintf(buf + strlen(buf), "%s - Too dark to tell\n\r",
					exits[door]);
	  else
	    sprintf(buf + strlen(buf), "%s - %s\n\r", exits[door],
	      real_roomp(exitdata->to_room)->name);
	}
      }
    }
  }

  send_to_char("Obvious exits:\n\r", ch);

  if (*buf)
    send_to_char(buf, ch);
  else
    send_to_char("None.\n\r", ch);
}

#endif
/*Score*/


extern const struct clan clan_list[MAX_CLAN];
extern const char *languagelist[];
extern char *classname[];
extern const char *class_names[];
void do_score(struct char_data *ch, char *argument, int cmd)  {
  	struct time_info_data playing_time;
  	static char buf[1000], buf2[1000];
  	struct time_info_data my_age;
  	char buff[50];
  	struct time_info_data real_time_passed(time_t t2, time_t t1);
  	int x;

 dlog("in do_score");

  	age2(ch, &my_age);

  	if (GET_TITLE(ch)) {
    	ch_printf(ch,"$c000BYou are $c000w%s\n\r", GET_TITLE(ch));
  	}

  playing_time = real_time_passed((time(0)-ch->player.time.logon) + ch->player.time.played, 0);


  ch_printf(ch, "$c000BYou are $c000w%d$c000B years old and $c000w%s$c000B. (Play time: $c000w%d$c000B days and $c000w%d$c000B hours)\n\r%s"
	  , my_age.year,DescAge(my_age.year,GET_RACE(ch)), playing_time.day, playing_time.hours
	  , (((my_age.month == 0) && (my_age.year == 0))? "$c000w It's your birthday today.\n\r":""));

  ch_printf(ch, "$c000BYou belong to the $c000w%s$c000B race, and speak the $c000w%s$c000B language.\n\r"
	  , RaceName[GET_RACE(ch)], languagelist[ch->player.speaks]);

  ch_printf(ch, "$c000BYou have $c000w%d$c000B($c0011%d$c000B) hit, $c000w%d$c000B($c0011%d$c000B) mana, $c000w%d$c000B($c0011%d$c000B) mv points.\n\r",
 		GET_HIT(ch),GET_MAX_HIT(ch), GET_MANA(ch),GET_MAX_MANA(ch), GET_MOVE(ch),GET_MAX_MOVE(ch));
  ch_printf(ch, "$c000BYou won $c000w%d$c000B Quests and own $c000w%d$c000B quest points.\n\r",ch->specials.questwon, ch->player.q_points);
  ch_printf(ch, "$c000BYou carry $c000w%s$c000B coins, and have an additional $c000w%d$c000B in the bank.\n\r", formatNum(GET_GOLD(ch)), (ch->points.bankgold) );
  ch_printf(ch, "$c000BYour alignment is: $c000w%s\n\r", AlignDesc(GET_ALIGNMENT(ch)));


	if ( !(GetMaxLevel(ch)>MAX_MORT || (IS_NPC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF)))) {
  		buf[0] = '\0';
  		sprintf(buff,"%s",formatNum(GET_LEADERSHIP_EXP(ch)));
  		ch_printf(ch,"$c000BCombat experience:$c000w %s $c000B    Leadership experience: $c000w%s$c000B\n\r"
  			, formatNum(GET_EXP(ch)), buff);

		for (x=0; x < MAX_CLASS; x++) {
  			if (HasClass(ch, pc_num_class(x))) {

    			sprintf(buf2, "%-5s$c000BLevel:$c000w%-2d  %-15s$c000B", " ",GET_LEVEL(ch, x), class_names[x]);
    			strcat(buf, buf2);
    			if (GetMaxLevel(ch)<MAX_IMMORT)
    	   			sprintf(buf2,"%s%s:$c000w%s$c000B \n\r"," ","Xp needed"	,formatNum((titles[x][GET_LEVEL(ch, x)+1].exp)- GET_EXP(ch)));
    			else
    	   			sprintf(buf2,"%s:0  %ld", class_names[x],(titles[x][GET_LEVEL(ch, x)+1].exp)- GET_EXP(ch));
    			strcat(buf,buf2);
  			}
		}
  		send_to_char(buf,ch);
	}

	if(IS_IMMORTAL(ch)) {
		ch_printf(ch,"$c000BYou are a level $c000w%d $c000Bimmortal.\n\r",GetMaxLevel(ch));
	}

   ch_printf(ch,"$c000BYou have killed $c000w%d$c000B monsters, and have died $c000w%d$c000B times. Arena: $c000w%d$c000B/$c000w%d$c000B\n\r",
    	ch->specials.m_kills, ch->specials.m_deaths, ch->specials.a_kills, ch->specials.a_deaths);
/*ch_printf(ch,"$c000BKills: $c000w%d$c000B   Deaths: $c000w%d$c000B   Arena Kills: $c000w%d$c000B   Arena Deaths: $c000w%d$c000B\n\r"
	,ch->specials.m_kills, ch->specials.m_deaths, ch->specials.a_kills, ch->specials.a_deaths);
*/


    /* Drow fight -4 in lighted rooms! */
	if (!IS_DARK(ch->in_room) && GET_RACE(ch) == RACE_DROW && !IS_AFFECTED2(ch,AFF2_DARKNESS) && !IS_UNDERGROUND(ch))   {
       ch_printf(ch,"$c0011The light is the area causes you great pain$c0009!\n\r");
	}
	if (IS_SET(ch->specials.affected_by,AFF_WINGSBURNED)) {
	   	send_to_char("$c0009Your burned and tattered wings are a source of great pain.$c000B\n\r",ch);
	}
	if (IS_SET(ch->specials.act,PLR_NOFLY))   {
   		send_to_char("$c000wYou are on the ground in spite of your fly item.$c000B\n\r",ch);
	}

	if(IS_PC(ch)) {
	 	ch->skills[STYLE_STANDARD].learned = 95;
		ch_printf(ch,"$c000BYou are currently fighting $c000w%s$c000B.\n\r", fight_styles[ch->style]);
	}

	if(GET_CLAN(ch) == 0) {
		send_to_char("$c000BYou do not belong to a clan.\n\r",ch);
	} else if (GET_CLAN(ch) == 1) {
		send_to_char("$c000BYou have recently been $c000wexiled$c000B from a clan.\n\r",ch);
	} else {
		ch_printf(ch,"$c000BYou belong to $c000w%s$c000B.\n\r", clan_list[GET_CLAN(ch)].name);
	}

	ch_printf(ch,"$c000BYou have $c000w%d$c000B practice sessions remaining.\n\r",ch->specials.spells_to_learn);

  switch(GET_POS(ch)) {
  	case POSITION_DEAD: send_to_char("$c0009You are DEAD!\n\r", ch); break;
  	case POSITION_MORTALLYW: send_to_char("$c0009You are mortally wounded!, you should seek help!\n\r",ch); break;
  	case POSITION_INCAP:  send_to_char("$c0009You are incapacitated, slowly fading away.\n\r",ch); break;
  	case POSITION_STUNNED: send_to_char("$c0011You are stunned! You can't move.\n\r", ch); break;
  	case POSITION_SLEEPING: send_to_char("$c0010You are $c000wsleeping.\n\r",ch); break;
  	case POSITION_RESTING: send_to_char("$c0012You are $c000wresting.\n\r",ch); break;
  	case POSITION_SITTING: send_to_char("$c0013You are $c000wsitting.\n\r",ch); break;
  	case POSITION_FIGHTING:
  		if (ch->specials.fighting)
  	    	act("$c1009You are fighting $N.", FALSE, ch, 0, ch->specials.fighting, TO_CHAR);//<---------------ACT
  	  	else
  	    	send_to_char("$c1009You are fighting thin air.\n\r",ch);
  	  	break;
  	case POSITION_STANDING: send_to_char("$c000BYou are $c000wstanding.\n\r",ch); break;
  	case POSITION_MOUNTED:
  	  if (MOUNTED(ch)) {
			sprintf(buf,"$c000BYou are riding on $c000w%s.\n\r",MOUNTED(ch)->player.short_descr);
			send_to_char(buf,ch);
  	  } else {
  	    send_to_char("$c000BYou are $c000wstanding.\n\r",ch);break;
  	  }
  	  break;
  	  default: send_to_char("$c000BYou are $c000wfloating.\n\r",ch); break;
  }
	  if (!IS_IMMORTAL(ch) && (!IS_NPC(ch))) {
	    if (GET_COND(ch,DRUNK)>10)
	       send_to_char("$c0011You are intoxicated.\n\r",ch);
	    if (GET_COND(ch,FULL)<2 && GET_COND(ch,FULL) != -1)
	      send_to_char("$c000BYou are $c000whungry$c000B...\n\r",ch);
	    if (GET_COND(ch,THIRST)<2  && GET_COND(ch,THIRST) != -1)
	      send_to_char("$c000BYou are $c000wthirsty$c000B...\n\r",ch);
	  }
	//send_to_char("\n\r\n\r$c000BType '$c000wpinfo$c000B' to see list of current character flags.\n\r",ch);
}



void do_time(struct char_data *ch, char *argument, int cmd)
{
  char buf[100], *suf;
  int weekday, day;

dlog("in do_time");

  sprintf(buf, "It is %d o'clock %s, on ",
	  ((time_info.hours % 12 == 0) ? 12 : ((time_info.hours) % 12)),
	  ((time_info.hours >= 12) ? "pm" : "am") );

  weekday = ((35*time_info.month)+time_info.day+1) % 7;/* 35 days in a month */

  strcat(buf,weekdays[weekday]);
  strcat(buf,"\n\r");
  send_to_char(buf,ch);

  day = time_info.day + 1;   /* day in [1..35] */

  if (day == 1)
    suf = "st";
  else if (day == 2)
    suf = "nd";
  else if (day == 3)
    suf = "rd";
  else if (day < 20)
    suf = "th";
  else if ((day % 10) == 1)
    suf = "st";
  else if ((day % 10) == 2)
    suf = "nd";
  else if ((day % 10) == 3)
    suf = "rd";
  else
    suf = "th";

  sprintf(buf, "The %d%s Day of the %s, Year %d.\n\r",
	  day,
	  suf,
	  month_name[(int)time_info.month],
	  time_info.year);

  send_to_char(buf,ch);
}


void do_weather(struct char_data *ch, char *argument, int cmd)
{
 static char *sky_look[4]= {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"};
  static char buf[1024];

dlog("in do_weather");

  if (OUTSIDE(ch)) {
    sprintf(buf,
	    "The sky is %s and %s.\n\r",
	    sky_look[weather_info.sky],
	    (weather_info.change >=0 ? "you feel a warm wind from south" :
	     "your foot tells you bad weather is due"));
    send_to_char(buf,ch);
  } else
    send_to_char("You have no feeling about the weather at all.\n\r", ch);
}


void do_help(struct char_data *ch, char *argument, int cmd)
{
  FILE *fl;
  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


dlog("in do_help");

  if (!ch->desc)
    return;

  for(;isspace(*argument); argument++)  ;


  if (*argument)
    {
      if (!help_index)  {
	  send_to_char("No help available.\n\r", ch);
	  return;
	}
      bot = 0;
      top = top_of_helpt;

      for (;;)
	{
	  mid = (bot + top) / 2;
	  minlen = strlen(argument);

	  if (!(chk = strn_cmp(argument, help_index[mid].keyword, minlen)))
	    {
	      rewind(help_fl);
	      fseek(help_fl, help_index[mid].pos, 0);
	      *buffer = '\0';
	      for (;;)  {
		  fgets(buf, 80, help_fl);
		  if (*buf == '#')
		    break;
		  if (strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2)
		    break;
		  strcat(buffer, buf);
		  strcat(buffer, "\r");
		}
	      page_string(ch->desc, buffer, 1);
	      return;
	    }     else if (bot >= top)      {

	      		if(cmd==38) {
		  			sprintf(buf,"spell %s",argument);
		  			do_help(ch, buf, 1);
		  			return;
		  		  } else
		  		     if(cmd==1) {
		  				  half_chop(argument,buf,buffer);

		  				sprintf(buf,"skill %s",buffer);
		  				do_help(ch, buf, 0);
		  				return;
					 }



	      send_to_char("There is no help on that word.\n\r", ch);
	      send_to_char("Perhaps try help skills <skill> or help spell <spell>.\n\r", ch);
	       //(GH)NO help so add that key word to a file called ADD_HELP
		  if (!(fl = fopen(NEWHELP_FILE, "a")))	{
		    log("Could not open the ADD_HELP-file.\n\r");
		    return;
		  }
		  sprintf(buf, "**%s: help %s\n", GET_NAME(ch), argument);
		  fputs(buf, fl);
	      fclose(fl);

	      return;
	    }
	  else if (chk > 0)
	    bot = ++mid;
	  else
	    top = --mid;
	}
      return;
    }


  send_to_char(help, ch);

}


void do_wizhelp(struct char_data *ch, char *arg, int cmd)
{
 char buf[MAX_STRING_LENGTH];

 int i, j = 1;
 NODE *n;

dlog("in do_wizhelp");

 if(IS_NPC(ch))
    return;

  one_argument(arg,buf);                /* new msw */
  if (*arg) {
    do_actual_wiz_help(ch,arg,cmd); /* asking for help on keyword, try looking in file */
    return;
   }

 sprintf(buf, "Wizhelp <keyword>\n\rWizard Commands Available To You:\n\r\n\r");

 for(i = 0; i < 27; i++) {
    n = radix_head[i].next;
    while(n) {
	if(n->min_level <= GetMaxLevel(ch) && n->min_level >= LOW_IMMORTAL) {
	   if(n->min_level == GetMaxLevel(ch))
	     sprintf((buf + strlen(buf)), "$c000BL:$c000Y%d $c000w%-11s",n->min_level,n->name);
	   else
	     sprintf((buf + strlen(buf)), "$c000BL:$c000Y%d $c000w%-11s",n->min_level,n->name);




	   if(!(j % 5)) {
	           sprintf((buf + strlen(buf)), "\n\r");
   	   }
	   j++;
	 }
	n = n->next;
      }
  }
	strcat(buf, "\n\r");

 page_string(ch->desc, buf, 1);
}

void do_actual_wiz_help(struct char_data *ch, char *argument, int cmd)
{


  int chk, bot, top, mid, minlen;
  char buf[MAX_STRING_LENGTH], buffer[MAX_STRING_LENGTH];


  dlog("in do_actual_wiz");

  if (!ch->desc)
    return;

  for(;isspace(*argument); argument++)  ;


  if (*argument)
    {
      if (!wizhelp_index)       {
	  send_to_char("No wizhelp available.\n\r", ch);
	  return;
	}
      bot = 0;
      top = top_of_wizhelpt;

      for (;;)
	{
	  mid = (bot + top) / 2;
	  minlen = strlen(argument);

	  if (!(chk = strn_cmp(argument, wizhelp_index[mid].keyword, minlen)))
	    {
	      rewind(wizhelp_fl);
	      fseek(wizhelp_fl, wizhelp_index[mid].pos, 0);
	      *buffer = '\0';
	      for (;;)  {
		  fgets(buf, 80, wizhelp_fl);
		  if (*buf == '#')
		    break;
		  if (strlen(buf)+strlen(buffer) > MAX_STRING_LENGTH-2)
		    break;
		  strcat(buffer, buf);
		  strcat(buffer, "\r");
		}
	      page_string(ch->desc, buffer, 1);
	      return;
	    }     else if (bot >= top)      {
	      send_to_char("There is no wizhelp on that word.\n\r", ch);
	      return;
	    }
	  else if (chk > 0)
	    bot = ++mid;
	  else
	    top = --mid;
	}
      return;
    }


  /* send a generic wizhelp menu like help I guess send_to_char(help, ch); */

}


void do_command_list(struct char_data *ch, char *arg, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  int i, j = 1;
  NODE *n;

  dlog("in do_command_list");

  if(IS_NPC(ch))
    return;

  sprintf(buf, "Commands Available To You:\n\r\n\r");

  for(i = 0; i < 27; i++) {
    n = radix_head[i].next;
    while(n) {
	if(n->min_level <= GetMaxLevel(ch)) {
	if (strlen(buf)+strlen(n->name) <= MAX_STRING_LENGTH)
	     sprintf((buf + strlen(buf)), "%-10s", n->name);
	   if(!(j % 7))
	    if (strlen(buf)+4 <= MAX_STRING_LENGTH)
		sprintf((buf + strlen(buf)), "\n\r");
	   j++;
	 }
	n = n->next;
      }
  }

 strcat(buf, "\n\r");

 page_string(ch->desc, buf, 1);
}

#define OK_NAME(name,mask)      (mask[0]=='\0' || \
				strncmp(strcpy(tmpname1,lower(GET_NAME(name))),\
					strcpy(tmpname2,lower(mask)),\
					strlen(mask))==0)
/*
  int OK_NAME(struct char_data *name, char *mask)
  {
  char n1[80],n2[80];
  if(!*mask) return 1;
  strcpy(n1,lower(GET_NAME(name)));
  strcpy(n2,lower(mask));
  return(strncmp(n1,n2,strlen(mask))==0);
}
*/


void do_who(struct char_data *ch, char *argument, int cmd)
{
  struct zone_data    *zd;
  struct room_data *rm=0;
  struct descriptor_data *d;
  struct char_data *person;
  char buffer[MAX_STRING_LENGTH*3]="",tbuf[1024];
  int count;

  char color_cnt=1;
  char flags[20]="";
  char name_mask[40]="";
  char tmpname1[80],tmpname2[80];
  char buf[256];

dlog("in do_who");

	/*  check for an arg */
	argument = one_argument(argument,tbuf);
	if(tbuf[0]=='-' && tbuf[1]!='\0')
		strcpy(flags,tbuf+1);
	else
		strcpy(name_mask,tbuf);
	if(*argument) {
		argument = one_argument(argument,tbuf);
		if(tbuf[0]=='-' && tbuf[1]!='\0')
			strcpy(flags,tbuf+1);
		else
			strcpy(name_mask,tbuf);
	}

	if ( (IS_IMMORTAL(ch) && flags[0]=='\0') || !IS_IMMORTAL(ch) || cmd == 234) {
		if( IS_IMMORTAL(ch) )
			sprintf(buffer,"$c0005Players [God Version -? for Help]\n\r--------\n\r");
		else if(cmd==234) {
			sprintf(buffer,"$c0005Players\n\r");
			strcat(buffer,"------------\n\r");
		} else {
			sprintf(buffer,"$c0005                        Havok Players\n\r");
			strcat(buffer,       "                        -------------\n\r");
		}
		if (cmd==234) { //(gh) says zone name in whoz
			rm = real_roomp(ch->in_room);  //new stuff for whoz(GH)
			zd = zone_table+rm->zone;
			sprintf(buf, "$c0005Zone: $c0015%s",zd->name);
			strcat(buffer,buf);
		if(IS_IMMORTAL(ch)) {
			sprintf(buf,"$c0005($c0015%ld$c0005)",rm->zone);
			strcat(buffer,buf);
		}
		strcat(buffer,"\n\r\n\r");
	}

	count=0;
	for (d = descriptor_list; d; d = d->next) {
		person=(d->original?d->original:d->character);
//		if(!person->in_room) /* Let's not show people who sit at menu   -Lennya */
//			return;

		if (CAN_SEE(ch, d->character) && (real_roomp(person->in_room)) &&
				(real_roomp(person->in_room)->zone == real_roomp(ch->in_room)->zone || cmd!=234 ) &&
				(!index(flags,'g') || IS_IMMORTAL(person))) {
			if (OK_NAME(person,name_mask)) {
				count++;
				color_cnt = (color_cnt++ % 9);  /* range 1 to 9 */

				if (cmd==234) { /* it's a whozone command */
					if ((!IS_AFFECTED(person, AFF_HIDE)) || (IS_IMMORTAL(ch))) {
						sprintf(tbuf,"$c0012%-25s - %s", GET_NAME(person),real_roomp(person->in_room)->name);
						if (GetMaxLevel(ch) >= LOW_IMMORTAL)
							sprintf(tbuf+strlen(tbuf)," [%ld]", person->in_room);
					}
				} else {
					char levels[40]="", classes[20]="";
					extern char *classname[];
					int i,total,classn; long bit;
#if 1
					if(!IS_IMMORTAL(person)) {
		for(bit=1,i=total=classn=0;i<=BARD_LEVEL_IND;i++, bit<<=1) {
			if(HasClass(person,bit)) {
				classn++;
				total+=person->player.level[i];
				if(strlen(classes)!=0)
					strcat(classes,"/");
				sprintf(classes+strlen(classes),"%s",classname[i]);
			}
		}
		if (total <=0)
			total =1;
		if (classn <= 0 )
			classn =1;
		total/=classn;
		if(GetMaxLevel(person)==50) {
			if(GET_EXP(person) > 200000000 || IS_SET(ch->specials.act, PLR_LEGEND))
				strcpy(levels,"$c0009Legend");
			else strcpy (levels,"$c0012Hero");
		}
		else if(total<11) strcpy(levels,"$c0008Apprentice");
		else if(total<21) strcpy(levels,"$c0004Pilgrim");
		else if(total<31) strcpy(levels,"$c0006Explorer");
		else if(total<41) strcpy(levels,"$c0014Adventurer");
		else if(total<51) strcpy(levels,"$c0015Mystical");
		sprintf(tbuf, "%s $c0012%s",levels, classes);
		sprintf(levels,"%32s","");
		strcpy(levels+10-((strlen(tbuf)-12)/2),tbuf);

		sprintf(tbuf, "%-32s $c0005: $c0007%s",
				levels,person->player.title?person->player.title:GET_NAME(person));//"(Null)");
/* commented this out becuz %-10s uses up its space for color codes as well,
 * thus making it necessary to use the same amount of colors for each clan, too
 * much of a bother imo.   -Lennya
 */

//		strcpy(levels,tbuf);
//		if (IS_SET(person->specials.act, PLR_CLAN_LEADER))
//			sprintf(bufx, "$c0008[$c000w%s$c0008]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else if(GET_CLAN(person)>0)
//			sprintf(bufx, "$c000c[$c000w%s$c000c]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else
//			sprintf(bufx, "           "); /* length 11 */
/* fix up length, ugly stuff */
//		length = 11; /* this should be enough length for any clan abbrev */
//		clength = length - color_strlen(ch, bufx, 596);
//		for(j = 1; j <= clength; j++) {
//			strcat(bufx, " "); /* add some spaces */
//		}
//		sprintf(tbuf, "%-32s %s $c0005: $c0007%s",
//				levels,bufx,person->player.title?person->player.title:GET_NAME(person));//"(Null)");

	} else {
		switch(GetMaxLevel(person)) {
		case 51: sprintf(levels, "Lesser Deity"); break;
		case 52: sprintf(levels,"Deity"); break;
		case 53: sprintf(levels, "Greater Deity"); break;
		case 54:  if (GET_SEX(person) == SEX_MALE){       // if loop checks if the person
				sprintf(levels, "Lesser God");            // is female or men
				break;                                    // modified by Thyrza 20/06/97
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Lesser Goddess");
				break;
				}

		case 55:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "God");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Goddess");
				break;
				}

		case 56:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "Greater God");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Greater Goddess");
				break;
				}


		case 57:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "God of Judgement");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Goddess of Judgement");
				break;
				}

		case 58:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "Lord");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Lady");
				break;
				}
		case 59:  if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Supreme Lady");
				break;
				}
			  else if (GET_SEX(person) == SEX_MALE) {
				sprintf(levels,"Supreme Lord");
				break;
				}
			  else {
				sprintf(levels,"Supreme Thing");
				break;
				}

		case 60: sprintf(levels, "Supreme Being");
		}

		if(!strcmp(GET_NAME(person), "Tsaron"))       // Hardcoded the names of the current
			sprintf(levels, "Supreme Dictator");      // High council members, this should be
		else if(!strcmp(GET_NAME(person), "Banon"))   // fixxed with new immortal system code
			sprintf(levels, "$c000BC$c000Rr$c000Ye$c000Ba$c000Rt$c000Yo$c000Br");// -MW 02/20/2001
		else if(!strcmp(GET_NAME(person), "Keirstad"))
			sprintf(levels, "Lord of Building");
		else if(!str_cmp(GET_NAME(person), "Ignatius"))
			sprintf(levels, "Dragon Lord");
		else if(!str_cmp(GET_NAME(person), "Pentak"))
			sprintf(levels, "Creator");

		sprintf(tbuf, "%s",levels);
		sprintf(levels,"%30s","");
		if(!strcmp(GET_NAME(person), "Banon")) {
			strcpy(levels+10-((strlen(tbuf)/2)/5),tbuf);

			sprintf(tbuf, " $c0011%-20s $c0005      : $c0007%s",levels,
						person->player.title?person->player.title:GET_NAME(person));//"(Null)");

//		if (IS_SET(person->specials.act, PLR_CLAN_LEADER))
//			sprintf(bufx, "$c0008[$c000w%s$c0008]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else if(GET_CLAN(person)>0)
//			sprintf(bufx, "$c000c[$c000w%s$c000c]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else
//			sprintf(bufx, "           "); /* length 11 */
/* fix up length, ugly stuff */
//		length = 11; /* this should be enough length for any clan abbrev */
//		clength = length - color_strlen(ch, bufx, 596);
//		for(j = 1; j <= clength; j++) {
//			strcat(bufx, " "); /* add some spaces */
///		}
//		sprintf(tbuf, "$c0011%-20s %s $c0005: $c0007%s",
//				levels,bufx,person->player.title?person->player.title:GET_NAME(person));//"(Null)");

//			if (IS_SET(person->specials.act, PLR_CLAN_LEADER))
//				sprintf(bufx, "$c0008[$c000w%s$c0008]$c000w ", clan_list[GET_CLAN(person)].shortname);
//			else if(GET_CLAN(person)>0)
//				sprintf(bufx, "$c000W[$c000w%s$c000W]$c000w ", clan_list[GET_CLAN(person)].shortname);
//			else
//				sprintf(bufx, "");

//			sprintf(tbuf, " $c0011%-32s %-12s$c0005: $c0007%s",levels,
//	                      bufx,person->player.title?person->player.title:GET_NAME(person));//"(Null)");

		} else {
			strcpy(levels+10-(strlen(tbuf)/2),tbuf);
			sprintf(tbuf, "$c0011%-20s $c0005: $c0007%s",levels,
							person->player.title?person->player.title:GET_NAME(person));//"(Null)");

//		if (IS_SET(person->specials.act, PLR_CLAN_LEADER))
//			sprintf(bufx, "$c0008[$c000w%s$c0008]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else if(GET_CLAN(person)>0)
//			sprintf(bufx, "$c000c[$c000w%s$c000c]$c000w", clan_list[GET_CLAN(person)].shortname);
//		else
//			sprintf(bufx, "           "); /* length 11 */
/* fix up length, ugly stuff */
//		length = 11; /* this should be enough length for any clan abbrev */
///		clength = length - color_strlen(ch, bufx, 596);
//		for(j = 1; j <= clength; j++) {
//			strcat(bufx, " "); /* add some spaces */
//		}
//		sprintf(tbuf, "$c0011%-20s %s $c0005: $c0007%s",
//				levels,bufx,person->player.title?person->player.title:GET_NAME(person));//"(Null)");

//			if (IS_SET(person->specials.act, PLR_CLAN_LEADER))
//				sprintf(bufx, "$c0008[$c000w%s$c0008]$c000w ", clan_list[GET_CLAN(person)].shortname);
//			else if(GET_CLAN(person)>0)
//				sprintf(bufx, "$c000W[$c000w%s$c000W]$c000w ", clan_list[GET_CLAN(person)].shortname);
//			else
//				sprintf(bufx, "");

//			sprintf(tbuf, "$c0011%-32s %-12s$c0005: $c0007%s",levels, bufx,
//					person->player.title?person->player.title:GET_NAME(person));//"(Null)");
		}
	}
#else
	sprintf(tbuf, "$c100%d%s", color_cnt, person->player.title?person->player.title:GET_NAME(person));//"(Null)");
#endif
	}

	if (IS_SET(person->player.user_flags, NEW_USER))
		sprintf(tbuf+strlen(tbuf),"$c000G [$c000WNEW$c000G]$c0007");
	    if(IS_AFFECTED2(person,AFF2_AFK))
	      sprintf(tbuf+strlen(tbuf),"$c0008 [AFK]$c0007");
	    if(IS_AFFECTED2(person,AFF2_QUEST))
	      sprintf(tbuf+strlen(tbuf),"$c0008 [$c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008]$c0007");

		if (IS_SET(ch->specials.act, NEW_USER))
	if (IS_LINKDEAD(person))
		sprintf(tbuf+strlen(tbuf),"$c0015 [LINKDEAD]$c0007");
	if (IS_IMMORTAL(ch) && person->invis_level > 50)
		sprintf(tbuf+strlen(tbuf), "(invis %d)",person->invis_level);
	    sprintf(tbuf+strlen(tbuf),"\n\r");
	    if (strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512)
	      strcat(buffer,tbuf);
	}
      }
    }
    if(index(flags,'g'))
      sprintf(tbuf, "\n\r$c0005Total visible gods: $c0015%d\n\r", count);
    else
      sprintf(tbuf, "\n\r$c0005Total visible players: $c0015%d\n\r", count);

    sprintf(tbuf+strlen(tbuf), "$c0005Max connects since last reboot: $c0015%ld\n\r", total_connections);
    sprintf(tbuf+strlen(tbuf), "$c0005Max players online since last reboot: $c0015%ld\n\r", total_max_players);

    if (strlen(buffer)+strlen(tbuf) < MAX_STRING_LENGTH*2-512)
      strcat(buffer,tbuf);
  } else {                                  /* GOD WHO */

    int listed = 0, count, lcount, l, skip = FALSE;
    char ttbuf[256];

    sprintf(buffer,"$c0005Players [God Version -? for Help]\n\r--------\n\r");

    count=0;
    lcount=0;
    if(index(flags,'?')) {
	send_to_char(buffer,ch);
	send_to_char("$c0007[-]i=idle l=levels t=title h=hit/mana/move s=stats r=race\n\r",ch);
	send_to_char("[-]d=linkdead g=God o=Mort [1]Mage[2]Cleric[3]War[4]Thief[5]Druid\n\r",ch);
	send_to_char("[-][6]Monk[7]Barb[8]Sorc[9]Paladin[!]Ranger[@]Psi\n\r", ch);
	return;
    }

    for (person = character_list; person; person = person->next) {
      if (!IS_NPC(person) && CAN_SEE(ch, person) && OK_NAME(person,name_mask)) {
	count++;
	if (person->desc == NULL) lcount ++;
	skip = FALSE;
	if (index(flags,'g') != NULL)
	  if (!IS_IMMORTAL(person)) skip = TRUE;
	if (index(flags,'o') != NULL)
	  if (IS_IMMORTAL(person)) skip = TRUE;
	if (index(flags,'1') != NULL)
	  if (!HasClass(person,CLASS_MAGIC_USER)) skip = TRUE;
	if (index(flags,'2') != NULL)
	  if (!HasClass(person,CLASS_CLERIC)) skip = TRUE;
	if (index(flags,'3') != NULL)
	  if (!HasClass(person,CLASS_WARRIOR)) skip = TRUE;
	if (index(flags,'4') != NULL)
	  if (!HasClass(person,CLASS_THIEF)) skip = TRUE;
	if (index(flags,'5') != NULL)
	  if (!HasClass(person,CLASS_DRUID)) skip = TRUE;
	if (index(flags,'6') != NULL)
	  if (!HasClass(person,CLASS_MONK)) skip = TRUE;
	if (index(flags,'7') != NULL)
	  if (!HasClass(person,CLASS_BARBARIAN)) skip = TRUE;
	if (index(flags,'8') != NULL)
	  if (!HasClass(person,CLASS_SORCERER)) skip = TRUE;
	if (index(flags,'9') != NULL)
	  if (!HasClass(person,CLASS_PALADIN)) skip = TRUE;
	if (index(flags,'!') != NULL)
	  if (!HasClass(person,CLASS_RANGER)) skip = TRUE;
	if (index(flags,'@') != NULL)
	  if (!HasClass(person,CLASS_PSI)) skip = TRUE;

	if (!skip)             {
	  if (person->desc == NULL) {
	    if (index(flags,'d') != NULL) {
	      sprintf(tbuf, "$c0003[%-12s] ", GET_NAME(person));
	      listed++;
	    }
	  } else {
	    if (IS_NPC(person) && IS_SET(person->specials.act, ACT_POLYSELF)) {
	      sprintf(tbuf, "(%-12s) ", GET_NAME(person));
	      listed++;
	    } else {
	      sprintf(tbuf, "$c0012%-14s ", GET_NAME(person));
	      listed++;
	    }
	  }
	  if ((person->desc != NULL) || (index(flags,'d') != NULL)) {
	    for (l = 0; l < strlen(flags) ; l++) {
	      switch (flags[l]) {
		case 'r': {  /* show race */
		    char bbuf[256];
		  sprinttype((person->race),RaceName,ttbuf);
		  sprintf(bbuf," [%s] ",ttbuf);
		  strcat(tbuf,bbuf);
		  break;
		}
		case 'i':
		  sprintf(ttbuf,"Idle:[%-3d] ",person->specials.timer);
		  strcat(tbuf,ttbuf);
		  break;
		case 'l':
		  sprintf(ttbuf,"Level:[%-2d/%-2d/%-2d/%-2d/%-2d/%-2d/%-2d/%-2d/%-2d/%-2d/%-2d] ",
			person->player.level[0],person->player.level[1],
			person->player.level[2],person->player.level[3],
			person->player.level[4],person->player.level[5],
			person->player.level[6],person->player.level[7],
			person->player.level[8],person->player.level[9],
			person->player.level[10]);
		  strcat(tbuf,ttbuf);
		  break;
		case 'h':
		  sprintf(ttbuf,"Hit:[%d/%-3d] Mana:[%d/%-3d] Move:[%d/%-3d]",GET_HIT(person),GET_MAX_HIT(person),GET_MANA(person),GET_MAX_MANA(person),GET_MOVE(person),GET_MAX_MOVE(person));
		  strcat(tbuf,ttbuf);
		  break;
		case 's':
		  if (GET_STR(person) != 18)
		    sprintf(ttbuf,"[S:%-2d I:%-2d W:%-2d C:%-2d D:%-2d CH:%-2d] ",
		       GET_STR(person),GET_INT(person),GET_WIS(person),
		       GET_CON(person),GET_DEX(person),GET_CHR(person));
		  else
		    sprintf(ttbuf,"[S:%-2d(%1d) I:%-2d W:%-2d C:%-2d D:%-2d CH:%-2d] ",
		       GET_STR(person),GET_ADD(person),GET_INT(person),
		       GET_WIS(person),GET_CON(person),GET_DEX(person),
		       GET_CHR(person));
		  strcat(tbuf,ttbuf);
		  break;
		case 't':
		  sprintf(ttbuf," %-16s ",(person->player.title?person->player.title:GET_NAME(person)));//"(null)"));
		  strcat(tbuf,ttbuf);
		  break;
		case 'v':
		   sprintf (ttbuf, "[I:%d]", person->invis_level);
		   strcat(tbuf,ttbuf);
		   break;
		default:
		  break;
	      }
	    }
	  }
	  if ((person->desc != NULL) || (index(flags,'d') != NULL)) {
	    if(OK_NAME(person,name_mask)) {
	      if ((strlen(buffer)+strlen(tbuf)) < ((MAX_STRING_LENGTH*2)-512)) {
		strcat(buffer,tbuf);
		strcat(buffer,"\n\r");
	      }
	    }
	  }
	}
      }
    }

    if (listed<=0)
      sprintf(tbuf,"\n\r$c0005No Matches\n\r");
    else
      sprintf(tbuf,"\n\r$c0005Total players / Link dead [%d/%d] (%2.0f%%)\n\r",
	  count,lcount,((float)lcount / (int)count) * 100);
    if ((strlen(buffer)+strlen(tbuf)) < ((MAX_STRING_LENGTH*2)-512))
	strcat(buffer,tbuf);
  }
  page_string(ch->desc,buffer,TRUE);
}

void do_users(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH], line[200], buf2[255];

  struct descriptor_data *d;

  dlog("in do_users");

  strcpy(buf, "Connections:\n\r------------\n\r");

  for (d = descriptor_list; d; d = d->next){
    if (d->character) {
      if ( CAN_SEE(ch, d->character) || ((GetMaxLevel(ch) >= 51) && ((d->character)->invis_level <= GetMaxLevel(ch))))
	{
	  if (d && d->character && d->character->player.name)
	    {
	      if(d->original)
		sprintf(line, "%-16s: ", d->original->player.name);
	      else
		sprintf(line, "%-16s: ", d->character->player.name);
	    }
	  else
	    sprintf(line, "UNDEFINED       : ");

	  if (d->host && *d->host)
        {
         sprintf(buf2, "%-22s [%s]\n\r", connected_types[d->connected],d->host);
	} else
          {
	    sprintf(buf2, "%-22s [%s]\n\r", connected_types[d->connected],"????");
	  }
	  strcat(line, buf2);
	  strcat(buf, line);
	} /* could not see the person */
    } /* no  character */
  } /* end for */

/*  send_to_char(buf, ch); */
  page_string(ch->desc,buf,TRUE);
}



void do_inventory(struct char_data *ch, char *argument, int cmd) {

  dlog("in do_inventory");

  send_to_char("You are carrying:\n\r", ch);
  list_obj_in_heap(ch->carrying, ch);
}


void do_equipment(struct char_data *ch, char *argument, int cmd) {
  int j,Worn_Index;
  bool found;
  char String[256];

  dlog("in do_equip");

  send_to_char("You are using:\n\r", ch);
  found = FALSE;
  for (Worn_Index = j=0; j< (MAX_WEAR - 1); j++) {
    sprintf(String,"%s",where[j]);
    send_to_char(String,ch);
    if (ch->equipment[j]){
      if (CAN_SEE_OBJ(ch,ch->equipment[j])) {
	show_obj_to_char(ch->equipment[j],ch,1);
	found = TRUE;
      } else {
	send_to_char("Something.\n\r",ch);
	found = TRUE;
      }
    } else send_to_char ("Nothing.\n\r", ch);
  }
}


void do_credits(struct char_data *ch, char *argument, int cmd) {

dlog("in do_credits");
  page_string(ch->desc, credits, 0);
}


void do_news(struct char_data *ch, char *argument, int cmd) {

dlog("in do_news");
  page_string(ch->desc, news, TRUE);
}


void do_info(struct char_data *ch, char *argument, int cmd) {

dlog("in do_info");
  page_string(ch->desc, info, 0);
}

void do_iwizlist(struct char_data *ch, char *argument, int cmd) {

  dlog("in do_iwizlist");
  page_string(ch->desc, iwizlist, TRUE);
}



void do_wizlist(struct char_data *ch, char *argument, int cmd) {

dlog("in do_wizlist");
  page_string(ch->desc, wizlist, TRUE);
}



int which_number_mobile(struct char_data *ch, struct char_data *mob)
{
  struct char_data      *i;
  char  *name;
  int   number;

  name = fname(mob->player.name);
  for (i=character_list, number=0; i; i=i->next) {
    if (isname(name, i->player.name) && i->in_room != NOWHERE) {
      number++;
      if (i==mob)
	return number;
    }
  }
  return 0;
}

char *numbered_person(struct char_data *ch, struct char_data *person)
{
  static char buf[MAX_STRING_LENGTH];
  if (IS_NPC(person) && IS_IMMORTAL(ch)) {
    sprintf(buf, "%d.%s", which_number_mobile(ch, person),
	    fname(person->player.name));
  } else {
    strcpy(buf, PERS(person, ch));
  }
  return buf;
}

void do_where_person(struct char_data *ch, struct char_data *person, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];

dlog("in do_where_person");

if (!CAN_SEE(ch, person))
   return;

  sprintf(buf, "%-30s- %s ", PERS(person, ch),
	  (person->in_room > -1 ? real_roomp(person->in_room)->name : "Nowhere"));

  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    sprintf(buf+strlen(buf),"[%ld]", person->in_room);

  strcpy(buf+strlen(buf), "\n\r");

  append_to_string_block(sb, buf);
}

void do_where_object(struct char_data *ch, struct obj_data *obj,
			    int recurse, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];

dlog("in do_where_object");

  if (obj->in_room != NOWHERE) { /* object in a room */
    sprintf(buf, "%-30s- %s [%d]\n\r",
	    obj->short_description,
	    real_roomp(obj->in_room)->name,
	    obj->in_room);
  } else if (obj->carried_by != NULL) { /* object carried by monster */
    sprintf(buf, "%-30s- carried by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->carried_by));
  } else if (obj->equipped_by != NULL) { /* object equipped by monster */
    sprintf(buf, "%-30s- equipped by %s\n\r",
	    obj->short_description,
	    numbered_person(ch, obj->equipped_by));
  } else if (obj->in_obj) { /* object in object */
    sprintf(buf, "%-30s- in %s\n\r",
	    obj->short_description,
	    obj->in_obj->short_description);
  } else {
    sprintf(buf, "%-30s- god doesn't even know where...\n\r",
	    obj->short_description);
  }
  if (*buf)
    append_to_string_block(sb, buf);

  if (recurse) {
    if (obj->in_room != NOWHERE)
      return;
    else if (obj->carried_by != NULL)
      do_where_person(ch, obj->carried_by, sb);
    else if (obj->equipped_by != NULL)
      do_where_person(ch, obj->equipped_by, sb);
    else if (obj->in_obj != NULL)
      do_where_object(ch, obj->in_obj, TRUE, sb);
  }
}

void do_where(struct char_data *ch, char *argument, int cmd)
{
  char name[MAX_INPUT_LENGTH+80], buf[MAX_STRING_LENGTH];
  char  *nameonly;
  register struct char_data *i;
  register struct obj_data *k;
  struct descriptor_data *d;
  int   number, count;
  struct string_block   sb;

dlog("in do_where");

	only_argument(argument, name);

	if (!*name) {
		if (GetMaxLevel(ch) < LOW_IMMORTAL) {
			send_to_char("What are you looking for?\n\r", ch);
			return;
		} else {
			init_string_block(&sb);
			append_to_string_block(&sb, "Players:\n\r--------\n\r");

			for (d = descriptor_list; d; d = d->next) {
				if (d->character && (d->connected == CON_PLYNG) && (d->character->in_room != NOWHERE)
									&& CAN_SEE(ch, d->character)) {
					if (d->original)   /* If switched */
						sprintf(buf, "%-20s - %s [%ld] In body of %s\n\r",
											d->original->player.name,
											real_roomp(d->character->in_room)->name,
											d->character->in_room,
											fname(d->character->player.name));
					else
						sprintf(buf, "%-20s - %s [%ld]\n\r",
											d->character->player.name,
											real_roomp(d->character->in_room)->name,
											d->character->in_room);

					append_to_string_block(&sb, buf);
				}
			}
			page_string_block(&sb,ch);
			destroy_string_block(&sb);
			return;
		}
	}

	if (isdigit(*name)) {
		nameonly = name;
		count = number = get_number(&nameonly);
	} else {
		count = number = 0;
	}

	*buf = '\0';

	init_string_block(&sb);

	for (i = character_list; i; i = i->next) {
		if (isname(name, i->player.name) && CAN_SEE(ch, i)) {
			if ((i->in_room != NOWHERE) &&
						((GetMaxLevel(ch)>=LOW_IMMORTAL) || (real_roomp(i->in_room)->zone ==
						real_roomp(ch->in_room)->zone))) {
				if (number==0 || (--count) == 0) {
					if (number==0) {
						sprintf(buf, "[%2d] ", ++count); /* I love short circuiting :) */
						append_to_string_block(&sb, buf);
					}
					do_where_person(ch, i, &sb);
					*buf = 1;
					if (number!=0)
						break;
				}
				if (GetMaxLevel(ch) < LOW_IMMORTAL)
					break;
			}
		}
	}
	/*  count = number;*/

	if (GetMaxLevel(ch) >= SAINT ) {
		for (k = object_list; k; k = k->next) {
			if (isname(name, k->name) && CAN_SEE_OBJ(ch, k)) {
				if (number==0 || (--count)==0) {
					if (number==0) {
						sprintf(buf, "[%2d] ", ++count);
						append_to_string_block(&sb, buf);
					}
					do_where_object(ch, k, number!=0, &sb);
					*buf = 1;
					if (number!=0)
						break;
				}
			}
		}
	}

	if (!*sb.data)
		send_to_char("Couldn't find any such thing.\n\r", ch);
	else
		page_string_block(&sb, ch);
	destroy_string_block(&sb);
}




void do_levels(struct char_data *ch, char *argument, int cmd)
{
  int i, RaceMax, class;
  char buf[MAX_STRING_LENGTH*2],buf2[MAX_STRING_LENGTH];


dlog("in do_levels");

  if (IS_NPC(ch))       {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }

  *buf = '\0';
/*
**  get the class
*/

  for (;isspace(*argument);argument++);

  if (!*argument) {
    send_to_char("You must supply a class!\n\r", ch);
    return;
  }

  switch(*argument) {
  case 'C':
  case 'c':
    class = CLERIC_LEVEL_IND;
    break;
  case 'F':
  case 'f':
  case 'W':
  case 'w':
    class = WARRIOR_LEVEL_IND;
    break;
  case 'M':
  case 'm':
    class = MAGE_LEVEL_IND;
    break;
  case 'T':
  case 't':
    class = THIEF_LEVEL_IND;
    break;
  case 'D':
  case 'd':
    class = DRUID_LEVEL_IND;
    break;
  case 'K':
  case 'k':
    class = MONK_LEVEL_IND;
    break;

  case 'B':
  case 'b':
    class = BARBARIAN_LEVEL_IND;
    break;

  case 'S':
  case 's':
    class = SORCERER_LEVEL_IND;
    break;

  case 'P':
  case 'p':
    class = PALADIN_LEVEL_IND;
    break;

  case 'R':
  case 'r':
    class = RANGER_LEVEL_IND;
    break;

  case 'I':
  case 'i':
    class = PSI_LEVEL_IND;
    break;

  default:
    sprintf(buf, "I don't recognize %s\n\r", argument);
    send_to_char(buf,ch);
    return;
    break;
  }

  RaceMax = RacialMax[GET_RACE(ch)][class];

 buf[0]=0;

  for (i = 1; i <= RaceMax; i++) {
/* crashed in sprintf here, see if you can figure out why. msw, 8/24/94 */
    sprintf(buf2, "[%2d] %9ld-%-9ld : %s\n\r", i,
	    titles[class][i].exp,
	    titles[class][i + 1].exp, (GET_SEX(ch)==SEX_FEMALE?titles[class][i].title_f:titles[class][i].title_m));
	/* send_to_char(buf, ch); */
	strcat(buf,buf2);
  }
	strcat(buf,"\n\r");
	page_string(ch->desc,buf,1);

}



void do_consider(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[256];
  int diff;

dlog("in do_consider");

  only_argument(argument, name);

  if (!(victim = get_char_room_vis(ch, name))) {
    send_to_char("Consider killing who?\n\r", ch);
    return;
  }

  if (victim == ch) {
    send_to_char("Easy! Very easy indeed!\n\r", ch);
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("Would you like to borrow a cross and a shovel?\n\r", ch);
    return;
  }

  act("$n looks at $N", FALSE, ch, 0, victim, TO_NOTVICT);
  act("$n looks at you", FALSE, ch, 0, victim, TO_VICT);


if (GetMaxLevel(ch)>=LOW_IMMORTAL) {
  send_to_char("Consider this, What the heck do you need con for?\n\r", ch);
  return;
 }

  diff =  GET_AVE_LEVEL(victim) - GET_AVE_LEVEL(ch);

  diff += MobLevBonus(victim);

  if (diff <= -10)
    send_to_char("Too easy to be believed.\n\r", ch);
  else if (diff <= -5)
    send_to_char("Not a problem.\n\r", ch);
  else if (diff <= -3)
    send_to_char("Rather easy.\n\r",ch);
  else if (diff <= -2)
    send_to_char("Easy.\n\r", ch);
  else if (diff <= -1)
    send_to_char("Fairly easy.\n\r", ch);
  else if (diff == 0)
    send_to_char("The perfect match!\n\r", ch);
  else if (diff <= 1)
    send_to_char("You would need some luck!\n\r", ch);
  else if (diff <= 2)
    send_to_char("You would need a lot of luck!\n\r", ch);
  else if (diff <= 3)
    send_to_char("You would need a lot of luck and great equipment!\n\r", ch);
  else if (diff <= 5)
    send_to_char("Do you feel lucky, punk?\n\r", ch);
  else if (diff <= 10)
    send_to_char("Are you crazy?  Is that your problem?\n\r", ch);
  else if (diff <= 30)
    send_to_char("You ARE mad!\n\r", ch);
  else
    send_to_char("Why don't I just kill you right now and save you the trouble?\n\r", ch);

#if 0

  if (ch->skills) {
    int skill=0;
    int learn=0;
    int num, num2;
    float fnum;

    if (IsAnimal(victim) && ch->skills[SKILL_CONS_ANIMAL].learned) {
      skill = SKILL_CONS_ANIMAL;
      learn = ch->skills[skill].learned;
      act("$N seems to be an animal", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsVeggie(victim) && ch->skills[SKILL_CONS_VEGGIE].learned) {
      if (!skill)
	skill = SKILL_CONS_VEGGIE;
      learn = MAX(learn, ch->skills[SKILL_CONS_VEGGIE].learned);
      act("$N seems to be an ambulatory vegetable",
	  FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsDiabolic(victim) && ch->skills[SKILL_CONS_DEMON].learned) {
      if (!skill)
	skill = SKILL_CONS_DEMON;
      learn = MAX(learn, ch->skills[SKILL_CONS_DEMON].learned);
      act("$N seems to be a demon!", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsReptile(victim) && ch->skills[SKILL_CONS_REPTILE].learned) {
      if (!skill)
	skill = SKILL_CONS_REPTILE;
      learn = MAX(learn, ch->skills[SKILL_CONS_REPTILE].learned);
      act("$N seems to be a reptilian creature",
	  FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsUndead(victim) && ch->skills[SKILL_CONS_UNDEAD].learned) {
      if (!skill)
	skill = SKILL_CONS_UNDEAD;
      learn = MAX(learn, ch->skills[SKILL_CONS_UNDEAD].learned);
      act("$N seems to be undead", FALSE, ch, 0, victim, TO_CHAR);
    }

    if (IsGiantish(victim)&& ch->skills[SKILL_CONS_GIANT].learned) {
      if (!skill)
	skill = SKILL_CONS_GIANT;
      learn = MAX(learn, ch->skills[SKILL_CONS_GIANT].learned);
      act("$N seems to be a giantish creature", FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsPerson(victim) && ch->skills[SKILL_CONS_PEOPLE].learned) {
      if (!skill)
	skill = SKILL_CONS_PEOPLE;
      learn = MAX(learn, ch->skills[SKILL_CONS_PEOPLE].learned);
      act("$N seems to be a human or demi-human",
	  FALSE, ch, 0, victim, TO_CHAR);
    }
    if (IsOther(victim)&& ch->skills[SKILL_CONS_OTHER].learned) {
      if (!skill)
	skill = SKILL_CONS_OTHER;
      learn = MAX(learn, ch->skills[SKILL_CONS_OTHER].learned/2);
      act("$N seems to be a monster you know about",
	  FALSE, ch, 0, victim, TO_CHAR);
    }

    if (learn > 95) learn = 95;

    if (learn == 0) return;

    WAIT_STATE(ch, PULSE_VIOLENCE*2);

#if 1

    num = (int)GetApprox(GET_MAX_HIT(victim), learn);
    num2 = (int)GET_MAX_HIT(ch);
    if (!num2) num2=1;
    fnum = ((int)num/(int)num2);
    sprintf(buf, "Est Max hits are: %s\n\r", DescRatio(fnum));
    send_to_char(buf, ch);

    num = (int)GetApprox(GET_AC(victim), learn);
    num2 = (int)GET_AC(ch);
    if (!num2) num2=1;
    fnum = ((int)num/(int)num2);

    sprintf(buf, "Est. armor class is : %s\n\r", DescRatio(fnum));
    send_to_char(buf, ch);



    if (learn > 60)
    {
      sprintf(buf, "Est. # of attacks: %s\n\r",
	      DescAttacks((int)GetApprox((int)victim->mult_att,
				  learn)));
      send_to_char(buf, ch);
    }

    if (learn > 70) {

      num =  (int)GetApprox((int)victim->specials.damnodice,
			learn);
      num2 = (int)GetApprox((int)victim->specials.damsizedice,
			learn);
	if (!num2) num2=1;
      fnum = (int)num*(num2/2.0);
      sprintf(buf, "Est. damage of attacks is %s\n\r",
	      DescDamage(fnum));

      send_to_char(buf, ch);
    }

    if (learn > 80) {

      num =   (int)GetApprox(GET_HITROLL(victim), learn);
      num2 =  ((int)21 - CalcThaco(ch));
      if (!num2) num2=1;
      if (num2 > 0)
	fnum = ((int)num/(int)num2);
      else
	fnum = 2.0;

      sprintf(buf, "Est. Thaco: %s\n\r", DescRatio(fnum));

      send_to_char(buf, ch);


      num =   GetApprox(GET_DAMROLL(victim), learn);
      num2 =  GET_DAMROLL(ch);
      if (!num2) num2=1;
      fnum = ((int)num/(int)num2);

      sprintf(buf, "Est. Dam bonus is: %s\n\r", DescRatio(fnum));

      send_to_char(buf, ch);

    }
#endif


  }
#endif

}

void do_spells(struct char_data *ch, char *argument, int cmd)
{
  int spl, i;   /* 16384 */
  char buf[MAX_STRING_LENGTH],tbuf[255];

dlog("in do_spells");

  if (IS_NPC(ch))    {
    send_to_char("You ain't nothin' but a hound-dog.\n\r", ch);
    return;
  }

  *buf=0;

 sprintf(buf + strlen(buf),
 "[# ] %-20s  MANA, Cl, Mu, Dr, Sc, Pa, Ra, Ps\n\r",
 "SPELL/SKILL\0");

  for (i = 1, spl = 0; i <= MAX_EXIST_SPELL; i++, spl++) {
    if (GetMaxLevel(ch) > LOW_IMMORTAL ||
	spell_info[i].min_level_cleric < ABS_MAX_LVL) {
if (!spells[spl]) {
	sprintf(tbuf,"!spells[spl] on %d, do_spells in act.info.c",i);
	log(tbuf);
} else
      sprintf(buf + strlen(buf),
"[%2d] %-20s  <%3d> %2d %3d %3d %3d %3d %3d %3d\n\r",
	      i, spells[spl],
	      spell_info[i].min_usesmana,
	      spell_info[i].min_level_cleric,
	      spell_info[i].min_level_magic,
	      spell_info[i].min_level_druid,
	      spell_info[i].min_level_sorcerer,
	      spell_info[i].min_level_paladin,
	      spell_info[i].min_level_ranger,
	      spell_info[i].min_level_psi
	      );
	}

  }
  strcat(buf, "\n\r");
  page_string(ch->desc, buf, 1);
}

void do_world(struct char_data *ch, char *argument, int cmd)
{
  char buf[1000];
  long ct, ot;
  char *tmstr, *otmstr;

dlog("in do_world");

  sprintf(buf, "$c0005Base Source: $c0014HavokMUD$c0005 Version $c0015%s.", VERSION);
  act(buf,FALSE, ch,0,0,TO_CHAR);
  ot = Uptime;
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "$c0005Start time was: $c0015%s $c0005(PST)", otmstr);
  act(buf,FALSE, ch,0,0,TO_CHAR);

  ct = time(0);
  tmstr = asctime(localtime(&ct));
  *(tmstr + strlen(tmstr) - 1) = '\0';
  sprintf(buf, "$c0005Current time is: $c0015%s $c0005(PST)", tmstr);
  act(buf,FALSE, ch,0,0,TO_CHAR);

if (GetMaxLevel(ch) >=LOW_IMMORTAL) {
   char tbuf[256];
   sprintbit((unsigned long)SystemFlags,system_flag_types,tbuf);
   sprintf(buf,"$c0005Current system settings :[$c0015%s$c0005]",tbuf);
  act(buf,FALSE,ch,0,0,TO_CHAR);
  }

#if HASH
  sprintf(buf, "$c0005Total number of rooms in world: $c0015%d", room_db.klistlen);
#else
  sprintf(buf, "$c0005Total number of rooms in world: $c0015%ld", room_count);
#endif
  act(buf,FALSE, ch,0,0,TO_CHAR);
  sprintf(buf, "$c0005Total number of zones in world: $c0015%d\n\r",
	  top_of_zone_table + 1);
  act(buf,FALSE, ch,0,0,TO_CHAR);
  sprintf(buf,"$c0005Total number of distinct mobiles in world: $c0015%d",
	  top_of_mobt + 1);
  act(buf,FALSE, ch,0,0,TO_CHAR);
  sprintf(buf,"$c0005Total number of distinct objects in world: $c0015%d\n\r",
	  top_of_objt + 1);
  act(buf,FALSE, ch,0,0,TO_CHAR);
  sprintf(buf,"$c0005Total number of registered players: $c0015%d",top_of_p_table + 1);
   act(buf,FALSE, ch,0,0,TO_CHAR);

  sprintf(buf, "$c0005Total number of monsters in game: $c0015%ld", mob_count);
  act(buf,FALSE, ch,0,0,TO_CHAR);

  sprintf(buf, "$c0005Total number of objects in game: $c0015%ld", obj_count);
  act(buf, FALSE,ch,0,0,TO_CHAR);

  sprintf(buf, "$c0005Total number of connections since last reboot: $c0015%ld",
  			total_connections );
  act(buf, FALSE,ch,0,0,TO_CHAR);

  sprintf(buf, "$c0005Max. # of players online since last reboot: $c0015%ld",
  			total_max_players );
  act(buf, FALSE,ch,0,0,TO_CHAR);


}

void do_resistances(struct char_data *ch, char *argument, int cmd) {
   int x;
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))

	send_to_char("\n\r$c0005Current resistances:\n\r--------------\n\r",ch);
	for(x=1;x<=BV13;x=x*2) {

	  if(IS_IMMUNE(ch,x))
	    	  ch_printf(ch,"$c000pYou are $c000CImmune $c000pto $c000C%s.\n\r",immunity_names[bitvector_num(x)]);
	  	else
	  	if(IsResist(ch, x))
	  	 	ch_printf(ch,"$c000pYou are $c000CResistant $c000pto $c000C%s.\n\r",immunity_names[bitvector_num(x)]);
	  	else
	   	  if(IsSusc(ch, x))
	  	  	ch_printf(ch,"$c000pYou are $c000CSusceptible$c000p to $c000C%s.\n\r",immunity_names[bitvector_num(x)]);
	  	  else
	  	  	ch_printf(ch,"$c000pYou are $c000CDefenseless$c000p to $c000C%s.\n\r",immunity_names[bitvector_num(x)]);
	}

}


void do_attribute(struct char_data *ch, char *argument, int cmd)
{
#define IS_IMMUNE(ch, bit) (IS_SET((ch)->M_immune, bit))
   char buf[MAX_STRING_LENGTH];
   struct affected_type *aff;
   struct time_info_data my_age;
   int i = 0, j2 = 0, Worn_Index = 0;
   short last_type;
   char buf2[MAX_STRING_LENGTH];
   struct obj_data *j=0;
   extern char *affected_bits[];
   extern char *affected_bits2[];
  dlog("in do_attrib");

   age2(ch, &my_age);

   sprintf(buf, "$c0005You are $c0014%d$c0005 years and $c0014%d$c0005 months, \
$c0014%d$c0005 cms, and you weigh $c0014%d$c0005 lbs.\n\r", my_age.year, my_age.month,
           ch->player.height, ch->player.weight);

   send_to_char(buf,ch);

   sprintf(buf, "$c0005You are carrying $c0014%d$c0005 lbs of equipment.\n\r",
           IS_CARRYING_W(ch));
   send_to_char(buf,ch);

   sprintf(buf,"$c0005You are$c0014 %s\n\r",ArmorDesc(ch->points.armor));
   send_to_char(buf,ch);

   if(GetMaxLevel(ch) > 5)
   {
      sprintf(buf,"$c0005You have $c0014%d$c0005/$c0015%d $c0005STR, $c0014%d $c0005INT, \
$c0014%d $c0005WIS, $c0014%d $c0005DEX, $c0014%d $c0005CON, $c0014%d $c0005CHR\n\r",
              GET_STR(ch), GET_ADD(ch), GET_INT(ch), GET_WIS(ch), GET_DEX(ch),
              GET_CON(ch), GET_CHR(ch));
      send_to_char(buf,ch);
   }

   sprintf(buf, "$c0005Your hit bonus and damage bonus are $c0014%s$c0005 and \
$c0014%s$c0005 respectively.\n\r",
           HitRollDesc(GET_HITROLL(ch)), DamRollDesc(GET_DAMROLL(ch)));
   send_to_char(buf,ch);



/* letrs say the resistances*/

send_to_char("\n\r$c0005Type 'Resist' to see a list of your resistances:\n\r",ch);


   /*
   **   by popular demand -- affected stuff
   */
   send_to_char("\n\r$c0005Current affects:\n\r--------------\n\r",ch);


   if(ch->affected)
   {
      for(aff = ch->affected; aff; aff = aff->next)
      {
         if (aff->type <= MAX_EXIST_SPELL)
         {
            switch(aff->type)
            {
               case SKILL_SNEAK:
               case SPELL_PRAYER:
               case SKILL_HIDE:
               case SKILL_QUIV_PALM:
               case SKILL_HUNT:
               case SKILL_DISGUISE:
               case SKILL_SWIM:
               case SKILL_SPY:
               case SKILL_FIRST_AID:
               case SKILL_LAY_ON_HANDS:
               case SKILL_MEDITATE:
                  break;
               case SKILL_MEMORIZE:
                  sprintf(buf, "$c0005Memorizing : '$c0014%s$c0005' will complete \
in $c0014%d $c0005minutes.\n\r", spells[aff->modifier-1], (aff->duration*4));
                  send_to_char(buf,ch);
                  break;

               default:
                  sprintf(buf, "$c0005Spell : '$c0014%s$c0005' will expire in \
$c0014%d $c0005hours.\n\r",spells[aff->type-1], aff->duration);
                  if(aff->type != last_type)
                     send_to_char(buf,ch);
                  break;
            }
            last_type = aff->type;
         }
      }
   }

   for(Worn_Index = j2=0; j2 < (MAX_WEAR - 1); j2++)
   {
      if(ch->equipment[j2])
      {
         j = ch->equipment[j2];
         for (i=0;i<MAX_OBJ_AFFECT;i++)
         {
            switch(j->affected[i].location)
            {
               case APPLY_SPELL:
                  sprintbit(j->affected[i].modifier,affected_bits,buf2);
                  if(strcmp(buf2, "NOBITS")==0)
                     break;
                  sprintf(buf,"$c0005Spell : '$c0014%s$c0005' granted though an item.\n\r", buf2);
                  send_to_char(buf, ch);
                  break;
               case APPLY_SPELL2:
                  sprintbit(j->affected[i].modifier,affected_bits2,buf2);
                  if(strcmp(buf2, "NOBITS")==0)
                     break;
                  sprintf(buf,"$c0005Spell : '$c0014%s$c0005' granted though an item.\n\r", buf2);
                  send_to_char(buf, ch);
                  break;
               default:
                  break;
            }
         }
      }
   }

}

void do_value(struct char_data *ch, char *argument, int cmd)
{
  char buf[1000],buf2[1000], name[1000];
  struct obj_data *obj=0;
  struct char_data *vict=0;

dlog("in do_value");

  /* Spell Names */


  /* For Objects */


  if (!HasClass(ch, CLASS_THIEF|CLASS_RANGER)) {
    send_to_char("Sorry, you can't do that here", ch);
    return;
  }

  argument = one_argument(argument, name);

  if ((obj = get_obj_in_list_vis(ch, name, ch->carrying))==0) {
    if ((vict = get_char_room_vis(ch, name))==0) {
      send_to_char("Who, or what are you talking about?\n\r", ch);
      return;
    } else {
      only_argument(argument, name);
      if ((obj = get_obj_in_list_vis(ch, name, vict->carrying))==0) {
	act("You can't see that on $M", FALSE, ch, obj, vict, TO_CHAR);
	act("$n looks you over", FALSE, ch, 0, vict, TO_VICT);
	act("$n looks $N over", FALSE, ch, 0, vict, TO_NOTVICT);
	return;
      }
    }
  }

  WAIT_STATE(ch, PULSE_VIOLENCE*2);

  if (!SpyCheck(ch)) {  /* failed spying check */
    if (obj && vict) {
      act("$n looks at you, and $s eyes linger on $p",
	  FALSE, ch, obj, vict, TO_VICT);
      act("$n studies $N",
	  FALSE, ch, 0, vict, TO_ROOM);

    } else if (obj) {
      act("$n intensely studies $p", FALSE, ch, obj, 0, TO_ROOM);
    } else{
      return;
    }
  }


  sprintf(buf, "Object: %s.  Item type: ", obj->short_description);
  sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
  strcat(buf,buf2); strcat(buf,"\n\r");
  send_to_char(buf, ch);

  if (!ch->skills) return;


  if (number(1,101) < ch->skills[SKILL_EVALUATE].learned/3) {
    if (obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbit((unsigned long)obj->obj_flags.bitvector,affected_bits,buf);
      strcat(buf,"\n\r");
      send_to_char(buf, ch);
    }
  }

  if (number(1,101) < ch->skills[SKILL_EVALUATE].learned/2) {
    send_to_char("Item is: ", ch);
    sprintbit((unsigned long) obj->obj_flags.extra_flags,extra_bits,buf);
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
  }

  sprintf(buf,"Weight: %d, Value: %d, Rent cost: %d  %s\n\r",
	    obj->obj_flags.weight,
	  GetApprox(obj->obj_flags.cost,
		    ch->skills[SKILL_EVALUATE].learned-10),
	  GetApprox(obj->obj_flags.cost_per_day,
		    ch->skills[SKILL_EVALUATE].learned-10), IS_RARE(obj)?"[RARE]":" ");
  send_to_char(buf, ch);

  if (ITEM_TYPE(obj) == ITEM_WEAPON) {
    sprintf(buf, "Damage Dice is '%dD%d'\n\r",
	    GetApprox(obj->obj_flags.value[1],
		      ch->skills[SKILL_EVALUATE].learned-10),
	    GetApprox(obj->obj_flags.value[2],
		      ch->skills[SKILL_EVALUATE].learned-10));
    send_to_char(buf, ch);
  } else if (ITEM_TYPE(obj) == ITEM_ARMOR) {

      sprintf(buf, "AC-apply is %d\n\r",
	      GetApprox(obj->obj_flags.value[0],
			ch->skills[SKILL_EVALUATE].learned-10));
      send_to_char(buf, ch);
  }
}

char *AlignDesc(int a)
{
  if (a <= -900) {
    return("Chaotic Evil");
  } else if (a <= -500) {
    return("Neutral Evil");
  } else if (a <= -351) {
    return("Lawful Evil");
  } else if (a <= -100) {
    return("Chaotic Neutral");
  } else if (a <= 100) {
    return("True Neutral");
  } else if (a <= 350) {
    return("Lawful Neutral");
  } else if (a <= 500) {
    return("Chaotic Good");
  } else if (a <= 900) {
    return("Neutral Good");
  } else{
    return("Lawful Good");
  }
}


char *ArmorDesc(int a)
{
  if (a >= 90) {
    return("barely armored");
  } else if (a >= 50) {
    return("Lightly armored");
  } else if (a >= 30) {
    return("Medium-armored");
  } else if (a >= 10) {
    return("Fairly well armored");
  } else if (a >= -10) {
    return("Well armored");
  } else if (a >= -30) {
    return("Quite well armored");
  } else if (a >= -50) {
    return("Very well armored");
  } else if (a >= -90) {
    return("Extremely well armored");
  } else {
    return("armored like a Dragon");
  }
}

char *HitRollDesc(int a)
{
  if (a < -5) {
    return("Quite bad");
  } else if (a < -1) {
    return("Pretty lousy");
  } else if (a <= 1) {
    return("Not Much of one");
  } else if (a < 3) {
    return("Not bad");
  } else if (a < 8) {
	return ("Pretty good");
  } else if (a < 15) {
    return("Damn good");
  } else if (a < 30) {
    return("Very good");
  } else if (a < 40) {
  	return("Extremely good");
  } else
  	return("Magnificent");
}
char *DamRollDesc(int a)
{
   if (a < -5) {
     return("Quite bad");
   } else if (a < -1) {
     return("Pretty lousy");
   } else if (a <= 1) {
     return("Not Much of one");
   } else if (a < 3) {
     return("Not bad");
   } else if (a < 8) {
 	return ("Pretty good");
   } else if (a < 15) {
     return("Damn good");
   } else if (a < 30) {
     return("Very good");
   } else if (a < 40) {
   	return("Extremely good");
   } else
   	return("Magnificent");
}
char *DescRatio(float f)  /* theirs / yours */
{
  if (f > 1.0) {
    return("More than twice yours");
  } else if (f > .75) {
    return("More than half again greater than yours");
  } else if (f > .6) {
    return("At least a third greater than yours");
  } else if (f > .4) {
    return("About the same as yours");
  } else if (f > .3) {
    return("A little worse than yours");
  } else if (f > .1) {
    return("Much worse than yours");
  } else {
    return("Extremely inferior");
  }
}

char *DescDamage(float dam)
{
  if (dam < 1.0) {
    return("Minimal Damage");
  } else if (dam <= 2.0) {
    return("Slight damage");
  } else if (dam <= 4.0) {
    return("A bit of damage");
  } else if (dam <= 10.0) {
    return("A decent amount of damage");
  } else if (dam <= 15.0) {
    return("A lot of damage");
  } else if (dam <= 25.0) {
    return("A whole lot of damage");
  } else if (dam <= 35.0) {
    return("A very large amount");
  } else {
    return("A TON of damage");
  }
}

char *DescAttacks(float a)
{
  if (a < 1.0) {
    return("Not many");
  } else if (a < 2.0) {
    return("About average");
  } else if (a < 3.0) {
    return("A few");
  } else if (a < 5.0) {
    return("A lot");
  } else if (a < 9.0) {
    return("Many");
  } else {
    return("A whole bunch");
  }
}

char *SpeedDesc(int a)
{

  if (a < 10) {
    return("Very Slow");
  } else if (a < 20) {
    return("Slow");
  } else if (a < 30) {
    return("About Average");
  } else if (a < 40) {
    return("Fast");
  } else if (a < 50) {
    return("Very Fast");
  } else {
    return("Lightning Quick");
  }
}


char *ArmorSize(int a)
{

  if (a == 0) {
    return("Adjustable");
  } else if (a == 1) {
    return("Tiny");
  } else if (a == 2) {
    return("Small");
  } else if (a == 3) {
    return("Medium");
  } else if (a == 4) {
    return("Large");
  } else if (a == 5) {
	return("Huge");
  } else if (a == 6) {
	return("Enormous");
  } else {
    return("Invalid Size");
  }
}



void do_display(struct char_data *ch, char *arg, int cmd)
{
 int i;

dlog("in do_display");

 if(IS_NPC(ch))
    return;

 i = atoi(arg);

 switch(i) {
 case 0: if(ch->term == 0) {
	    send_to_char("Display unchanged.\n\r", ch);
	    return;
	  }
	  ch->term = 0;
	  ScreenOff(ch);
	  send_to_char("Display now turned off.\n\r", ch);
	  return;

 case 1: if(ch->term == 1) {
	    send_to_char("Display unchanged.\n\r", ch);
	    return;
	   }
	  ch->term = VT100;
	  InitScreen(ch);
	  send_to_char("Display now set to VT100.\n\r", ch);
	  return;

 default: if(ch->term == VT100) {
	    send_to_char("Term type is currently VT100.\n\r", ch);
	    return;
	  }
	  send_to_char("Display is currently OFF.\n\r", ch);
	  return;
 }
}

void ScreenOff(struct char_data *ch)
{
 char buf[255];

 sprintf(buf, VT_MARGSET, 0, ch->size- 1);
 send_to_char(buf, ch);
 send_to_char(VT_HOMECLR, ch);
}

void do_resize(struct char_data *ch, char *arg, int cmd)
{
 int i;

dlog("in do_resize");

 if(IS_NPC(ch))
   return;

 i = atoi(arg);

 if(i < 7) {
    send_to_char("Screen size must be greater than 7.\n\r", ch);
    return;
  }

if (i > 50) {
  send_to_char("Size must be smaller than 50.\n\r",ch);
  return;
}

 ch->size = i;

 if(ch->term == VT100) {
    ScreenOff(ch);
    InitScreen(ch);
  }

 send_to_char("Ok.\n\r", ch);
 return;
}

int MobLevBonus(struct char_data *ch)
{
  int t=0;

  if (mob_index[ch->nr].func == magic_user)
    t+=5;
  if (mob_index[ch->nr].func == BreathWeapon)
    t+=7;
  if (mob_index[ch->nr].func == fighter)
    t+=3;
  if (mob_index[ch->nr].func == snake)
    t+=3;

  t+=(ch->mult_att-1)*3;

  if (GET_HIT(ch) > GetMaxLevel(ch)*8)
    t+=1;
  if (GET_HIT(ch) > GetMaxLevel(ch)*12)
    t+=2;
  if (GET_HIT(ch) > GetMaxLevel(ch)*16)
    t+=3;
  if (GET_HIT(ch) > GetMaxLevel(ch)*20)
    t+=4;

  return(t);
}

void do_show_skill(struct char_data *ch, char *arg, int cmd)
{
  char buf[254], buffer[MAX_STRING_LENGTH];
  int i=0;

dlog("in do_show_skill");

  buffer[0]='\0';

  if (!ch->skills)
    return;

  for (; isspace(*arg); arg++);

  if (!arg)   {
    send_to_char("You need to supply a class for that.",ch);
    return;
  }

  switch(*arg) {
  case 'w':
  case 'W':
  case 'f':
  case 'F':
    {
		if (!HasClass(ch, CLASS_WARRIOR)) {
			send_to_char("I bet you think you're a fighter.\n\r", ch);
			return;
		}
		send_to_char("You can learn these warrior skills:\n\r", ch);
		while(warriorskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",warriorskills[i].level,
						warriorskills[i].name,how_good(ch->skills[warriorskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[warriorskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		i = 0;
		if(OnlyClass(ch,CLASS_WARRIOR)) {
			while(scwarskills[i].level != -1) {
				sprintf(buf,"[%-2d] %-30s %-15s",scwarskills[i].level,
							scwarskills[i].name,how_good(ch->skills[scwarskills[i].skillnum].learned));
				if (IsSpecialized(ch->skills[scwarskills[i].skillnum].special))
					strcat(buf," (special)");
				strcat(buf," \n\r");
				if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
					break;
				strcat(buffer, buf);
				strcat(buffer, "\r");
				i++;
			}
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;

	case 't':
	case 'T': {
		if (!HasClass(ch, CLASS_THIEF)) {
			send_to_char("I bet you think you're a thief.\n\r", ch);
			return;
		}
		send_to_char("Your class can learn these skills:\n\r\n\r", ch);
		while(thiefskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",thiefskills[i].level, thiefskills[i].name,
				/* kludged a bit to get rid of the dex bonus to five skills */
				(IS_SET(ch->skills[thiefskills[i].skillnum].flags,SKILL_KNOWN)) ?
				(how_good(ch->skills[thiefskills[i].skillnum].learned)):" (not learned)");
			if (IsSpecialized(ch->skills[thiefskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;
	case 'M':
	case 'm': {
		if (!HasClass(ch, CLASS_MAGIC_USER)) {
			send_to_char("I bet you think you're a magic-user.\n\r", ch);
			return;
		}
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(mageskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",mageskills[i].level,
						mageskills[i].name,how_good(ch->skills[mageskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[mageskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
	break;
  case 'C':
  case 'c':
    {
      if (!HasClass(ch, CLASS_CLERIC)) {
	send_to_char("I bet you think you're a cleric.\n\r", ch);
	return;
      }
		while(clericskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",clericskills[i].level,
						clericskills[i].name,how_good(ch->skills[clericskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[clericskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  case 'D':
  case 'd':
    {
      if (!HasClass(ch, CLASS_DRUID)) {
	send_to_char("I bet you think you're a druid.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(druidskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",druidskills[i].level,
						druidskills[i].name,how_good(ch->skills[druidskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[druidskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;

  case 'K':
  case 'k': {
      if (!HasClass(ch, CLASS_MONK)) {
	send_to_char("I bet you think you're a monk.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(monkskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",monkskills[i].level, monkskills[i].name,
				/* kludged a bit to get rid of the dex bonus to five skills */
				(IS_SET(ch->skills[monkskills[i].skillnum].flags,SKILL_KNOWN)) ?
				(how_good(ch->skills[monkskills[i].skillnum].learned)):" (not learned)");
			if (IsSpecialized(ch->skills[monkskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;

  case 'b':
  case 'B':
    {
      if (!HasClass(ch, CLASS_BARBARIAN)) {
	send_to_char("I bet you think you're a Barbarian.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(barbskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",barbskills[i].level,
						barbskills[i].name,how_good(ch->skills[barbskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[barbskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;

  case 'a':
  case 'A':
    {
      if (!HasClass(ch, CLASS_BARD)) {
	send_to_char("I bet you think you're a Bard.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(bardskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",bardskills[i].level,
						bardskills[i].name,how_good(ch->skills[bardskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[bardskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;

  case 'S':
  case 's':
    {
      if (!HasClass(ch, CLASS_SORCERER)) {
	send_to_char("I bet you think you're a sorcerer.\n\r", ch);
	return;
      }
		while(sorcskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",sorcskills[i].level,
						sorcskills[i].name,how_good(ch->skills[sorcskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[sorcskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
      page_string(ch->desc, buffer, 1);
      return;
    }
    break;


  case 'p':
  case 'P': {
      if (!HasClass(ch, CLASS_PALADIN)) {
	send_to_char("I bet you think you're a paladin.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these skills:\n\r", ch);
		while(paladinskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",paladinskills[i].level,
						paladinskills[i].name,how_good(ch->skills[paladinskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[paladinskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;


  case 'R':
	case 'r': {
		if (!HasClass(ch, CLASS_RANGER)) {
			send_to_char("I bet you think you're a ranger.\n\r", ch);
			return;
		}
		send_to_char("Your class can learn these spells:\n\r", ch);
		while(rangerskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",rangerskills[i].level,
						rangerskills[i].name,how_good(ch->skills[rangerskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[rangerskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;


  case 'i':
  case 'I': {
      if (!HasClass(ch, CLASS_PSI)) {
	send_to_char("I bet you think you're a psionist.\n\r", ch);
	return;
      }
		send_to_char("Your class can learn these spells:\n\r", ch);
		while(psiskills[i].level != -1) {
			sprintf(buf,"[%-2d] %-30s %-15s",psiskills[i].level,
						psiskills[i].name,how_good(ch->skills[psiskills[i].skillnum].learned));
			if (IsSpecialized(ch->skills[psiskills[i].skillnum].special))
				strcat(buf," (special)");
			strcat(buf," \n\r");
			if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
				break;
			strcat(buffer, buf);
			strcat(buffer, "\r");
			i++;
		}
		page_string(ch->desc, buffer, 1);
		return;
	}
    break;

  default:
    send_to_char("Which class? (skill [m s c w t d r p k i a])\n\r", ch);
  }

}




/* this command will only be used for immorts as I am using it as a way */
/* to figure out how to look into rooms next to this room. Will be using*/
/* the code for throwing items. I figure there is no IC reason for a PC */
/* to have a command like this. Do what ya want on your on MUD          */
void do_spot(struct char_data *ch, char *argument, int cmd)
{
   static char *keywords[]= {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"};
   char *dir_desc[] = {
      "to the north",
      "to the east",
      "to the south",
      "to the west",
      "upwards",
      "downwards"};
   char *rng_desc[] = {
      "right here",
      "immediately",
      "nearby",
      "a ways",
      "a ways",
      "far",
      "far",
      "very far",
      "very far"};
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
   int sd, smin, smax, swt, i, max_range = 6, range, rm, nfnd;
   struct char_data *spud;
	//char *PowerLevelDesc(long a);

dlog("in do_scan");

/*
   sprintf(buf,"In scan - Room #%d, %s scanning.", ch->in_room,GET_NAME(ch));
   slog(buf);
*/
	/*
		Check mortals spot skill, and give THEM a max scan of
		2 rooms.
	*/

	if (!ch->skills) {
		send_to_char("You do not have skills!\n\r",ch);
		return;
	    }

	if (GetMaxLevel(ch)<LOW_IMMORTAL) {
	   if (!ch->skills[SKILL_SPOT].learned)   {
		 send_to_char ("You have not been trained to spot.\n\r",ch);
		 return;
		}

	      if (dice(1,101) > ch->skills[SKILL_SPOT].learned)      {
		send_to_char("Absolutely no-one anywhere.\n\r",ch);
		WAIT_STATE(ch,2);
		return;
	      } /* failed */

	max_range=2;    /* morts can only spot two rooms away */

	}       /* was mortal */

   argument_split_2(argument,arg1,arg2);
   sd = search_block(arg1, keywords, FALSE);
   if (sd==-1) {
      smin = 0;
      smax = 5;
      swt = 3;
      sprintf(buf,"$n peers intently all around.");
      sprintf(buf2,"You peer intently all around, and see :\n\r");
   } else {
      smin = sd;
      smax = sd;
      swt = 1;
      sprintf(buf,"$n peers intently %s.",dir_desc[sd]);
      sprintf(buf2,"You peer intently %s, and see :\n\r",dir_desc[sd]);
   }

   act(buf, FALSE, ch, 0, 0, TO_ROOM);
   send_to_char(buf2,ch);
   nfnd = 0;
   /* Check in room first */
   for (spud=real_roomp(ch->in_room)->people;spud;spud=spud->next_in_room) {
      if ((CAN_SEE(ch,spud))&&(!IS_SET(spud->specials.affected_by,AFF_HIDE))&&(spud!=ch)) {
	 if (IS_NPC(spud)) {
	    sprintf(buf,"%30s : right here\n\r",spud->player.short_descr);
	 } else {
	    sprintf(buf,"%30s : right here\n\r",GET_NAME(spud));
	 }
	 send_to_char(buf,ch);
	 nfnd++;
      }
   }
   for (i=smin;i<=smax;i++) {
      rm = ch->in_room;
      range = 0;
      while (range<max_range) {
	 range++;
	 if (clearpath(ch, rm,i)) {
	    rm = real_roomp(rm)->dir_option[i]->to_room;
	    for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
	       if ((CAN_SEE(ch,spud))&&(!(IS_SET(spud->specials.affected_by,AFF_HIDE)))) {
		  if (IS_NPC(spud)) {
		     sprintf(buf,"%30s : %s %s\n\r",spud->player.short_descr,rng_desc[range],dir_desc[i]);
		  } else {
		     sprintf(buf,"%30s : %s %s\n\r",GET_NAME(spud),rng_desc[range],dir_desc[i]);
		  }
		  send_to_char(buf,ch);
		  nfnd++;
	       }
	    }
	 } else {
	    range = max_range + 1;
	 }
      }
    }
    if (nfnd==0) send_to_char("Absolutely no-one anywhere.\n\r",ch);
    WAIT_STATE(ch,swt);
}

void do_scan(struct char_data *ch, char *argument, int cmd)
{
   static char *keywords[]= {
	"north",
	"east",
	"south",
	"west",
	"up",
	"down",
	"\n"};
   char *dir_desc[] = {
      "to the north",
      "to the east",
      "to the south",
      "to the west",
      "upwards",
      "downwards"};
   char *rng_desc[] = {
      "right here",
      "immediately",
      "nearby",
      "a ways",
      "a ways",
      "far",
      "far",
      "very far",
      "very far"};
   char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
   char arg1[MAX_STRING_LENGTH], arg2[MAX_STRING_LENGTH];
   int sd, smin, smax, swt, i, max_range = 6, range, rm, nfnd;
   struct char_data *spud;
	long CalcPowerLevel(struct char_data *ch);
	char *PowerLevelDesc(long a);
dlog("in do_scan");

/*
   sprintf(buf,"In scan - Room #%d, %s scanning.", ch->in_room,GET_NAME(ch));
   slog(buf);
*/
	/*
		Check mortals spot skill, and give THEM a max scan of
		2 rooms.
	*/

	if (!ch->skills) {
		send_to_char("You do not have skills!\n\r",ch);
		return;
	    }

	if (GetMaxLevel(ch)<LOW_IMMORTAL) {
	   if (!ch->skills[SKILL_SPOT].learned)   {
		 send_to_char ("You have not been trained to spot.\n\r",ch);
		 return;
		}

	      if (dice(1,101) > ch->skills[SKILL_SPOT].learned)      {
		send_to_char("Absolutely no-one anywhere.\n\r",ch);
		WAIT_STATE(ch,2);
		return;
	      } /* failed */

	max_range=2;    /* morts can only spot two rooms away */

	}       /* was mortal */

   argument_split_2(argument,arg1,arg2);
   sd = search_block(arg1, keywords, FALSE);

   //only_argument(argument, name);
   //Find char in room
     if ((spud = get_char_room_vis(ch, arg1))) {

  		sprintf(buf,"$n peers intently at $N.");
  		sprintf(buf2,"You peer intently at $N.  You sense an aura power of %ld",CalcPowerLevel(spud));
  		act(buf, FALSE, ch,0, spud, TO_ROOM);
  		act(buf2, FALSE, ch,0, spud, TO_CHAR);

		return;
	}
   if (sd==-1) {
      smin = 0;
      smax = 5;
      swt = 3;
      sprintf(buf,"$n peers intently all around.");
      sprintf(buf2,"You peer intently all around, and see :\n\r");
   } else {
      smin = sd;
      smax = sd;
      swt = 1;
      sprintf(buf,"$n peers intently %s.",dir_desc[sd]);
      sprintf(buf2,"You peer intently %s, and see :\n\r",dir_desc[sd]);
   }

   act(buf, FALSE, ch, 0, 0, TO_ROOM);
   send_to_char(buf2,ch);
   nfnd = 0;
   /* Check in room first */
   for (spud=real_roomp(ch->in_room)->people;spud;spud=spud->next_in_room) {
      if ((CAN_SEE(ch,spud))&&(!IS_SET(spud->specials.affected_by,AFF_HIDE))&&(spud!=ch)) {
	 if (IS_NPC(spud)) {
	    sprintf(buf,"You sense a %s aura here coming from %s.\n\r"
	    ,PowerLevelDesc(CalcPowerLevel(spud)),spud->player.short_descr);
	 } else {
	    sprintf(buf,"You sense a %s aura here coming from %s.\n\r"
	    	,PowerLevelDesc(CalcPowerLevel(spud)),GET_NAME(spud));
	 }
	 send_to_char(buf,ch);
	 nfnd++;
      }
   }
   for (i=smin;i<=smax;i++) {
      rm = ch->in_room;
      range = 0;
      while (range<max_range) {
	 range++;
	 if (clearpath(ch, rm,i)) {
	    rm = real_roomp(rm)->dir_option[i]->to_room;
	    for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
	       if ((CAN_SEE(ch,spud))&&(!(IS_SET(spud->specials.affected_by,AFF_HIDE)))) {
		  if (IS_NPC(spud)) {
		     sprintf(buf,"You sense a %s aura %s %s.\n\r"
		     ,PowerLevelDesc(CalcPowerLevel(spud)),rng_desc[range],dir_desc[i]);
			//spud->player.short_descr

		     //if (IS_IMMORTAL(ch))
		  	//	sprintf(buf,"[%s]->%s",buf,spud->player.short_descr);
		   } else {
		     sprintf(buf,"You sense a %s aura %s %s.\n\r"
		     ,PowerLevelDesc(CalcPowerLevel(spud)), rng_desc[range], dir_desc[i]);

		     //if (IS_IMMORTAL(ch))
		  	//	sprintf(buf,"[%s]->%s",buf,GET_NAME(spud));
		  }
		  send_to_char(buf,ch);
		  nfnd++;
	       }
	    }
	 } else {
	    range = max_range + 1;
	 }
      }
    }
    if (nfnd==0) send_to_char("Absolutely no-one anywhere.\n\r",ch);
       WAIT_STATE(ch,swt);
}

/* Calculates a char or mobs powerlevel
 * @ returns :powerlevel
 * @ param :Character struct
 * @ Author :Banon (GH)
 * @ Date :May 16, 2002
 * maxlevel * Hitroll * damroll * Numatts + HIT*100
 */
long CalcPowerLevel(struct char_data *ch)  {
	long power;
	extern struct str_app_type str_app[];
	int hitroll, damroll;
		if(IS_IMMORTAL(ch))
			power = 2000000;
		else power = 0;

 		hitroll = ch->points.hitroll+str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
 		damroll = ch->points.damroll+str_app[STRENGTH_APPLY_INDEX(ch)].todam;


	power = power + GetMaxLevel(ch) * hitroll * damroll * ch->mult_att + GET_HIT(ch)*100;
	if (IS_AFFECTED(ch,AFF_SANCTUARY))
		power = power * 2;


	return power;
}
/*
 *
 *
 *
 * You sense a <Godlike> aura nearby to the north.
 */

char *PowerLevelDesc(long a)
{


   if (a < 5000) {
     return("very weak");
   } else if (a < 25000) {
     return("weak");
   } else if (a <= 56000) {
     return("small");
   } else if (a < 130000) {
     return("medium");
   } else if (a < 210000) {
 	return ("strong");
   } else if (a < 400000) {
     return("very strong");
   } else if (a < 500000) {
     return("powerful");
   } else if (a < 600000) {
   	return("extremely powerful");
   } else
   	return("Godlike");
}

void list_groups(struct char_data *ch)
{
  struct descriptor_data *i;
  struct char_data *person;
  struct follow_type *f;
  int count = 0;
  char buf[MAX_STRING_LENGTH*2],tbuf[MAX_STRING_LENGTH];


  sprintf(buf,"$c0015[------- Adventuring Groups -------]\n\r");

  /* go through the descriptor list */
  for (i = descriptor_list;i;i=i->next) {
  /* find everyone who is a master  */
    if (!i->connected) {
      person = (i->original ? i->original:i->character);

  /* list the master and the group name */
      if (!person->master && IS_AFFECTED(person, AFF_GROUP)) {
	if (person->specials.group_name && CAN_SEE(ch, person)) {

sprintf(tbuf, "          $c0015%s\n\r$c0014%s\n\r",person->specials.group_name,
		fname (GET_NAME(person)));
strcat(buf,tbuf);

	  /* list the members that ch can see */
	  count = 0;
	  for(f=person->followers; f; f=f->next) {
	    if (IS_AFFECTED(f->follower, AFF_GROUP) && IS_PC(f->follower)) {
	      count++;
	      if (CAN_SEE(ch, f->follower) && strlen(GET_NAME(f->follower))>1 ) {
	      sprintf(tbuf,"$c0013%s\n\r", fname(GET_NAME(f->follower)));
	      strcat(buf,tbuf);
	      } else {
		sprintf(tbuf,"$c0013Someone\n\r");
		strcat(buf,tbuf);
	      }
	    }
	  }
  /* if there are no group members, then remove the group title */
	  if (count < 1) {
    send_to_char("Your group name has been removed, your group is too small\n\r",person);
	if (person->specials.group_name)
	    free(person->specials.group_name);
	    person->specials.group_name=0;
	  }
	}
      }
    }
  }
 strcat(buf,"\n\r$c0015[---------- End List --------------]\n\r");
 page_string(ch->desc,buf,1);
}

int can_see_linear(struct char_data *ch, struct char_data *targ, int *rng, int *dr)
{
   int i, rm, max_range = 6, range = 0;
   struct char_data *spud;

   for (i=0;i<6;i++) {
      rm = ch->in_room;
      range = 0;
      while (range<max_range) {
	 range++;
	 if (clearpath(ch, rm,i)) {
	    rm = real_roomp(rm)->dir_option[i]->to_room;
	    for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
	       if ((spud==targ)&&(CAN_SEE(ch,spud))) {
		  *rng = range;
		  *dr = i;
		  return i;
	       }
	    }
	 }
      }
   }
   return -1;
}

struct char_data *get_char_linear(struct char_data *ch, char *arg, int *rf, int *df)
/* Returns direction if can see, -1 if not */
{
   int i, rm, max_range = 6, range = 0, n, n_sofar = 0;
   struct char_data *spud;
   char *tmp, tmpname[MAX_STRING_LENGTH];

   strcpy(tmpname, arg);
   tmp = tmpname;
   if (!(n = get_number(&tmp))) return NULL;

   rm = ch->in_room;
   i = 0;
   range = 0;
   for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
      if ((isname(tmp, GET_NAME(spud)))&&(CAN_SEE(ch,spud))) {
	 n_sofar++;
	 if (n_sofar==n) {
	    *rf = range;
	    *df = i;
	    return spud;
	 }
      }
   }

   for (i=0;i<6;i++) {
      rm = ch->in_room;
      range = 0;
      while (range<max_range) {
	 range++;
	 if (clearpath(ch, rm,i)) {
	    rm = real_roomp(rm)->dir_option[i]->to_room;
	    for (spud=real_roomp(rm)->people;spud;spud=spud->next_in_room) {
	       if ((isname(tmp, GET_NAME(spud)))&&(CAN_SEE(ch,spud))) {
		  n_sofar++;
		  if (n_sofar==n) {
		     *rf = range;
		     *df = i;
		     return spud;
		  }
	       }
	    }
	 } else {
	    range = max_range+1;
	 }
      }
   }
   return NULL;
}

void do_glance(struct char_data *ch, char *argument, int cmd)
{
char arg[MAX_INPUT_LENGTH+30];
int bits;
struct char_data *tmp_char;
struct obj_data *found_object;

only_argument(argument, arg);

 if (*arg) {
	bits = generic_find(arg, FIND_CHAR_ROOM, ch, &tmp_char, &found_object);
	if (tmp_char) {
	  glance_at_char(tmp_char, ch);
	  if (ch != tmp_char) {
	    act("$n glances at you.", TRUE, ch, 0, tmp_char, TO_VICT);
	    act("$n glances at $N.", TRUE, ch, 0, tmp_char, TO_NOTVICT);
	  }
	  return;
	} else {
	    send_to_char ("They are not here...", ch);
	    return;
	  }
     } else {
        send_to_char("Try to glance at someone...", ch);
        return;
      }
   }

void do_whoarena(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *d;
  struct char_data *person;
  char buffer[MAX_STRING_LENGTH*3]="",tbuf[1024];
  int count;
  char color_cnt=1;

  char name_mask[40]="";
  char tmpname1[80],tmpname2[80];
  char levels[40]="";
  extern char *classname[];
  int i,total,classn; long bit;
dlog("in do_whoarena");

      sprintf(buffer,"$c0005                        Havok Arena Players\n\r");
      strcat(buffer,       "                           -------------\n\r");

    count=0;
    for (d = descriptor_list; d; d = d->next) {
      person=(d->original?d->original:d->character);
      if (CAN_SEE(ch, d->character) &&
	  (real_roomp(person->in_room)) &&
	  (real_roomp(person->in_room)->zone == 124)) {
	if (OK_NAME(person,name_mask)) {
	  count++;
    	  color_cnt = (color_cnt++ % 9);  /* range 1 to 9 */
#if 1
	   if(!IS_IMMORTAL(person)) {
         char classes[20]="";
	      for(bit=1,i=total=classn=0;i<BARD_LEVEL_IND+1;i++, bit<<=1) {

		if(HasClass(person,bit)) {
/*                  if(strlen(levels)!=0) strcat(levels,"/");
		  sprintf(levels+strlen(levels),"%d",person->player.level[i]);*/
		  classn++; total+=person->player.level[i];
		  if(strlen(classes)!=0) strcat(classes,"/");
		  sprintf(classes+strlen(classes),"%s",classname[i]);
		}
	      }
	     if (total <=0)
		total =1;
	if (classn <= 0 )
		classn =1;
	      total/=classn;
		   if(GetMaxLevel(person)==50) strcpy (levels,"$c0012Hero");
	      else if(total<11) strcpy(levels,"$c0008Apprentice");
	      else if(total<21) strcpy(levels,"$c0004Pilgrim");
	      else if(total<31) strcpy(levels,"$c0006Explorer");
	      else if(total<41) strcpy(levels,"$c0014Adventurer");
	      else if(total<51) strcpy(levels,"$c0015Mystical");
              sprintf(tbuf, "%s $c0012%s",levels, classes);
              /*(GH)Uncommented.. does it do anything? */sprintf(levels,"%32s","");
              strcpy(levels+10-((strlen(tbuf)-12)/2),tbuf);
              sprintf(tbuf, "%-32s $c0005: $c0007%s",levels,
                      person->player.title?person->player.title:GET_NAME(person));//"(Null)");
	    }
	    else {
	       switch(GetMaxLevel(person)) {
		case 51: sprintf(levels, "Lesser Deity"); break;
		case 52: sprintf(levels,"Deity"); break;
		case 53: sprintf(levels, "Greater Deity"); break;
		case 54:  if (GET_SEX(person) == SEX_MALE){       //if loop checks if the person
				sprintf(levels, "Lesser God");     //is female or men
				break;                             //modified by Thyrza 20/06/97
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Lesser Goddess");
				break;
				}

		case 55:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "God");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Goddess");
				break;
				}

		case 56:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "Greater God");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Greater Goddess");
				break;
				}


		case 57:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "God of Judgement");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Goddess of Judgement");
				break;
				}

		case 58:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "Lord");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Lady");
				break;
				}

		case 59:  if (GET_SEX(person) == SEX_MALE){
				sprintf(levels, "Supreme Lord");
				break;
				}
			  else if (GET_SEX(person) == SEX_FEMALE){
				sprintf(levels, "Supreme Lady");
				break;
				}

		case 60: sprintf(levels, "Supreme Being");
	      }
      	if(!str_cmp(GET_NAME(person), "Meckyl"))
      		//sprintf(levels, "");
            *levels = '\0';

            sprintf(tbuf, "%s",levels);
              sprintf(levels,"%30s","");
              strcpy(levels+10-(strlen(tbuf)/2),tbuf);
              if(real_roomp(ch->in_room)->zone == 124) {
                sprintf(tbuf, "$c0011%-20s $c0005: $c0007%s",levels,
                      person->player.title?person->player.title:GET_NAME(person));//"(Null)");
	  		  } else {
                 sprintf(tbuf, "$c0011%-20s $c0005: $c0007%s",levels,GET_NAME(person)
                      );


				switch(GET_POS(person)) {
				  case POSITION_DEAD :
				  	sprintf(tbuf,"%s is dead", tbuf);
				  	break;
				  case POSITION_MORTALLYW :
				    sprintf(tbuf,"%s is mortally wounded", tbuf);
				    break;
				  case POSITION_INCAP :
				    sprintf(tbuf,"%s is incapitated", tbuf);
				    break;
				  case POSITION_STUNNED :
				    sprintf(tbuf,"%s is stunned", tbuf);
				    break;
				  case POSITION_SLEEPING :
				  	sprintf(tbuf,"%s is sleeping", tbuf);
				    break;
				  case POSITION_RESTING  :
				    sprintf(tbuf,"%s is resting", tbuf);
				    break;
				  case POSITION_SITTING  :
				    sprintf(tbuf,"%s is sitting", tbuf);
				    break;
				  case POSITION_STANDING :
				    sprintf(tbuf,"%s is standing", tbuf);
				    break;
				  default :
				      sprintf(tbuf,"%s", tbuf);break;
				  }


				if (person->specials.fighting)
					sprintf(tbuf,"%s and fighting %s.", tbuf, GET_NAME(person->specials.fighting));

                sprintf(tbuf,"%-40s [HP:%2.0f%% MANA:%2.0f%% MV:%2.0f%%]",tbuf,
			    ((float)GET_HIT(person)  / (int)GET_MAX_HIT(person))  * 100.0 + 0.5,
			    ((float)GET_MANA(person) / (int)GET_MAX_MANA(person)) * 100.0 + 0.5,
			    ((float)GET_MOVE(person) / (int)GET_MAX_MOVE(person)) * 100.0 + 0.5);



			 }

	    }
#else
	    sprintf(tbuf, "$c100%d%s", color_cnt,
		      person->player.title?person->player.title:GET_NAME(person));//"(Null)");
#endif
	  }
	  if(IS_AFFECTED2(person,AFF2_AFK))
	      sprintf(tbuf+strlen(tbuf),"$c0008 [AFK] $c0007");
	    if(IS_AFFECTED2(person,AFF2_QUEST))
	      sprintf(tbuf+strlen(tbuf),"$c0008 [$c000RQ$c000Yu$c000Ge$c000Bs$c000Ct$c0008]$c0007");
	if (IS_LINKDEAD(person))
		sprintf(tbuf+strlen(tbuf),"$c0015 [LINKDEAD] $c0007");
	if (IS_IMMORTAL(ch) && person->invis_level > 50)
		sprintf(tbuf+strlen(tbuf), "(invis)");
	    sprintf(tbuf+strlen(tbuf),"\n\r");
	    if (strlen(buffer)+strlen(tbuf) < (MAX_STRING_LENGTH*2)-512)
	      strcat(buffer,tbuf);
	}
      }

  sprintf(tbuf, "\n\r$c0005Total visible arena players: $c0015%d\n\r", count);
  if (strlen(buffer)+strlen(tbuf) < MAX_STRING_LENGTH*2-512)
      strcat(buffer,tbuf);
  page_string(ch->desc,buffer,TRUE);
}

void do_map(struct char_data *ch, char *argument, int cmd) {

send_to_char("DO_MAP",ch);

}

/* clanlist command, shows which clans exist, or who is in what in which clan
 * Lennya 20030611
 */
void do_clanlist(struct char_data *ch, char *arg, int cmd)
{
	struct char_data *tmp;
	struct char_file_u player;
	char buf[254], name[MAX_STRING_LENGTH], tmp_name[254], tmp_short[254];
	int x = 0, j = 0, i = 0, clan = 0, length = 35, clength =0;
	extern int top_of_p_table;
	extern struct player_index_element *player_table;
	extern const struct clan clan_list[MAX_CLAN];

	if(!ch)
		return;

	if(!*arg) {
		/* list the clans */
		send_to_char("              $c000c-=* $c0008Clan List $c000c*=-\n\r",ch);
		send_to_char("\n\r",ch);
		x = 1;
		while(clan_list[x].number != -1) {
			sprintf(name,"%s",clan_list[x].name);
			sprintf(name,"%s",CAP(name));
			length = 35; /* this should be enough length for any clan name */
			clength = length - color_strlen(ch, name, 596);
			for(j = 1; j <= clength; j++) {
				strcat(name, " ");
			}
			sprintf(buf,"$c000c[$c000w%2d$c000c] $c000w%s   $c000c[$c000w%s$c000c]\n\r",
				x, name, clan_list[x].shortname);

			send_to_char(buf,ch);
			x++;
		}
		return;
	} else {
		if (!isdigit(*arg)) {
			send_to_char("Usage:  clanlist\n\r",ch);
			send_to_char("        clanlist <clan number>\n\r",ch);
			return;
		} else {
			clan = atoi(arg);
			if(clan < 1 || clan > MAX_CLAN) {
				send_to_char("Unknown clan number.\n\r", ch);
				return;
			} else {
				/* valid clan number */
				ch_printf(ch,"    $c000c-=* $c000w%s $c000wClan info $c000c*=-\n\r",clan_list[clan].name);

				/* loop through pfiles, check for saint[clan] */
//				ch_printf(ch,"\n\r       $c000c- $c000w%s Saints $c000c-\n\r",clan_list[clan].name);
				for(i=0;i<top_of_p_table+1;i++) {
					if (load_char((player_table + i)->name, &player) > -1) {
						/* store to a tmp char that we can deal with */
						CREATE(tmp, struct char_data,1);
						clear_char(tmp);
						store_to_char(&player, tmp);
						if(IS_IMMORTAL(tmp) && GET_CLAN(tmp) == clan) {
							ch_printf(ch, "$c000c[$c0008%s$c000c] $c000w%s %ld\n\r",(GET_SEX(tmp)?((GET_SEX(tmp) != SEX_FEMALE)?"Patron":"Matron"):"Notron",
									CalcPowerLevel(tmp)),
								tmp->player.title?tmp->player.title:GET_NAME(tmp));
						}
						free(tmp);
					} else {
						log("screw up bigtime in load_char, saint part, in clanlist");
						return;
					}
				}
				i=0;
				/* loop through pfiles, check for leader[clan] */
//				ch_printf(ch,"\n\r       $c000c- $c000w%s Leaders $c000c-\n\r",clan_list[clan].name);
				for(i=0;i<top_of_p_table+1;i++) {
					if (load_char((player_table + i)->name, &player) > -1) {
						/* store to a tmp char that we can deal with */
						CREATE(tmp, struct char_data,1);
						clear_char(tmp);
						store_to_char(&player, tmp);
						if(!IS_IMMORTAL(tmp) && IS_SET(tmp->specials.act, PLR_CLAN_LEADER) && GET_CLAN(tmp) == clan) {
							ch_printf(ch, "$c000c[$c0008Leader$c000c] $c000w%s %ld\n\r",
								tmp->player.title?tmp->player.title:GET_NAME(tmp),CalcPowerLevel(tmp));
						}
						free(tmp);
					} else {
						log("screw up bigtime in load_char, leader part, in clanlist");
						return;
					}
				}
				i=0;
				/* now loop through pfiles, check for member[clan] */
//				ch_printf(ch,"\n\r       $c000c- $c000w%s Members $c000c-\n\r",clan_list[clan].name);
				for(i=0;i<top_of_p_table+1;i++) {
					if (load_char((player_table + i)->name, &player) > -1) {
						/* store to a tmp char that we can deal with */
						CREATE(tmp, struct char_data,1);
						clear_char(tmp);
						store_to_char(&player, tmp);
						if(!IS_IMMORTAL(tmp) && !IS_SET(tmp->specials.act, PLR_CLAN_LEADER) && GET_CLAN(tmp) == clan) {
							ch_printf(ch, "$c000c[$c0008Member$c000c] $c000w%s\n\r",
								tmp->player.title?tmp->player.title:GET_NAME(tmp));
						}
						free(tmp);
					} else {
						log("screw up bigtime in load_char, member part, in clanlist");
						return;
					}
				}
			}
		}
	}
}
