/*
  DaleMUD v2.0  Released 2/1994
  See license.doc for distribution terms.   DaleMUD is based on DIKUMUD
*/

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "protos.h"

/* extern variables */
extern struct index_data *obj_index; /*Object maxxes*/
extern struct str_app_type str_app[];
extern struct descriptor_data *descriptor_list;
extern char *drinks[];
extern int drink_aff[][3];
extern struct spell_info_type spell_info[];

/* auction stuff */
	int auct_loop = 0;
		/* if 0, nothing up for sale
		   if 1, something up for auction, no bids received
		   if 1, something up for auction, still no bids received
		   if 3, a bid received
		   if 4, going once
		   if 5, going twice, about to be gone.
		*/
	long intbid = 0;	/* if intbid == 0, there's nothing up for auction */
	long minbid = 0;
	struct char_data *auctioneer; /* who is auctioning junk? */
	struct char_data *bidder; /* who currently has the highest bid? */
//	struct obj_data *auctionobj; /* the item itself */
	extern const struct skillset weaponskills[];
	struct time_info_data age(struct char_data *ch);
	/* Spell Names */
	extern char *spells[];
	/* For Objects */
	extern char *AttackType[];
	extern struct index_data *obj_index;
	extern char *item_types[];
	extern char *extra_bits[];
	extern char *apply_types[];
	extern char *affected_bits[];
	extern char *affected_bits2[];
	extern char *immunity_names[];
	extern char *wear_bits[];



void weight_change_object(struct obj_data *obj, int weight)
{
  struct obj_data *tmp_obj;
  struct char_data *tmp_ch;
  char buf[255];

  if (GET_OBJ_WEIGHT(obj) + weight < 1) {
      weight = 0 - (GET_OBJ_WEIGHT(obj) -1);
      if (obj->carried_by) {
        sprintf(buf,"Bad weight change on %s, carried by %s.",obj->name,obj->carried_by->player.name);
        log(buf);
      } else {
        sprintf(buf,"Bad weight change on %s.",obj->name);
        log(buf);
      }
  }

  if (obj->in_room != NOWHERE) {
    GET_OBJ_WEIGHT(obj) += weight;
  } else if (tmp_ch = obj->carried_by) {
    obj_from_char(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_char(obj, tmp_ch);
  } else if (tmp_obj = obj->in_obj) {
    obj_from_obj(obj);
    GET_OBJ_WEIGHT(obj) += weight;
    obj_to_obj(obj, tmp_obj);
  } else {
    log("Unknown attempt to subtract weight from an object.");
  }
}



void name_from_drinkcon(struct obj_data *obj)
{
  int i;
  char *new_name;

  for(i=0; (*((obj->name)+i)!=' ') && (*((obj->name)+i)!='\0'); i++)  ;

  if (*((obj->name)+i)==' ') {
    new_name=strdup((obj->name)+i+1);
if (obj->name)
    free(obj->name);
    obj->name=new_name;
  }
}



void name_to_drinkcon(struct obj_data *obj,int type)
{
  char *new_name;
  extern char *drinknames[];

  CREATE(new_name,char,strlen(obj->name)+strlen(drinknames[type])+2);
  sprintf(new_name,"%s %s",drinknames[type],obj->name);
if (obj->name)
  free(obj->name);
  obj->name=new_name;
}



void do_drink(struct char_data *ch, char *argument, int cmd)
{
  char buf[255];
  struct obj_data *temp;
  struct affected_type af;
  int amount;

dlog("in do_drink");

  only_argument(argument,buf);

  if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying))) {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if (temp->obj_flags.type_flag!=ITEM_DRINKCON) {
    act("You can't drink from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if((GET_COND(ch,DRUNK)>15)&&(GET_COND(ch,THIRST)>0)) {
    /* The pig is drunk */
    act("You're just sloshed.", FALSE, ch, 0, 0, TO_CHAR);
    act("$n is looks really drunk.", TRUE, ch, 0, 0, TO_ROOM);
    return;
  }

  if((GET_COND(ch,FULL)>20)&&(GET_COND(ch,THIRST)>0)) /* Stomach full */ {
      act("Your stomach can't contain anymore!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if (temp->obj_flags.type_flag==ITEM_DRINKCON){
    if (temp->obj_flags.value[1]>0)  /* Not empty */ {
      sprintf(buf,"$n drinks %s from $p",drinks[temp->obj_flags.value[2]]);
      act(buf, TRUE, ch, temp, 0, TO_ROOM);
      sprintf(buf,"You drink the %s.\n\r",drinks[temp->obj_flags.value[2]]);
      send_to_char(buf,ch);

      if (drink_aff[temp->obj_flags.value[2]][DRUNK] > 0 )
        amount = (25-GET_COND(ch,THIRST))/
          drink_aff[temp->obj_flags.value[2]][DRUNK];
      else
        amount = number(3,10);

      amount = MIN(amount,temp->obj_flags.value[1]);
      /* Subtract amount, if not a never-emptying container */
      if (!IS_SET(temp->obj_flags.value[3],DRINK_PERM) &&
          (temp->obj_flags.value[0] > 20))
        weight_change_object(temp, -amount);

	if (!IS_IMMORTAL(ch)) {

      gain_condition(ch,DRUNK,(int)((int)drink_aff
                 [temp->obj_flags.value[2]][DRUNK]*amount)/4);

      gain_condition(ch,FULL,(int)((int)drink_aff
                           [temp->obj_flags.value[2]][FULL]*amount)/4);

      gain_condition(ch,THIRST,(int)((int)drink_aff
                     [temp->obj_flags.value[2]][THIRST]*amount)/4);

	}

      if(GET_COND(ch,DRUNK)>10)
        act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

      if(GET_COND(ch,THIRST)>20)
        act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

      if(GET_COND(ch,FULL)>20)
        act("You are full.",FALSE,ch,0,0,TO_CHAR);

      /* The shit was poisoned ! */
      if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)) {
          act("Oops, it tasted rather strange ?!!?",FALSE,ch,0,0,TO_CHAR);
          act("$n chokes and utters some strange sounds.",
              TRUE,ch,0,0,TO_ROOM);
          af.type = SPELL_POISON;
          af.duration = amount*3;
          af.modifier = 0;
          af.location = APPLY_NONE;
          af.bitvector = AFF_POISON;
          affect_join(ch,&af, FALSE, FALSE);
        }

      /* empty the container, and no longer poison. */
      if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
        temp->obj_flags.value[1]-= amount;
      if(!temp->obj_flags.value[1]) {  /* The last bit */
        temp->obj_flags.value[2]=0;
        temp->obj_flags.value[3]=0;
        name_from_drinkcon(temp);
      }
      if (temp->obj_flags.value[1] < 1) {  /* its empty */
        if (temp->obj_flags.value[0] < 20) {
          extract_obj(temp);  /* get rid of it */
        }
      }
      return;

    }
    act("It's empty already.",FALSE,ch,0,0,TO_CHAR);

    return;

  }

}



void do_eat(struct char_data *ch, char *argument, int cmd)
{
        char buf[100];
        int j, num;
        struct obj_data *temp;
        struct affected_type af;
dlog("in do_eat");
        one_argument(argument,buf);

        if(!(temp = get_obj_in_list_vis(ch,buf,ch->carrying)))  {
                act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        if((temp->obj_flags.type_flag != ITEM_FOOD) &&
           (GetMaxLevel(ch) < DEMIGOD)) {
            act("Your stomach refuses to eat that!?!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        if(GET_COND(ch,FULL)>20) /* Stomach full */     {
                act("You are to full to eat more!",FALSE,ch,0,0,TO_CHAR);
                return;
        }

        act("$n eats $p",TRUE,ch,temp,0,TO_ROOM);
        act("You eat $p.",FALSE,ch,temp,0,TO_CHAR);

	if(!IS_IMMORTAL(ch)) {
        gain_condition(ch,FULL,temp->obj_flags.value[0]);
	}

        if(GET_COND(ch,FULL)>20)
                act("You are full.",FALSE,ch,0,0,TO_CHAR);

        for(j=0; j<MAX_OBJ_AFFECT; j++)
            if (temp->affected[j].location == APPLY_EAT_SPELL) {
                   num = temp->affected[j].modifier;

/* hit 'em with the spell */

                   ((*spell_info[num].spell_pointer)
                         (6, ch, "", SPELL_TYPE_POTION, ch, 0));
                 }

        if(temp->obj_flags.value[3] && (GetMaxLevel(ch) < LOW_IMMORTAL)) {
           act("That tasted rather strange !!",FALSE,ch,0,0,TO_CHAR);
           act("$n coughs and utters some strange sounds.",
               FALSE,ch,0,0,TO_ROOM);

                af.type = SPELL_POISON;
                af.duration = temp->obj_flags.value[0]*2;
                af.modifier = 0;
                af.location = APPLY_NONE;
                af.bitvector = AFF_POISON;
                affect_join(ch,&af, FALSE, FALSE);
        }
	//obj_index[temp->item_number].number--;
        extract_obj(temp);
}


void do_pour(struct char_data *ch, char *argument, int cmd)
{
  char arg1[132];
  char arg2[132];
  char buf[256];
  struct obj_data *from_obj;
  struct obj_data *to_obj;
  int temp;

dlog("in do_pour");

  argument_interpreter(argument, arg1, arg2);

  if(!*arg1) /* No arguments */ {
    act("What do you want to pour from?",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(!(from_obj = get_obj_in_list_vis(ch,arg1,ch->carrying)))   {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(from_obj->obj_flags.type_flag!=ITEM_DRINKCON)      {
    act("You can't pour from that!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(from_obj->obj_flags.value[1]==0)   {
    act("The $p is empty.",FALSE, ch,from_obj, 0,TO_CHAR);
    return;
  }

  if(!*arg2)    {
    act("Where do you want it? Out or in what?",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(!str_cmp(arg2,"out")) {
    act("$n empties $p", TRUE, ch,from_obj,0,TO_ROOM);
    act("You empty the $p.", FALSE, ch,from_obj,0,TO_CHAR);

    weight_change_object(from_obj, -from_obj->obj_flags.value[1]);

    from_obj->obj_flags.value[1]=0;
    from_obj->obj_flags.value[2]=0;
    from_obj->obj_flags.value[3]=0;
    name_from_drinkcon(from_obj);

    return;

  }

  if(!(to_obj = get_obj_in_list_vis(ch,arg2,ch->carrying)))  {
    act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(to_obj->obj_flags.type_flag!=ITEM_DRINKCON)        {
    act("You can't pour anything into that.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if((to_obj->obj_flags.value[1]!=0)&&
     (to_obj->obj_flags.value[2]!=from_obj->obj_flags.value[2])) {
    act("There is already another liquid in it!",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  if(!(to_obj->obj_flags.value[1]<to_obj->obj_flags.value[0]))  {
    act("There is no room for more.",FALSE,ch,0,0,TO_CHAR);
    return;
  }

  sprintf(buf,"You pour the %s into the %s.",
          drinks[from_obj->obj_flags.value[2]],arg2);
  send_to_char(buf,ch);

  /* New alias */
  if (to_obj->obj_flags.value[1]==0)
    name_to_drinkcon(to_obj,from_obj->obj_flags.value[2]);

  /* First same type liq. */
  to_obj->obj_flags.value[2]=from_obj->obj_flags.value[2];

  /*
    the new, improved way of doing this...
  */
  temp = from_obj->obj_flags.value[1];
  from_obj->obj_flags.value[1] = 0;
  to_obj->obj_flags.value[1] += temp;
  temp = to_obj->obj_flags.value[1] - to_obj->obj_flags.value[0];

  if (temp>0) {
    from_obj->obj_flags.value[1] = temp;
  } else {
    name_from_drinkcon(from_obj);
  }

  if (from_obj->obj_flags.value[1] > from_obj->obj_flags.value[0])
    from_obj->obj_flags.value[1] = from_obj->obj_flags.value[0];

  /* Then the poison boogie */
  to_obj->obj_flags.value[3]=
    (to_obj->obj_flags.value[3]||from_obj->obj_flags.value[3]);

  return;
}

void do_sip(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  struct obj_data *temp;

dlog("in do_sip");

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(temp->obj_flags.type_flag!=ITEM_DRINKCON)    {
      act("You can't sip from that!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(GET_COND(ch,DRUNK)>10) /* The pig is drunk ! */    {
      act("You simply fail to reach your mouth!",FALSE,ch,0,0,TO_CHAR);
      act("$n tries to sip, but fails!",TRUE,ch,0,0,TO_ROOM);
      return;
    }

  if(!temp->obj_flags.value[1])  /* Empty */    {
      act("But there is nothing in it?",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  act("$n sips from the $p",TRUE,ch,temp,0,TO_ROOM);
  sprintf(buf,"It tastes like %s.\n\r",drinks[temp->obj_flags.value[2]]);
  send_to_char(buf,ch);

  gain_condition(ch,DRUNK,(int)(drink_aff[temp->obj_flags.value[2]]
                                [DRUNK]/4));

  gain_condition(ch,FULL,(int)(drink_aff[temp->obj_flags.value[2]][FULL]/4));

  gain_condition(ch,THIRST,(int)(drink_aff[temp->obj_flags.value[2]]
                                 [THIRST]/4)
);

#if 0
  if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM) ||
     (temp->obj_flags.value[0] > 19))
    weight_change_object(temp, -1); /* Subtract one unit, unless permanent */
#endif

  if(GET_COND(ch,DRUNK)>10)
    act("You feel drunk.",FALSE,ch,0,0,TO_CHAR);

  if(GET_COND(ch,THIRST)>20)
    act("You do not feel thirsty.",FALSE,ch,0,0,TO_CHAR);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(IS_SET(temp->obj_flags.value[3],DRINK_POISON)
     && !IS_AFFECTED(ch,AFF_POISON)) /* The shit was poisoned ! */    {
      act("But it also had a strange taste!",FALSE,ch,0,0,TO_CHAR);

      af.type = SPELL_POISON;
      af.duration = 3;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_to_char(ch,&af);
    }

  if(!IS_SET(temp->obj_flags.value[3],DRINK_PERM))
    temp->obj_flags.value[1]--;

  if(!temp->obj_flags.value[1])  /* The last bit */    {
      temp->obj_flags.value[2]=0;
      temp->obj_flags.value[3]=0;
      name_from_drinkcon(temp);
    }

  return;

}


void do_taste(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type af;
  char arg[80];
  struct obj_data *temp;

dlog("in do_taste");

  one_argument(argument,arg);

  if(!(temp = get_obj_in_list_vis(ch,arg,ch->carrying)))    {
      act("You can't find it!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  if(temp->obj_flags.type_flag==ITEM_DRINKCON)    {
      do_sip(ch,argument,0);
      return;
    }

  if(!(temp->obj_flags.type_flag==ITEM_FOOD))    {
      act("Taste that?!? Your stomach refuses!",FALSE,ch,0,0,TO_CHAR);
      return;
    }

  act("$n tastes the $p", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $p", FALSE, ch, temp, 0, TO_CHAR);

  gain_condition(ch,FULL,1);

  if(GET_COND(ch,FULL)>20)
    act("You are full.",FALSE,ch,0,0,TO_CHAR);

  if(temp->obj_flags.value[3]&&!IS_AFFECTED(ch,AFF_POISON))     {
      act("Ooups, it did not taste good at all!",FALSE,ch,0,0,TO_CHAR);

      af.type = SPELL_POISON;
      af.duration = 2;
      af.modifier = 0;
      af.location = APPLY_NONE;
      af.bitvector = AFF_POISON;
      affect_to_char(ch,&af);
    }

  temp->obj_flags.value[0]--;

  if(!temp->obj_flags.value[0]) {       /* Nothing left */
      act("There is nothing left now.",FALSE,ch,0,0,TO_CHAR);
      extract_obj(temp);
    }

  return;

}



/* functions related to wear */

perform_wear(struct char_data *ch, struct obj_data *obj_object, long keyword)
{
  switch(keyword) {
  case 0 :
    act("$n lights $p and holds it.", FALSE, ch, obj_object,0,TO_ROOM);
    break;
  case 1 :
    act("$n wears $p on $s finger.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 2 :
    act("$n wears $p around $s neck.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 3 :
    act("$n wears $p on $s body.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 4 :
    act("$n wears $p on $s head.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 5 :
    act("$n wears $p on $s legs.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 6 :
    act("$n wears $p on $s feet.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 7 :
    act("$n wears $p on $s hands.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 8 :
    act("$n wears $p on $s arms.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 9 :
    act("$n wears $p about $s body.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 10 :
    act("$n wears $p about $s waist.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 11 :
    act("$n wears $p around $s wrist.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 12 :
    act("$n wields $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 13 :
    act("$n grabs $p.",TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 14 :
    act("$n starts using $p as a shield.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 15:
    act("$n wears $p on $s back.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 16:
    act("$n insert $p on $s ear.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  case 17:
    act("$n wears $p on $s eye.", TRUE, ch, obj_object,0,TO_ROOM);
    break;
  }
}


int IsRestricted(int Mask, int Class)
{
  long i;

for (i = CLASS_MAGIC_USER; i<= CLASS_NECROMANCER; i*=2) {
    if (IS_SET(i, Mask) && (!IS_SET(i, Class))) {
      Mask -= i;
    } /* if */
  } /* for */

if (Mask == Class)
    return(TRUE);

  return(FALSE);
}


void wear(struct char_data *ch, struct obj_data *obj_object, long keyword)
{
  char buffer[MAX_STRING_LENGTH];
  int BitMask;
  struct room_data *rp;
  extern const struct race_type race_list[];

  if (!IS_IMMORTAL(ch)) {

    BitMask = GetItemClassRestrictions(obj_object);
	/* only class items */

/*Right here I want to check for Mage/Sorc Only Class and let either wear
  it. --Pentak*/
if (IS_SET(obj_object->obj_flags.extra_flags,ITEM_ONLY_CLASS)) {
   if((OnlyClass(ch,CLASS_MAGIC_USER) || OnlyClass(ch,CLASS_SORCERER))
      && (CLASS_MAGIC_USER + CLASS_SORCERER ==
          GetItemClassRestrictions(obj_object)))
	;
   else {
    if (!OnlyClass(ch,BitMask)) { /* check here only for class restricts */
	send_to_char("You are not the proper person for this.\n\r",ch);
	return;
      }
    }
   }  else	/* not only-class, okay to check normal anti-settings */
    if (IsRestricted(BitMask, ch->player.class) && IS_PC(ch)) {
      send_to_char("You are forbidden to do that.\n\r", ch);
      return;
    }

   }



	if(GET_ITEM_TYPE(obj_object)==ITEM_ARMOR) {


		//0=all

		//obj->obj_flags.value[2]          size of armor
		//race_list[race].size
		if( obj_object->obj_flags.value[2] == 0 ) {


		} else
			if(obj_object->obj_flags.value[2] == race_list[ch->race].size
				|| obj_object->obj_flags.value[2] == (race_list[ch->race].size + 1)
				|| obj_object->obj_flags.value[2] == (race_list[ch->race].size - 1) ) {

			} else {
			send_to_char("It doesnt' seem to fit",ch);
			return;
		}

	//::Tsaron:: '0=General size,1=tiny, 2=small, 3=medium, 4=large, 5=huge, 6=gargantuan '
	//You think 'then slowly introduce the sizes with new zones.'
	//::Tsaron:: 'medium covers everything human, elf, half-orc'
	//::Tsaron:: 'small is dwarf, gnome, and halfling'
	//::Tsaron:: 'ogre and troll would be large'
	//::Tsaron:: 'giant would be huge'

	}

  if (anti_barbarian_stuff(obj_object) && GET_LEVEL(ch,BARBARIAN_LEVEL_IND) !=0
      && GetMaxLevel(ch) < LOW_IMMORTAL) {
      send_to_char("Eck! Not that! You sense magic on it!!! You quickly drop it!\n\r", ch);
      act("$n shivers and drops $p!",FALSE,ch,obj_object,0,TO_ROOM);
      obj_from_char(obj_object);
      obj_to_room(obj_object,ch->in_room);
      return;
    }

  if (IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_MEN)) {
    if (GET_SEX(ch) != SEX_FEMALE) {
      send_to_char("Only women can do that.\n\r", ch);
      return;
    }
  }

  if (IS_SET(obj_object->obj_flags.extra_flags, ITEM_ANTI_WOMEN)) {
    if (GET_SEX(ch) != SEX_MALE) {
      send_to_char("Only men can do that.\n\r", ch);
      return;
    }
  }
 //14 is for shields..   (Bards can't wear shields);  (GH)
  if (keyword == 14 && !IS_IMMORTAL(ch) &&
    OnlyClass(ch,CLASS_BARD)) { /* Lennya, fix to make mobs able to still wear shields */
//	 HasClass(ch,CLASS_BARD)) {
    send_to_char("Bards can't wear shields!!\n\r",ch);
    return;
  }

  if (!IsHumanoid(ch)) {
    if ((keyword != 13) || (!HasHands(ch))) {
      send_to_char("You can't wear things!\n\r",ch);
      return;
    }
  }
  rp = real_roomp(ch->in_room);

  switch(keyword) {
  case 0: {  /* LIGHT SOURCE */
    if (ch->equipment[WEAR_LIGHT]) {
      send_to_char("You are already holding a light source.\n\r", ch);
    } else if (ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight>
          str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
        send_to_char("You cannot wield a two handed weapon, and hold a light source.\n\r", ch);
    } else if ((ch->equipment[WIELD] && ch->equipment[HOLD])) {
        send_to_char("Sorry, you only have two hands.\n\r", ch);
    } else if (rp->sector_type == SECT_UNDERWATER &&
               obj_object->obj_flags.value[2] != -1) {
      send_to_char("You can't light that underwater!\n\r", ch);
    } else {
      if (obj_object->obj_flags.value[2])
        real_roomp(ch->in_room)->light++;
      sprintf(buffer,"You light %s and hold it.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
      perform_wear(ch,obj_object,keyword);
      obj_from_char(obj_object);
      equip_char(ch,obj_object, WEAR_LIGHT);
    }
  } break;

  case 1: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) {
      if ((ch->equipment[WEAR_FINGER_L]) && (ch->equipment[WEAR_FINGER_R])) {
        send_to_char(
            "You are already wearing something on your fingers.\n\r", ch);
      } else {
        perform_wear(ch,obj_object,keyword);
        if (ch->equipment[WEAR_FINGER_L]) {
          send_to_char("Ok. (right finger)\n\r", ch);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_FINGER_R);
        } else {
          send_to_char("Ok. (left finger)\n\r", ch);
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_FINGER_L);
        }
      }
    } else {
      send_to_char("You can't wear that on your finger.\n\r", ch);
    }
  } break;
  case 2: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) {
      if ((ch->equipment[WEAR_NECK_1]) && (ch->equipment[WEAR_NECK_2])) {
        send_to_char("You can't wear any more around your neck.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s around your neck.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        if (ch->equipment[WEAR_NECK_1]) {
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_NECK_2);
        } else {
          obj_from_char(obj_object);
          equip_char(ch, obj_object, WEAR_NECK_1);
        }
      }
    } else {
      send_to_char("You can't wear that around your neck.\n\r", ch);
    }
  } break;
  case 3: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) {
      if (ch->equipment[WEAR_BODY]) {
        send_to_char("You already wear something on your body.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your body.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch,  obj_object, WEAR_BODY);
      }
    } else {
      send_to_char("You can't wear that on your body.\n\r", ch);
    }
  } break;
  case 4: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) {
      if (ch->equipment[WEAR_HEAD]) {
        send_to_char("You already wear something on your head.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your head.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_HEAD);
      }
    } else {
      send_to_char("You can't wear that on your head.\n\r", ch);
    }
  } break;
  case 5: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) {
      if (ch->equipment[WEAR_LEGS]) {
        send_to_char("You already wear something on your legs.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your legs.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_LEGS);
      }
    } else {
      send_to_char("You can't wear that on your legs.\n\r", ch);
    }
  } break;
  case 6: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) {
      if (ch->equipment[WEAR_FEET]) {
        send_to_char("You already wear something on your feet.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your feet.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_FEET);
      }
    } else {
      send_to_char("You can't wear that on your feet.\n\r", ch);
    }
  } break;
  case 7: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) {
      if (ch->equipment[WEAR_HANDS]) {
        send_to_char("You already wear something on your hands.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your hands.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_HANDS);
      }
    } else {
      send_to_char("You can't wear that on your hands.\n\r", ch);
    }
  } break;
  case 8: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) {
      if (ch->equipment[WEAR_ARMS]) {
        send_to_char("You already wear something on your arms.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your arms.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_ARMS);
      }
    } else {
      send_to_char("You can't wear that on your arms.\n\r", ch);
    }
  } break;
  case 9: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) {
      if (ch->equipment[WEAR_ABOUT]) {
        send_to_char("You already wear something about your body.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s about your body.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_ABOUT);
      }
    } else {
      send_to_char("You can't wear that about your body.\n\r", ch);
    }
  } break;
  case 10: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) {
      if (ch->equipment[WEAR_WAISTE]) {
        send_to_char("You already wear something about your waist.\n\r",
                     ch);
      } else {
      sprintf(buffer,"You wear %s around your waist.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch,  obj_object, WEAR_WAISTE);
      }
    } else {
      send_to_char("You can't wear that about your waist.\n\r", ch);
    }
  } break;
  case 11: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) {
      if ((ch->equipment[WEAR_WRIST_L]) && (ch->equipment[WEAR_WRIST_R])) {
        send_to_char(
             "You already wear something around both your wrists.\n\r", ch);
      } else {
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        if (ch->equipment[WEAR_WRIST_L]) {
          send_to_char("Ok. (right wrist)\n\r", ch);
          equip_char(ch,  obj_object, WEAR_WRIST_R);
        } else {
          send_to_char("Ok. (left wrist)\n\r", ch);
          equip_char(ch, obj_object, WEAR_WRIST_L);
        }
      }
    } else {
      send_to_char("You can't wear that around your wrist.\n\r", ch);
    }
  } break;

  case 12:
    if (CAN_WEAR(obj_object,ITEM_WIELD)) {
      if (ch->equipment[WIELD]) {
        send_to_char("You are already wielding something.\n\r", ch);
      } else if (ch->equipment[WEAR_LIGHT] && ch->equipment[HOLD]) {
	send_to_char("You must first remove something from one of your hands.\n\r"
		     , ch);
      } else {

        if (GET_OBJ_WEIGHT(obj_object) >
            str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
	  {
	    send_to_char("It is too heavy for you to use single-handedly.\n\r",ch);
	    if (GET_OBJ_WEIGHT(obj_object) <
		(3*str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)/2)
	      {
		send_to_char("But, you can use it two handed\n\r", ch);
		if (ch->equipment[WEAR_SHIELD])
		  {
		    send_to_char("If you removed your shield\n\r", ch);
		  } /* wearing shield */

		else if (ch->equipment[HOLD] || ch->equipment[WEAR_LIGHT])
		  {
		    send_to_char("If you removed what was in your hands\n\r", ch);
		  } /* holding light type */

		else
		  {
		    perform_wear(ch,obj_object,keyword);
		    obj_from_char(obj_object);
		    equip_char(ch, obj_object, WIELD);
		  } /*  fine, wear it */
	      } /* strong enough */
	    else
	      {
		send_to_char("You are to weak to wield it two handed also.\n\r",ch);
	      } /* to weak to wield two handed */

	  } else {
	    sprintf(buffer,"You wield %s\n\r", obj_object->short_description);
	    send_to_char(buffer, ch);
	    perform_wear(ch,obj_object,keyword);
	    obj_from_char(obj_object);
	    equip_char(ch, obj_object, WIELD);
	  }
      }
    } else {
      send_to_char("You can't wield that.\n\r", ch);
    }
    break;

  case 13:
    if (CAN_WEAR(obj_object,ITEM_HOLD)) {
      if (ch->equipment[HOLD])
      {
        send_to_char("You are already holding something.\n\r", ch);
      } else
       if (ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight
                  > str_app[STRENGTH_APPLY_INDEX(ch)].wield_w)
      {
        send_to_char("You cannot wield a two handed weapon and hold something also.\n\r", ch);
      } else
       if (ch->equipment[WEAR_LIGHT] && ch->equipment[WIELD])
       {
          send_to_char("Sorry, both hands are full!\n\r", ch);
       } else {
        if (CAN_WEAR(obj_object,ITEM_WIELD))
        {
          if (ch->equipment[WEAR_SHIELD]) {
            send_to_char("You can't wear a shield and hold a weapon!\n\r", ch);
            return;
          }

          if (GET_OBJ_WEIGHT(obj_object) >
            str_app[STRENGTH_APPLY_INDEX(ch)].wield_w/2) {
            send_to_char("That weapon is too heavy for you to hold\n\r", ch);
            return;
          }
        } else if (ch->equipment[WIELD]) {
          if (GET_OBJ_WEIGHT(obj_object) >
                   str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
            send_to_char("That item is too heavy for you to hold\n\r", ch);
            return;
          }
        }
      sprintf(buffer,"You grab %s and hold it.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, HOLD);
      }
    } else {
      send_to_char("You can't hold this.\n\r", ch);
    }
    break;


  case 14: {
    if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) {
      if ((ch->equipment[WEAR_SHIELD])) {
        send_to_char("You are already using a shield\n\r", ch);
      } else
      if (ch->equipment[WIELD] && ch->equipment[WIELD]->obj_flags.weight >
              str_app[STRENGTH_APPLY_INDEX(ch)].wield_w) {
            send_to_char("You cannot wield a two handed weapon and wear a shield.\n\r", ch);
      } else
       if (ch->equipment[HOLD] && CAN_WEAR(ch->equipment[HOLD],ITEM_WIELD)){
            send_to_char("You can't wear a shield and hold a weapon!\n\r", ch);
            return;
          }

       else
      {
        perform_wear(ch,obj_object,keyword);
        sprintf(buffer, "You start using %s.\n\r",
                obj_object->short_description);
        send_to_char(buffer, ch);
        obj_from_char(obj_object);
        equip_char(ch, obj_object, WEAR_SHIELD);
      }
    } else {
      send_to_char("You can't use that as a shield.\n\r", ch);
    }
  } break;
  case 15:
    if (CAN_WEAR(obj_object,ITEM_WEAR_BACK) && obj_object->obj_flags.type_flag==ITEM_CONTAINER) {
      if (ch->equipment[WEAR_BACK]) {
        send_to_char("You already wear something on your back.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your back.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch,  obj_object, WEAR_BACK);
      }
    } else {
      send_to_char("You can wear only containers on your back.\n\r", ch);
    }
    break;
  case 16:
    if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) {
      if ((ch->equipment[WEAR_EAR_L]) && (ch->equipment[WEAR_EAR_R])) {
        send_to_char(
             "You already have something in both your ears.\n\r", ch);
      } else {
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        if (ch->equipment[WEAR_EAR_L]) {
          send_to_char("Ok. (right ear)\n\r", ch);
          equip_char(ch,  obj_object, WEAR_EAR_R);
        } else {
          send_to_char("Ok. (left ear)\n\r", ch);
          equip_char(ch, obj_object, WEAR_EAR_L);
        }
      }
    } else {
      send_to_char("You can't insert this in your ear.\n\r", ch);
    }
    break;
  case 17:
    if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) {
      if (ch->equipment[WEAR_EYES]) {
        send_to_char("You already have something on your eyes.\n\r", ch);
      } else {
      sprintf(buffer,"You wear %s on your eyes.\n\r",obj_object->short_description);
      send_to_char(buffer, ch);
        perform_wear(ch,obj_object,keyword);
        obj_from_char(obj_object);
        equip_char(ch,  obj_object, WEAR_EYES);
      }
    } else {
      send_to_char("You can't put that on your eyes.\n\r", ch);
    }
    break;
  case -1: {
    sprintf(buffer,"Wear %s where?.\n\r", obj_object->short_description);
    send_to_char(buffer, ch);
  } break;
  case -2: {
    sprintf(buffer,"You can't wear %s.\n\r", obj_object->short_description);
    send_to_char(buffer, ch);
  } break;
  default: {
    log("Unknown type called in wear.");
  } break;
  }
}


void do_wear(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH+80];
  char arg2[MAX_INPUT_LENGTH+80];
  char buf[256];
  char buffer[MAX_INPUT_LENGTH+80];
  struct obj_data *obj_object, *next_obj;
  int keyword;
  static char *keywords[] = {
    "finger",
    "neck",
    "body",
    "head",
    "legs",
    "feet",
    "hands",
    "arms",
    "about",
    "waist",
    "wrist",
    "wield",
    "hold",
    "shield",
    "back",
    "ears",
    "eye",
    "\n"
    };

dlog("in do_wear");

  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    if (!strcmp(arg1,"all")) {

      for (obj_object = ch->carrying; obj_object; obj_object = next_obj) {
        next_obj = obj_object->next_content;
        keyword = -2;

        if (CAN_WEAR(obj_object,ITEM_HOLD)) keyword = 13;
        if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = 14;
        if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = 1;
        if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = 2;
        if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = 11;
        if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = 10;
        if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = 8;
        if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = 7;
        if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = 6;
        if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = 5;
        if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = 9;
        if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = 4;
        if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = 3;
        if (CAN_WEAR(obj_object,ITEM_WIELD)) keyword = 12;
        if (CAN_WEAR(obj_object,ITEM_WEAR_BACK) &&
            obj_object->obj_flags.type_flag==ITEM_CONTAINER) keyword=15;
        if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) keyword = 17;
        if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) keyword = 16;


        if (keyword != -2) {
          sprintf(buf,"%s :", obj_object->short_description);
          send_to_char(buf,ch);
          wear(ch, obj_object, keyword);
        }
      }

    } else {
      obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
      if (obj_object) {
        if (*arg2) {
          keyword = search_block(arg2, keywords, FALSE); /* Partial Match */
          if (keyword == -1) {
            sprintf(buf, "%s is an unknown body location.\n\r", arg2);
            send_to_char(buf, ch);
          } else {
          sprintf(buf,"%s :", obj_object->short_description);
          send_to_char(buf,ch);
            wear(ch, obj_object, keyword+1);
          }
        } else {
          keyword = -2;
          if (CAN_WEAR(obj_object,ITEM_HOLD)) keyword = 13;
          if (CAN_WEAR(obj_object,ITEM_WEAR_SHIELD)) keyword = 14;
          if (CAN_WEAR(obj_object,ITEM_WEAR_FINGER)) keyword = 1;
          if (CAN_WEAR(obj_object,ITEM_WEAR_NECK)) keyword = 2;
          if (CAN_WEAR(obj_object,ITEM_WEAR_WRIST)) keyword = 11;
          if (CAN_WEAR(obj_object,ITEM_WEAR_WAISTE)) keyword = 10;
          if (CAN_WEAR(obj_object,ITEM_WEAR_ARMS)) keyword = 8;
          if (CAN_WEAR(obj_object,ITEM_WEAR_HANDS)) keyword = 7;
          if (CAN_WEAR(obj_object,ITEM_WEAR_FEET)) keyword = 6;
          if (CAN_WEAR(obj_object,ITEM_WEAR_LEGS)) keyword = 5;
          if (CAN_WEAR(obj_object,ITEM_WEAR_ABOUT)) keyword = 9;
          if (CAN_WEAR(obj_object,ITEM_WEAR_HEAD)) keyword = 4;
          if (CAN_WEAR(obj_object,ITEM_WEAR_BODY)) keyword = 3;
          if (CAN_WEAR(obj_object,ITEM_WEAR_BACK) &&
             obj_object->obj_flags.type_flag==ITEM_CONTAINER) keyword=15;
          if (CAN_WEAR(obj_object,ITEM_WEAR_EYE)) keyword = 17;
          if (CAN_WEAR(obj_object,ITEM_WEAR_EAR)) keyword = 16;

          sprintf(buf,"%s :", obj_object->short_description);
          send_to_char(buf,ch);
          wear(ch, obj_object, keyword);
        }
      } else {
        sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
        send_to_char(buffer,ch);
      }
    }
  } else {
    send_to_char("Wear what?\n\r", ch);
  }
}


void do_wield(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH+80];
  char arg2[MAX_INPUT_LENGTH+80];
  char buffer[MAX_INPUT_LENGTH+80];
  struct obj_data *obj_object;
  int keyword = 12;


dlog("in do_wield");

  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object) {
      wear(ch, obj_object, keyword);
    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
      send_to_char(buffer,ch);
    }
  } else {
    send_to_char("Wield what?\n\r", ch);
  }
}

void do_draw(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_INPUT_LENGTH+80];
  char arg2[MAX_INPUT_LENGTH+80];
  char buffer[MAX_INPUT_LENGTH+80];
  struct obj_data *obj_object, *obj2;
  int keyword = 12;


dlog("in do_draw");

  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    obj_object = get_obj_in_list_vis(ch, arg1, ch->carrying);
    if (obj_object) {
	  if(ch->equipment[WIELD]) {
		if (IS_OBJ_STAT(ch->equipment[WIELD], ITEM_NODROP)) {
			send_to_char("You can't draw your weapon.. Your existing one must be cursed!!.",ch);
			return;
		}

		if ((obj2 = unequip_char(ch,WIELD))!=NULL) {
           obj_to_char(obj2, ch);
           act("You stop using $p and attempt to draw another weapon.",FALSE,ch,obj2,0,TO_CHAR);
           act("$n stops using $p and attempts to draw another weapon.",TRUE,ch,obj2,0,TO_ROOM);
		}
	  }

      wear(ch, obj_object, keyword);


    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
      send_to_char(buffer,ch);
    }
  } else {
    send_to_char("Wield what?\n\r", ch);
  }
}

void do_grab(struct char_data *ch, char *argument, int cmd)
{
  char arg1[128];
  char arg2[128];
  char buffer[256];
  struct obj_data *obj_object;

dlog("in do_grab");

  argument_interpreter(argument, arg1, arg2);

  if (*arg1) {
    obj_object = get_obj_in_list(arg1, ch->carrying);
    if (obj_object) {
      if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
        wear(ch, obj_object, WEAR_LIGHT);
      else
        wear(ch, obj_object, 13);
    } else {
      sprintf(buffer, "You do not seem to have the '%s'.\n\r",arg1);
      send_to_char(buffer,ch);
    }
  } else {
    send_to_char("Hold what?\n\r", ch);
  }
}


void do_remove(struct char_data *ch, char *argument, int cmd)
{
  char arg1[128],*T,*P;
  char buffer[256];
  int Rem_List[20],Num_Equip;
  struct obj_data *obj_object;
  int j;

dlog("in do_remove");

  one_argument(argument, arg1);

  if (*arg1) {
    if (!strcmp(arg1,"all")) {
      for (j=0;j<MAX_WEAR;j++) {
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {
          if (ch->equipment[j]) {
            if ((obj_object = unequip_char(ch,j))!=NULL) {
              obj_to_char(obj_object, ch);

              act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
              if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                if (obj_object->obj_flags.value[2])
                  real_roomp(ch->in_room)->light--;

            }
          }
        } else {
          send_to_char("You can't carry any more stuff.\n\r",ch);
          j = MAX_WEAR;
        }
      }
      act("$n stops using $s equipment.",TRUE,ch,obj_object,0,TO_ROOM);
      return;
    }
    if (isdigit(arg1[0]))           {
      /* PAT-PAT-PAT */

      /* Make a list of item numbers for stuff to remove */

      for (Num_Equip = j=0;j<MAX_WEAR;j++)          {
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))               {
          if (ch->equipment[j]) Rem_List[Num_Equip++] = j;
        }
      }

      T = arg1;

      while (isdigit(*T) && (*T != '\0'))           {
        P = T;
        if (strchr(T,','))              {
          P = strchr(T,',');
          *P = '\0';
        }
        if (atoi(T) > 0 && atoi(T) <= Num_Equip)                {
          if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch))                 {
            j = Rem_List[atoi(T) - 1];
            if (ch->equipment[j])                       {
              if ((obj_object = unequip_char(ch,j))!=NULL)   {
                obj_to_char(obj_object, ch);


                act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
                act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);

                if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
                  if (obj_object->obj_flags.value[2])
                    real_roomp(ch->in_room)->light--;

              }
            }
          }
          else               {
            send_to_char("You can't carry any more stuff.\n\r",ch);
            j = MAX_WEAR;
          }
        } else  {
          sprintf(buffer,"You dont seem to have the %s\n\r",T);
          send_to_char(buffer,ch);
        }
        if (T != P) T = P + 1;
        else *T = '\0';
      }
    }  else {
      obj_object = get_object_in_equip_vis(ch, arg1, ch->equipment, &j);
      if (obj_object) {
         if (IS_OBJ_STAT(obj_object,ITEM_NODROP)) {
              send_to_char
                ("You can't let go of it, it must be CURSED!\n\r", ch);
              return;
         }
        if (CAN_CARRY_N(ch) != IS_CARRYING_N(ch)) {

          obj_to_char(unequip_char(ch, j), ch);

          if (obj_object->obj_flags.type_flag == ITEM_LIGHT)
            if (obj_object->obj_flags.value[2])
              real_roomp(ch->in_room)->light--;

          act("You stop using $p.",FALSE,ch,obj_object,0,TO_CHAR);
          act("$n stops using $p.",TRUE,ch,obj_object,0,TO_ROOM);

        } else {
          send_to_char("You can't carry that many items.\n\r", ch);
        }
      } else {
        send_to_char("You are not using it.\n\r", ch);
      }
    }
  }
  else {
    send_to_char("Remove what?\n\r", ch);
  }

  check_falling(ch);
}

void do_auction(struct char_data *ch, char *argument, int cmd)
{
	char item[50], bid[20], buf[MAX_INPUT_LENGTH];
	extern int Silence;
	struct obj_data *auctionobj;

dlog("in do_auction");

	/* first see if noone else is auctioning stuff */
	if (minbid > 0) {
		if(!bidder) {
			sprintf(buf, "%s is currently auctioning %s, minimum bid set at %d. Wait your turn.\n\r",
					GET_NAME(auctioneer), auctionobj->short_description, minbid);
		} else {
			sprintf(buf, "%s is currently auctioning %s, current bid of %d by %s. Wait your turn.\n\r",
					GET_NAME(auctioneer), auctionobj->short_description, intbid, GET_NAME(bidder));
		}
		send_to_char(buf, ch);
		return;
	}

	if(!*argument) {
		send_to_char("What did you want to auction?\n\r",ch);
		return;
	}

	half_chop(argument,item,bid);

	if (!(auctionobj = get_obj_in_list_vis(ch, item, ch->carrying))) {
		send_to_char("You don't seem to have that item.\n\r",ch);
		return;
	}

	if(!(minbid=atoi(bid)))
		minbid=1; /* min bid is 1 coin, and we got an auction runnin. */
	else if(minbid > 50000000) {
		send_to_char("Sorry, maximum starting bid is 50,000,000 coins.",ch);
		minbid = 0;
		return;
	}


	sprintf(buf,"$c000cAuction:  $c000w%s$c000c auctions $c000w%s$c000c. Minimum bid set at $c000w%d$c000c coins.\n\r"
			,GET_NAME(ch), auctionobj->short_description, minbid);
	send_to_all(buf);

	send_to_char("Your item is taken away from you.\n\r",ch);

	auct_loop = 1;

	auctioneer = ch;

	obj_from_char(auctionobj);
	ch->specials.auction=auctionobj;
	do_save(ch,"",0);

}



void do_bid(struct char_data *ch, char *argument, int cmd)
{
	char item[50], buf[MAX_INPUT_LENGTH], arg[254];
	long bid = 0;
	long newminbid = 0;
	float fnewminbid = 0;
	struct obj_data *auctionobj;

dlog("in do_bid");

	if(IS_NPC(ch))
		return;

	if(!*argument) {  /* show help bid */
		send_to_char("Usage:   bid ?            to see stats on item currently up for auction.\n\r",ch);
		send_to_char("         bid <amount>     to place a bid on the item currently up for auction.\n\r",ch);
		if(IS_IMMORTAL(ch))
			send_to_char("         bid cancel       to cancel an auction.\n\r",ch);
		return;
	}

	if(minbid < 1) {
		send_to_char("But there's nothing up for sale!\n\r",ch);
		return;
	}

	if(!(auctionobj = auctioneer->specials.auction)) {
		log("auctionobj not found in do_bid");
		return;
	}

	only_argument(argument,arg);

	if(isdigit(*arg)) {
		/* can't bid on your own auctions */
		if(auctioneer == ch && !IS_IMMORTAL(ch)) {
			send_to_char("Meh, stop bidding on your own stuff, punk!\n\r",ch);
			return;
		}
		/* can't bid higher than your own bid */
		if(bidder == ch && !IS_IMMORTAL(ch)) {
			send_to_char("You already have the highest bid.\n\r",ch);
			return;
		}
		if(!(bid = atoi(arg))) {
			send_to_char("That is not a valid number.\n\r",ch);
			return;
		}
		/* Is the bid 5% higher than the current? */
		fnewminbid = 1.05 * intbid;
		newminbid = (int) fnewminbid;
		if(newminbid == intbid)
			newminbid++;
		if(bid < newminbid) {
			sprintf(buf,"Sorry, your bid has to be at least 5%s higher (min. %d).\n\r","%",newminbid);
			send_to_char(buf,ch);
			return;
		}
		if(bid < minbid) {
			sprintf(buf,"Sorry, your bid has to be at least the minimum bid (%d).\n\r","%",minbid);
			send_to_char(buf,ch);
			return;
		}
		/* does player have the coin on hand? */
		if(GET_GOLD(ch)<bid && !IS_IMMORTAL(ch)) {
			send_to_char("You don't have that many coins in your pocket.\n\r",ch);
			return;
		}
		/* bid seems to be okay */

		/* reset previous bidder, if any */
		if(bidder) {
			if(bidder->specials.minbid > 0)
				GET_GOLD(bidder) += bidder->specials.minbid;
			else
				log("previous bidder had a bid of less than 1 coin?!");
			bidder->specials.minbid = 0;
			do_save(bidder, "", 0);
		}

		/* take coin as a precaution and put it in specials.minbid, make him bidder */
		GET_GOLD(ch) -= bid;
		ch->specials.minbid = bid;
		intbid = bid;
		bidder = ch;
		do_save(ch, "", 0);

		sprintf(buf,"$c000cAuction:  $c000w%s$c000c places a bid of $c000w%d$c000c coins for $c000w%s$c000c.\n\r"
				,GET_NAME(ch), intbid, auctionobj->short_description);
		send_to_all(buf);
		/* reset auction loop to 4 */
		auct_loop = 4;
		return;

	} else {
		if(!strcmp(arg,"?")) {
			/* show player the item stats */
			auction_id(50, ch, 0, auctionobj);
			return;
		}
		if(IS_IMMORTAL(ch) && !strcmp(arg,"cancel")) {
			/* cancel this auction */
			send_to_all("$c000cAuction: the auction has been cancelled.\n\r");

			/* reset previous bidder, if any */
			if(bidder) {
				if(bidder->specials.minbid > 0)
					GET_GOLD(bidder) += bidder->specials.minbid;
				else
					log("previous bidder had a bid of less than 1 coin?!");
				bidder->specials.minbid = 0;
				send_to_char("You are returned your deposit for this auction.\n\r",bidder);
				do_save(bidder, "", 0);
			}
			intbid = 0;
			minbid = 0;

			auctioneer->specials.auction = 0;
				assert(!auctionobj->in_obj);
				assert(auctionobj->in_room == NOWHERE);
				assert(!auctionobj->carried_by);

			auctionobj->equipped_by = 0;
			auctionobj->eq_pos = -1;

			obj_to_char(auctionobj, auctioneer);
			send_to_char("Your item is returned to you.\n\r",auctioneer);
			do_save(auctioneer, "", 0);
			auct_loop = 0;
			return;
		}
		send_to_char("That is not a valid argument. Type bid without arguments for help.\n\r",ch);
	}
}

void auction_id(byte level, struct char_data *ch, struct char_data *victim, struct obj_data *obj)
{
	char buf[256], buf2[256];
	int i;
	bool found;
	extern const struct skillset weaponskills[];
	struct time_info_data age(struct char_data *ch);

	extern char *spells[];
	extern char *RaceName[];

	extern char *AttackType[];
	extern struct index_data *obj_index;
	extern char *item_types[];
	extern char *extra_bits[];
	extern char *apply_types[];
	extern char *affected_bits[];
	extern char *affected_bits2[];
	extern char *immunity_names[];
	extern char *wear_bits[];

	assert(ch && obj);

	if (obj) {
	send_to_char("$c000cThe item currently on auction has the following stats:\n\r\n\r", ch);

	sprintf(buf, "$c000cObject '$c000w%s$c000c', Item type: $c000w", obj->name);
	sprinttype(GET_ITEM_TYPE(obj),item_types,buf2);
	strcat(buf,buf2);

	if(IS_WEAPON(obj)) {
		if(IS_IMMORTAL(ch))
			sprintf(buf2,"$c000c, Weapon Speed: $c000w%s$c000c($c000w%.2f$c000c)",SpeedDesc(obj->speed),(float)obj->speed/100);
		else
			sprintf(buf2,"$c000c, Weapon Speed: $c000w%s$c000c",SpeedDesc(obj->speed));

		strcat(buf,buf2);
	}
	strcat(buf,"\n\r");
	send_to_char(buf, ch);

	if (GetMaxLevel(ch)>LOW_IMMORTAL) {
		sprintf(buf, "$c000cR-number: [$c000w%d$c000c], V-number: [$c000w%d$c000c]",
			obj->item_number, (obj->item_number >= 0) ? obj_index[obj->item_number].virtual : 0);
		if (obj->max==0)
			sprintf(buf2,"$c000w%s","unlimited");
		else
			sprintf(buf2,"$c000w%d$c000c", obj->max,obj->level);
		sprintf(buf,"%s $c000cObjMax: [$c000w%s$c000c], Tweak Rate: [$c000w%d$c000c]\n\r", buf, buf2, obj->tweak);
		send_to_char(buf, ch);

		if(obj->level==0)
			sprintf(buf2,"$c000cEgo: $c000wNone$c000c, ");
		else
			sprintf(buf2,"$c000cEgo: $c000wLevel %d$c000c, ",obj->level);
		send_to_char(buf2,ch);

		sprintf(buf2,"$c000cLast modified by $c000w%s $c000con $c000w%s",obj->modBy,asctime(localtime(&obj->modified)));
		send_to_char(buf2,ch);
	}

	if (obj->obj_flags.bitvector) {
		send_to_char("$c000cItem will give you following abilities:$c000w  ", ch);
		sprintbit((unsigned)obj->obj_flags.bitvector,affected_bits,buf);
		strcat(buf,"\n\r");
		send_to_char(buf, ch);
	}

	send_to_char("$c000cItem is:$c000w ", ch);
	sprintbit( (unsigned)obj->obj_flags.extra_flags,extra_bits,buf);
	sprintf(buf2,"$c000w");
	strcat(buf2,buf);
	strcat(buf2,"\n\r");
	send_to_char(buf2,ch);

	sprintf(buf,"$c000cWeight:$c000w %d$c000c, Value: $c000w%d,$c000c Rent cost: $c000w %d  %s\n\r",
			obj->obj_flags.weight, obj->obj_flags.cost,
			obj->obj_flags.cost_per_day
			, IS_RARE(obj)? "$c000c[$c00wRARE$c000c]":" ");

	send_to_char(buf, ch);

	send_to_char("$c000cCan be worn on :$c000w", ch);
	sprintbit((unsigned)obj->obj_flags.wear_flags,wear_bits,buf);
	sprintf(buf2,"$c000w");
	strcat(buf2,buf);
	strcat(buf2,"\n\r");
	send_to_char(buf2, ch);

	switch (GET_ITEM_TYPE(obj)) {

		case ITEM_SCROLL :
		case ITEM_POTION :
			sprintf(buf, "$c000cLevel$c000w %d $c000cspells of:$c000w\n\r",obj->obj_flags.value[0]);
			send_to_char(buf, ch);
			if (obj->obj_flags.value[1] >= 1) {
				sprinttype(obj->obj_flags.value[1]-1,spells,buf);
				sprintf(buf2,"$c000w%s",buf);
				strcat(buf2,"\n\r");
				send_to_char(buf2, ch);
			}
			if (obj->obj_flags.value[2] >= 1) {
				sprinttype(obj->obj_flags.value[2]-1,spells,buf);
				sprintf(buf2,"$c000w%s",buf);
				strcat(buf2,"\n\r");
				send_to_char(buf2, ch);
			}
			if (obj->obj_flags.value[3] >= 1) {
				sprinttype(obj->obj_flags.value[3]-1,spells,buf);
				sprintf(buf2,"$c000w%s",buf);
				strcat(buf2,"\n\r");
				send_to_char(buf2, ch);
			}
			break;
		case ITEM_WAND :
		case ITEM_STAFF :
			sprintf(buf, "$c000cCosts $c000w%d $c000cmana to use, with$c000w %d$c000c charges left.\n\r",
					obj->obj_flags.value[1], obj->obj_flags.value[2]);
			send_to_char(buf, ch);
			sprintf(buf, "$c000cLevel $c000w%d $c000cspell of:$c000w\n\r", obj->obj_flags.value[0]);
			send_to_char(buf, ch);
			if (obj->obj_flags.value[3] >= 1) {
				sprinttype(obj->obj_flags.value[3]-1,spells,buf);
				sprintf(buf2,"$c000w%s",buf);
				strcat(buf2,"\n\r");
				send_to_char(buf2, ch);
			}
			break;
		case ITEM_WEAPON :
			sprintf(buf, "$c000cDamage Dice is '$c000w%dD%d$c000c' [$c000w%s$c000c] [$c000w%s$c000c]\n\r",
					obj->obj_flags.value[1], obj->obj_flags.value[2], AttackType[obj->obj_flags.value[3]/*-1*/],
					weaponskills[obj->weapontype].name);
			send_to_char(buf, ch);
			break;
		case ITEM_ARMOR :
			sprintf(buf, "$c000cAC-apply is: $c000w%d,   ", obj->obj_flags.value[0]);
			send_to_char(buf, ch);
			sprintf(buf, "$c000cSize of armor is: $c000w%s\n\r", ArmorSize(obj->obj_flags.value[2]));
			send_to_char(buf, ch);
			break;
	}

	found = FALSE;

	for (i=0;i<MAX_OBJ_AFFECT;i++) {
		if ((obj->affected[i].location != APPLY_NONE) && (obj->affected[i].modifier != 0)) {
			if (!found) {
				send_to_char("$c000cCan affect you as :$c000w\n\r", ch);
				found = TRUE;
			}
			sprinttype(obj->affected[i].location,apply_types,buf2);
			sprintf(buf," $c000c   Affects :$c000w %s$c000c by $c000w", buf2);
			send_to_char(buf,ch);
			switch(obj->affected[i].location) {
				case APPLY_M_IMMUNE:
				case APPLY_IMMUNE:
				case APPLY_SUSC:
					sprintbit(obj->affected[i].modifier,immunity_names,buf2);
					sprintf(buf,"$c000w");
					strcat(buf,buf2);
					sprintf(buf2,buf);
					strcat(buf2,"\n\r");
					break;
				case APPLY_ATTACKS:
					sprintf(buf2,"$c000w%f\n\r", obj->affected[i].modifier/10);
					break;
				case APPLY_WEAPON_SPELL:
				case APPLY_EAT_SPELL:
					sprintf(buf2,"$c000w%s\n\r", spells[obj->affected[i].modifier-1]);
					break;
				case APPLY_SPELL:
					sprintbit(obj->affected[i].modifier,affected_bits, buf2);
					sprintf(buf,"$c000w");
					strcat(buf,buf2);
					strcat(buf,"\n\r");
					sprintf(buf2,buf);
					break;
				case APPLY_SPELL2:
					sprintbit(obj->affected[i].modifier,affected_bits2, buf2);
					sprintf(buf,"$c000w");
					strcat(buf,buf2);
					strcat(buf,"\n\r");
					sprintf(buf2,buf);
					break;
				case APPLY_RACE_SLAYER:
					sprintf(buf2,"$c000w%s\n\r", RaceName[obj->affected[i].modifier]);
					break;
				case APPLY_ALIGN_SLAYER:
					if (obj->affected[i].modifier > 1 )
						sprintf(buf2,"$c000wSLAY GOOD\n\r");
					else if (obj->affected[i].modifier == 1 )
						sprintf(buf2,"$c000wSLAY NEUTRAL\n\r");
					else /* less than 1 == slay evil */
						sprintf(buf2,"$c000wSLAY EVIL\n\r");
					break;
				default:
					sprintf(buf2,"$c000w%d\n\r", obj->affected[i].modifier);
					break;
			}
			send_to_char(buf2,ch);
		}
	}

	}
}

