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

      gain_condition(ch,DRUNK,(int)((int)drink_aff
                 [temp->obj_flags.value[2]][DRUNK]*amount)/4);

      gain_condition(ch,FULL,(int)((int)drink_aff
                           [temp->obj_flags.value[2]][FULL]*amount)/4);

      gain_condition(ch,THIRST,(int)((int)drink_aff
                     [temp->obj_flags.value[2]][THIRST]*amount)/4);

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
        act("You eat the $o.",FALSE,ch,temp,0,TO_CHAR);

        gain_condition(ch,FULL,temp->obj_flags.value[0]);

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

  act("$n sips from the $o",TRUE,ch,temp,0,TO_ROOM);
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

  act("$n tastes the $o", FALSE, ch, temp, 0, TO_ROOM);
  act("You taste the $o", FALSE, ch, temp, 0, TO_CHAR);

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

for (i = CLASS_MAGIC_USER; i<= CLASS_BARD; i*=2) {
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
      HasClass(ch,CLASS_BARD)){
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("You already wear something about your waiste.\n\r",
                     ch);
      } else {
        send_to_char("OK.\n\r", ch);
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
	    sprintf(buffer," Ok\n\r");//,obj_object->short_description);
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
        sprintf(buffer," Ok.\n\r");//,obj_object->short_description);
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
        send_to_char("OK.\n\r", ch);
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
        send_to_char("OK.\n\r", ch);
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
