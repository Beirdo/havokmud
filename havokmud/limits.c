
#include <stdio.h>
#include <assert.h>

#include "protos.h"

/* struct room_data *real_roomp(int); */

extern struct char_data *character_list;
extern struct obj_data *object_list;
extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct room_data *world;
extern const char *RaceName[];
extern int RacialMax[][MAX_CLASS];

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];



char *ClassTitles(struct char_data *ch)
{
  unsigned char i, count=0;
  static char buf[256];

    for (i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
      if (GET_LEVEL(ch, i)) {
	count++;
	if (count > 1) {
	  sprintf(buf + strlen(buf),
	   "/%s",GET_CLASS_TITLE(ch, i,GET_LEVEL(ch,i)));
	} else {
	  sprintf(buf, "%s", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)));
	}
      }
    }
  return(buf);
}


/* When age < 15 return the value p0 */
/* When age in 15..29 calculate the line between p1 & p2 */
/* When age in 30..44 calculate the line between p2 & p3 */
/* When age in 45..59 calculate the line between p3 & p4 */
/* When age in 60..79 calculate the line between p4 & p5 */
/* When age >= 80 return the value p6 */
int graf(int age, int race, int p0, int p1, int p2, int p3, int p4, int
p5, int p6)
{
  extern const struct race_type race_list[];

  if (age < race_list[race].start)
    return(p0);
  else if (age <= race_list[race].mature)
    return (int) (p1+(((age-race_list[race].start)*(p2-p1))/(race_list[race].mature-race_list[race].start)));
  else if (age <= race_list[race].middle)
    return (int) (p2+(((age-race_list[race].mature)*(p3-p2))/(race_list[race].middle-race_list[race].mature)));
  else if (age <= race_list[race].old)
    return (int) (p3+(((age-race_list[race].middle)*(p4-p3))/(race_list[race].old-race_list[race].middle)));
  else if (age <= race_list[race].ancient)
    return (int) (p4+(((age-race_list[race].old)*(p5-p4))/(race_list[race].ancient-race_list[race].old)));
  else if (age <= race_list[race].venerable)
    return (int) (p5+(((age-race_list[race].ancient)*(p6-p5))/(race_list[race].venerable-race_list[race].ancient)));
  else
    return (int) (p6+(((age-race_list[race].ancient)*(p6-p6-p5))/(race_list[race].venerable-race_list[race].ancient)));

}


/* The three MAX functions define a characters Effective maximum */
/* Which is NOT the same as the ch->points.max_xxxx !!!          */
int mana_limit(struct char_data *ch)
{
  int max, tmp;

  max = 0;

  if (IS_NPC(ch)) return(100);

  if (HasClass(ch, CLASS_MAGIC_USER)) {
    max += 100;
    max += GET_LEVEL(ch, MAGE_LEVEL_IND) * 5;
  }

			/* actually this is worthless as Sorcerer's do not */
			/* use mana at all.... */
  if (HasClass(ch, CLASS_SORCERER)) {
    max += 100;
    max += GET_LEVEL(ch, SORCERER_LEVEL_IND) * 5;
  }

  if (HasClass(ch, CLASS_PSI)) {
    max += 100;
    max += GET_LEVEL(ch, PSI_LEVEL_IND) * 5;
  }


  if (HasClass(ch, CLASS_PALADIN)) {
    max += 100;
    max += (GET_LEVEL(ch, PALADIN_LEVEL_IND)/4) * 5;
  }

  if (HasClass(ch, CLASS_RANGER)) {
    max += 100;
    max += (GET_LEVEL(ch, RANGER_LEVEL_IND)/4) * 5;
  }

  if (HasClass(ch, CLASS_CLERIC)) {
    max += 100;
    max += (GET_LEVEL(ch, CLERIC_LEVEL_IND)/3) * 5;
  }

  if (HasClass(ch, CLASS_DRUID)) {
    max += 100;
    max += (GET_LEVEL(ch, DRUID_LEVEL_IND)/3) * 5;
  }

  if (HasClass(ch, CLASS_THIEF)) {
    max += 100;
  }

  if (HasClass(ch, CLASS_WARRIOR)) {
    max += 100;
  }

  if (HasClass(ch, CLASS_MONK)) {
    max += 100;
  }
  if (HasClass(ch,CLASS_BARD)) {
    max +=100;
  }
  max /= HowManyClasses(ch);

/*
  new classes should be inserted here.
*/

  tmp = 0;


   tmp = GET_INT(ch)/3;
   tmp += 2;
   tmp = tmp*3;

  max += tmp;
  max += ch->points.max_mana;   /* bonus mana */

 /*
  * Add class mana maximums here...
 */

 if (OnlyClass(ch,CLASS_BARBARIAN)) /* 100 mana max for barbs */
 {
    max=100;  /* barbarians only get 100 mana... */
 }

  return(max);
}


int hit_limit(struct char_data *ch)
{
  int max;
  int race;

  if (IS_PC(ch)) {
      struct time_info_data ma;
      age2(ch, &ma);
      race = GET_RACE(ch);
      max = (ch->points.max_hit) +
	(graf(ma.year, race, 2,4,17,14,8,0,-10));
  } else
    max = (ch->points.max_hit);


  /* Class/Level calculations */

  /* Skill/Spell calculations */

  return (max);
}


int move_limit(struct char_data *ch)
{
  int max;

  if (!IS_NPC(ch)) {
    max = 100 + GET_CON(ch);
  } else {
    max = ch->points.max_move;
  }

  if (IsRideable(ch))
    max *= 2;


  if (GET_RACE(ch) == RACE_DWARF || GET_RACE(ch) == RACE_ROCK_GNOME ||
     GET_RACE(ch) == RACE_DEEP_GNOME || GET_RACE(ch) == RACE_FOREST_GNOME)
    max -= 35;
  else if (GET_RACE(ch) == RACE_MOON_ELF || GET_RACE(ch) == RACE_DROW  ||
  	   GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
  	   GET_RACE(ch) == RACE_SEA_ELF ||
           GET_RACE(ch)== RACE_HALF_ELF)
    max += 20;
  else if (GET_RACE(ch) == RACE_AVARIEL)
    max += 35;
  else if (GET_RACE(ch) == RACE_HALFLING)
    max -= 45 ;
   else if (GET_RACE(ch) == RACE_HALF_GIANT)
    max +=60;
  else if (GET_RACE(ch) == RACE_HALF_OGRE)
	max +=50        ;

  if (GET_CLASS(ch) == CLASS_BARBARIAN)
    max +=45;  /* barbs get more move ... */


  max += ch->points.max_move;  /* move bonus */

  return (max);
}


/* manapoint gain pr. game hour */
int mana_gain(struct char_data *ch)
{
  int gain;
  int race;

  if((IS_NPC(ch)) && (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
    /* Neat and fast */
    gain = 8;
  } else {
    struct time_info_data ma;
    age2(ch, &ma);
    race = GET_RACE(ch);
#ifdef NEWGAIN
    gain = graf(ma.year, race, 3,9,12,16,20,16,2);
#else
    gain = graf(ma.year, race, 2,4,6,8,10,16,2);
#endif
  }

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += gain;
      break;
    case POSITION_RESTING:
      gain+= (gain>>1);  /* Divide by 2 */
      break;
    case POSITION_SITTING:
      gain += (gain>>2); /* Divide by 4 */
      break;
    }

    gain += gain;


  gain += wis_app[GET_WIS(ch)].bonus*2;


  gain += ch->points.mana_gain;

  if (IS_AFFECTED(ch,AFF_POISON))
    gain >>= 2;

  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 2;

  if (GET_RACE(ch) == RACE_MOON_ELF || GET_RACE(ch) == RACE_ROCK_GNOME ||
  	   GET_RACE(ch) == RACE_GOLD_ELF || GET_RACE(ch) == RACE_WILD_ELF ||
  	   GET_RACE(ch) == RACE_SEA_ELF || GET_RACE(ch) ==
           RACE_FOREST_GNOME || GET_RACE(ch) == RACE_DROW || GET_RACE(ch)
           == RACE_HALF_ELF || GET_RACE(ch) == RACE_DEEP_GNOME ||
           GET_RACE(ch) == RACE_AVARIEL)
    gain+=2;

  if (GET_COND(ch, DRUNK)>10)
    gain += (gain >> 1);
  else if (GET_COND(ch, DRUNK)>0)
    gain += (gain >> 2);

    /* Class calculations */

/* magic type people get quicker mana re-gen, fighter/paladin/rangers get it slower */
 if (!HasClass(ch,CLASS_MAGIC_USER|CLASS_SORCERER|CLASS_CLERIC|CLASS_DRUID|CLASS_PSI)) {
    gain -=2;
  }
	/* these guys get mana even slower */
if (HasClass(ch, CLASS_BARBARIAN))
	 gain -=2;

    /* Skill/Spell calculations */

if (affected_by_spell(ch,SKILL_MEDITATE))
  	gain +=3;


  return (gain);
}


int hit_gain(struct char_data *ch)
     /* Hitpoint gain pr. game hour */
{

  int gain, dam, i, race;


  if(IS_NPC(ch)) {
    gain = 8;
    /* Neat and fast */
  } else {

    if (GET_POS(ch) == POSITION_FIGHTING) {
      gain = 0;
    } else {
      struct time_info_data ma;
      age2(ch, &ma);
      race = GET_RACE(ch);
#ifdef NEWGAIN

       gain = graf(ma.year, race, 3,9,12,16,12,6,1);
#else
      gain = graf(ma.year, race, 2,4,6,8,6,3,1);
#endif
    }
  }

    /* Position calculations    */

  switch (GET_POS(ch)) {
  case POSITION_SLEEPING:
    gain += gain;
    break;
  case POSITION_RESTING:
    gain+= gain>>1;
    break;
  case POSITION_SITTING:
    gain += gain>>2;
    break;
  }


  if (GET_RACE(ch) == RACE_DWARF)
    gain += 2;

  if (GET_RACE(ch) == RACE_HALFLING)
    gain += 1;


if (GET_RACE(ch) == RACE_HALF_GIANT) /* faster hps regen */
   gain+=3;
if (GET_RACE(ch) == RACE_HALF_OGRE)
   gain +=2;
if (GET_RACE(ch) == RACE_HALF_ORC)
	gain +=1;

   if (GET_CLASS(ch) == CLASS_BARBARIAN)
       gain += 4;    /* barbs gain hits faster... */

  gain += con_app[GET_CON(ch)].hitp/2;

  if (IS_AFFECTED(ch,AFF_POISON))  {
    gain = 0;
    dam = number(10,32);
    if (GET_RACE(ch) == RACE_HALFLING)
      dam = number(1,20);
    if (affected_by_spell(ch, SPELL_SLOW_POISON))
      dam /= 4;
    damage(ch, ch, dam, SPELL_POISON);
  }

  if (IS_AFFECTED2(ch, AFF2_HEAT_STUFF)) {
    dam = 0;
    /*
      count items in eq
      */
    for (i=0;i<=HOLD;i++) {
      if (ch->equipment[i]) {
	dam += 2;
      }
    }

    damage(ch, ch, dam, SPELL_HEAT_STUFF);
  }

  gain += ch->points.hit_gain;

  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 4;

  if (GET_COND(ch, DRUNK)>10)
    gain += (gain >> 1);
  else if (GET_COND(ch, DRUNK)>0)
    gain += (gain >> 2);

    /* Class/Level calculations */
	/* non-warrior types get slower hps re-gen */
if (!HasClass(ch,CLASS_WARRIOR|CLASS_PALADIN|CLASS_RANGER|CLASS_BARBARIAN)) {
    gain -=2;
   if (gain < 0 && !ch->specials.fighting)    /* give them a small break */
       damage(ch,ch,gain*-1,TYPE_SUFFERING);
  }

    /* Skill/Spell calculations */

  return (gain);
}



int move_gain(struct char_data *ch)
     /* move gain pr. game hour */
{
  int gain, race;

  if(IS_NPC(ch)) {
    gain = 22;
    if (IsRideable(ch))
      gain += gain/2;

    /* Neat and fast */
  } else {
      struct time_info_data ma;
      age2(ch, &ma);
      race = GET_RACE(ch);
    if (GET_POS(ch) != POSITION_FIGHTING)
#ifdef NEWGAIN
      gain = graf(ma.year, race, 15,21,25,28,20,10,3);
#else
      gain = graf(ma.year, race, 10,15,20,22,15,7,1);
#endif

    else
      gain = 0;
  }

    /* Position calculations    */
    switch (GET_POS(ch)) {
    case POSITION_SLEEPING:
      gain += (gain>>2); /* Divide by 4 */
      break;
    case POSITION_RESTING:
      gain+= (gain>>3);  /* Divide by 8 */
      break;
    case POSITION_SITTING:
      gain += (gain>>4); /* Divide by 16 */
      break;
    }


  if (GET_RACE(ch) == RACE_DWARF)
    gain += 4;

if (GET_RACE(ch) == RACE_HALF_GIANT)
	gain +=6;
if (GET_RACE(ch) == RACE_HALF_OGRE)
	gain +=5;
if (GET_RACE(ch) == RACE_HALF_ORC)
	gain +=4;

  gain += ch->points.move_gain;

  if (IS_AFFECTED(ch,AFF_POISON))
    gain >>= 5;

  if((GET_COND(ch,FULL)==0)||(GET_COND(ch,THIRST)==0))
    gain >>= 3;

/* Class specific stuff */

	/* non-thief/monks types regen move slower */
if (!HasClass(ch,CLASS_THIEF|CLASS_MONK))
  gain-=2;

  gain += 15;  //They were regenning too slowly... added that for now -MW
  return (gain);
}



/* Gain maximum in various points */
void advance_level(struct char_data *ch, int class)
{
  int add_hp, i;
  char bufx[50];
  int add_mana = 0, add_move=0;
  if (class > MAX_CLASS) {
    log("Bad advance class.. no such class");
    return;
  }
  	add_move=GET_MAX_MOVE(ch);
	add_mana=GET_MAX_MANA(ch);
  if (GET_LEVEL(ch, class) > 0 &&
      GET_EXP(ch) < titles[class][GET_LEVEL(ch, class)+1].exp) {
    /*  they can't advance here */
    log("Bad advance_level, can't advance in this class.");
    return;
  }

  GET_LEVEL(ch, class) += 1;



  if (class == WARRIOR_LEVEL_IND || class == BARBARIAN_LEVEL_IND ||
      class == PALADIN_LEVEL_IND || class == RANGER_LEVEL_IND)
     add_hp = con_app[GET_RCON(ch)].hitp;
  else
     add_hp = MIN(con_app[GET_RCON(ch)].hitp,2);


  switch(class) {

  case MAGE_LEVEL_IND : {
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
	add_hp += number(2,6);
    else
      add_hp += 1;
  } break;

  case SORCERER_LEVEL_IND : {
    if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 12)
	add_hp += number(2,6);
    else
      add_hp += 1;
  } break;

  case CLERIC_LEVEL_IND : {
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12)
	add_hp += number(3,15);
    else
      add_hp += 3;
  } break;

  case THIEF_LEVEL_IND : {
    if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 12)
	add_hp +=number(2,10);
    else
      add_hp += 2;
  } break;

  case PSI_LEVEL_IND : {
    if (GET_LEVEL(ch, PSI_LEVEL_IND) < 12)
	add_hp +=number(2,10);
    else
      add_hp += 2;
  } break;

  case WARRIOR_LEVEL_IND : {
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 10)
	add_hp += number(3,16);
    else
      add_hp += 4;

  } break;

  case RANGER_LEVEL_IND : {
    if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 11)
	add_hp += number(3,13);
    else
      add_hp += 4;

  } break;

  case PALADIN_LEVEL_IND : {
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 10)
	add_hp += number(3,16);
    else
      add_hp += 4;

  } break;

  case BARBARIAN_LEVEL_IND : {
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) < 15)
	add_hp += number(3,22);
    else
      add_hp += 6;
  } break;


  case DRUID_LEVEL_IND:
    if (GET_LEVEL(ch,  DRUID_LEVEL_IND) < 15)
	add_hp += number(2,14);
    else
      add_hp += 3;
    break;

  case MONK_LEVEL_IND:
    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 17)
	add_hp += number (2,10);

    else
      add_hp += 2;
    break;
  case BARD_LEVEL_IND:
    if(GET_LEVEL(ch,BARD_LEVEL_IND) < 15)
      add_hp += number (2,10);
    else
      add_hp += 2;
    break;
  }

  add_hp /= HowManyClasses(ch);



	if(MAX(1, add_hp) > 0) {
		sprintf(bufx,"You feel healthier. +%d Health\n\r",add_hp);
		send_to_char(bufx,ch);
	}
  ch->points.max_hit += MAX(1, add_hp);

  if (ch->specials.spells_to_learn < 70)
    ch->specials.spells_to_learn += MAX(1, MAX(2, wis_app[GET_RWIS(ch)].bonus)/HowManyClasses(ch));
  else {
    send_to_char("Practices: Use them or lose them.\n\r", ch);
  }

  ClassSpecificStuff(ch);

  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    for (i = 0; i < 3; i++)
      ch->specials.conditions[i] = -1;


	if(add_mana!=GET_MAX_MANA(ch)) {
		sprintf(bufx,"You feel more magical.  +%d Mana\n\r", GET_MAX_MANA(ch)-add_mana);
		send_to_char(bufx,ch);
	}

	if(add_move!=GET_MAX_MOVE(ch)) {
		sprintf(bufx,"You feel your endurance increasing.  +%d Movement\n\r", GET_MAX_MOVE(ch)-add_move);
		send_to_char(bufx,ch);
	}
}



/* Lose in various points */

/*
** Damn tricky for multi-class...
** Fixed by msw ....
*/

void drop_level(struct char_data *ch, int class, int goddrain)
{
  int add_hp, lin_class;

  extern struct wis_app_type wis_app[];
  extern struct con_app_type con_app[];

 if (!goddrain) {
  if (GetMaxLevel(ch) >= LOW_IMMORTAL)
    return;
 }

  if (GetMaxLevel(ch) == 1)
    return;

  add_hp = con_app[GET_RCON(ch)].hitp;

  switch(class)
  {

  case CLASS_MAGIC_USER : {
    lin_class = MAGE_LEVEL_IND;
    if (GET_LEVEL(ch, MAGE_LEVEL_IND) < 12)
      add_hp += number(2, 8);
    else
      add_hp += 2;
   }   break;

  case CLASS_SORCERER : {
    lin_class = SORCERER_LEVEL_IND;
    if (GET_LEVEL(ch, SORCERER_LEVEL_IND) < 12)
      add_hp += number(2, 8);
    else
      add_hp += 2;
   }   break;

  case CLASS_CLERIC : {
    lin_class = CLERIC_LEVEL_IND;
    if (GET_LEVEL(ch, CLERIC_LEVEL_IND) < 12)
      add_hp += number(2, 12);
    else
      add_hp += 5;
}   break;

  case CLASS_THIEF : {
    lin_class = THIEF_LEVEL_IND;
    if (GET_LEVEL(ch, THIEF_LEVEL_IND) < 12)
      add_hp += number(2,10);
    else
      add_hp += 4;
}   break;

  case CLASS_PSI : {
    lin_class = PSI_LEVEL_IND;
    if (GET_LEVEL(ch, PSI_LEVEL_IND) < 12)
      add_hp += number(2,10);
    else
      add_hp += 4;
}   break;

  case CLASS_WARRIOR : {
    lin_class = WARRIOR_LEVEL_IND;
    if (GET_LEVEL(ch, WARRIOR_LEVEL_IND) < 10)
      add_hp += number(2,16);
    else
      add_hp += 6;
}   break;

  case CLASS_PALADIN : {
    lin_class = PALADIN_LEVEL_IND;
    if (GET_LEVEL(ch, PALADIN_LEVEL_IND) < 10)
      add_hp += number(2,16);
    else
      add_hp += 6;
}   break;


  case CLASS_RANGER : {
    lin_class = RANGER_LEVEL_IND;
    if (GET_LEVEL(ch, RANGER_LEVEL_IND) < 11)
      add_hp += number(2,13);
    else
      add_hp += 6;
}   break;

  case CLASS_DRUID:{
   lin_class = DRUID_LEVEL_IND;
    if (GET_LEVEL(ch,  DRUID_LEVEL_IND) < 15)
      add_hp += number(2,12);
    else
      add_hp += 5;
}    break;

  case CLASS_MONK:{
    lin_class = MONK_LEVEL_IND;
    if (GET_LEVEL(ch, MONK_LEVEL_IND) < 17)
      add_hp += number(2,10);
    else
      add_hp += 4;
}   break;

  case CLASS_BARBARIAN : {
    lin_class = BARBARIAN_LEVEL_IND;
    if (GET_LEVEL(ch, BARBARIAN_LEVEL_IND) < 15)
      add_hp += number(2,14);
    else
      add_hp += 8;
}   break;

  } /*end switch */


  GET_LEVEL(ch, lin_class) -= 1;

  if (GET_LEVEL(ch, lin_class) < 1)
   {
    GET_LEVEL(ch, lin_class) = 1;
    if (ch->points.max_hit > 20)
      ch->points.max_hit = 20;
   }


  if (class = CLASS_WARRIOR)
    add_hp = MAX(add_hp, 6);
  if (class = CLASS_BARBARIAN)
    add_hp = MAX(add_hp, 8);
  if (class = CLASS_PALADIN)
    add_hp = MAX(add_hp, 6);
  if (class = CLASS_RANGER)
    add_hp = MAX(add_hp, 5);
  if (class = CLASS_CLERIC)
    add_hp = MAX(add_hp, 5);
  if (class = CLASS_THIEF)
    add_hp = MAX(add_hp, 4);
  if (class = CLASS_PSI)
    add_hp = MAX(add_hp, 4);
  if (class = CLASS_MAGIC_USER)
    add_hp = MAX(add_hp, 3);
  if (class = CLASS_SORCERER)
    add_hp = MAX(add_hp, 3);
  if (class = CLASS_MONK)
    add_hp = MAX(add_hp, 4);
  if (class = CLASS_DRUID)
    add_hp = MAX(add_hp, 5);


  add_hp /= HowManyClasses(ch);


  if (add_hp <=2) add_hp=3;

  ch->points.max_hit -= MAX(1,add_hp);
  if (ch->points.max_hit < 1)
    ch->points.max_hit = 1;

    ch->specials.spells_to_learn -= MAX(1, MAX(2, wis_app[GET_RWIS(ch)].bonus)/HowManyClasses(ch));

if (ch->points.exp >
    MIN(titles[lin_class][GET_LEVEL(ch, lin_class)].exp, GET_EXP(ch)))
     ch->points.exp =
       MIN(titles[lin_class][GET_LEVEL(ch, lin_class)].exp, GET_EXP(ch));

  if (ch->points.exp < 0)
    ch->points.exp = 0;

  send_to_char("You lose a level.\n\r", ch);

}



void set_title(struct char_data *ch)
{

  char buf[256];

  sprintf(buf,
     "%s the %s %s", GET_NAME(ch), RaceName[ch->race], ClassTitles(ch));

  if (GET_TITLE(ch)) {
    free(GET_TITLE(ch));
    CREATE(GET_TITLE(ch),char,strlen(buf)+1);
  } else {
    CREATE(GET_TITLE(ch),char,strlen(buf)+1);
  }

  strcpy(GET_TITLE(ch), buf);

}

void gain_exp(struct char_data *ch, int gain)
{
  int i;
  char buf[256];
  short chrace;
  save_char(ch,AUTO_RENT);

  if (!IS_PC(ch) && ch->master && IS_AFFECTED(ch, AFF_CHARM)) {
    if (ch->master->in_room == ch->in_room) {
      if (gain > 1) {
	gain/=2;
	sprintf(buf, "you gain $N's share of %d exp", gain);
	act(buf, 0, ch->master, 0, ch, TO_CHAR);
	gain_exp(ch->master, gain);
      }
    }
    return;
  }



  if (!IS_IMMORTAL(ch)) {
    if (gain > 0) {
      gain /= HowManyClasses(ch);

      if (GetMaxLevel(ch) == 1) {
	gain *= 2;
      }

      if (IS_PC(ch)) {
	if (ch->desc && ch->desc->original)
	  chrace = ch->desc->original->race;
	else
	  chrace = GET_RACE(ch);
	for (i = MAGE_LEVEL_IND; i < MAX_CLASS; i++) {
	  if (GET_LEVEL(ch,i)&&(GET_LEVEL(ch,i))<RacialMax[chrace][i])
	  {
   	    if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch,i)+2].exp)
   	    {
	      send_to_char("You must practice at a guild before you can gain any more experience\n\r", ch);
	      GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
	      return;
	    } else if (GET_EXP(ch) >= titles[i][GET_LEVEL(ch,i)+1].exp) {
	      /* do nothing..this is cool */
	    } else if (GET_EXP(ch)+gain >= titles[i][GET_LEVEL(ch,i)+1].exp) {
	      sprintf(buf, "You have gained enough to be a(n) %s\n\r", GET_CLASS_TITLE(ch, i, GET_LEVEL(ch, i)+1));
	      send_to_char(buf, ch);
	      send_to_char("You must return to a guild to earn the level\n\r",ch);
	      if (GET_EXP(ch)+gain >= titles[i][GET_LEVEL(ch,i)+2].exp) {
		GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
		return;
	      }
	    }
	  }
	}
      }

      GET_EXP(ch)+=gain;
      if (IS_PC(ch)) {
         for (i=MAGE_LEVEL_IND; i< MAX_CLASS; i++) {
           if (GET_LEVEL(ch,i)&&GET_LEVEL(ch, i)<RacialMax[chrace][i]) {
             if (GET_EXP(ch) > titles[i][GET_LEVEL(ch,i)+2].exp) {
               GET_EXP(ch) = titles[i][GET_LEVEL(ch,i)+2].exp - 1;
             }
          }
        }
      }
    }

    if (gain < 0) {
      GET_EXP(ch) += gain;
      if (GET_EXP(ch) < 0)
	GET_EXP(ch) = 0;
    }
  }
}


void gain_exp_regardless(struct char_data *ch, int gain, int class)
{
  int i;
  bool is_altered = FALSE;

  save_char(ch,AUTO_RENT);
  if (!IS_NPC(ch)) {
    if (gain > 0) {
      GET_EXP(ch) += gain;

      for (i=0;(i<ABS_MAX_LVL) &&(titles[class][i].exp <= GET_EXP(ch)); i++) {
	if (i > GET_LEVEL(ch,class)) {
	  send_to_char("You raise a level\n\r", ch);
	  GET_LEVEL(ch,class) = i;
	  advance_level(ch,class);
	  is_altered = TRUE;
	}
      }
    }
    if (gain < 0)
      GET_EXP(ch) += gain;
    if (GET_EXP(ch) < 0)
      GET_EXP(ch) = 0;
  }
  if (is_altered)
    set_title(ch);
}

void gain_condition(struct char_data *ch,int condition,int value)
{
  bool intoxicated;

  if(GET_COND(ch, condition)==-1) /* No change */
    return;

  intoxicated=(GET_COND(ch, DRUNK) > 0);

  GET_COND(ch, condition)  += value;

  GET_COND(ch,condition) = MAX(0,GET_COND(ch,condition));
  GET_COND(ch,condition) = MIN(24,GET_COND(ch,condition));

  if(GET_COND(ch,condition))
    return;

  switch(condition){
  case FULL :
    {
      send_to_char("$c0009You are hungry.$c0007\n\r",ch);
      return;
    }
  case THIRST :
    {
      send_to_char("$c0009You are thirsty.$c0007\n\r",ch);
      return;
    }
  case DRUNK :
    {
      if(intoxicated)
	send_to_char("You are now sober.\n\r",ch);
      return;
    }
    default : break;
  }

}


#define VOID_PULL_TIME   30
#define FORCE_RENT_TIME  40
void check_idling(struct char_data *ch)
{
char buf[255];

  void do_save(struct char_data *ch, char *argument, int cmd);


  if (++(ch->specials.timer) == 8) {
       do_save(ch, "", 0);

  } else if (ch->specials.timer == VOID_PULL_TIME) {

    if (ch->in_room != NOWHERE && ch->in_room != 0) {
      ch->specials.was_in_room = ch->in_room;
      if (ch->specials.fighting)     	{
	stop_fighting(ch->specials.fighting);
	stop_fighting(ch);
      }
      act("$n disappears into the void.", TRUE, ch, 0, 0, TO_ROOM);
      send_to_char("You have been idle, and are pulled into a void.\n\r", ch);
      char_from_room(ch);
      char_to_room(ch, 0);  /* Into room number 0 */
    }

  } else if (ch->specials.timer == FORCE_RENT_TIME)  {
      struct obj_cost cost;
	do_save(ch,"",0);
      if (ch->in_room != NOWHERE)
	char_from_room(ch);

      char_to_room(ch, 4);

      if (ch->desc)
	close_socket(ch->desc);

      ch->desc = 0;

      if (recep_offer(ch, NULL, &cost,TRUE)) {
   /* if above fails we lose their EQ! */
	cost.total_cost = 100;
	save_obj(ch, &cost, 1);
       } else {
         sprintf(buf,"%s had a failed recp_offer, they are losing EQ!",GET_NAME(ch));
         log(buf);
         slog(buf);
        }

      extract_char(ch);
    }
}



int ObjFromCorpse( struct obj_data *c)
{
  struct obj_data *jj, *next_thing;

  for(jj = c->contains; jj; jj = next_thing) {
    next_thing = jj->next_content; /* Next in inventory */
    if (jj->in_obj) {
      obj_from_obj(jj);
      if (c->in_obj)
	obj_to_obj(jj,c->in_obj);
      else if (c->carried_by) {
	obj_to_room(jj,c->carried_by->in_room);
	check_falling_obj(jj, c->carried_by->in_room);
      } else if (c->in_room != NOWHERE) {
	obj_to_room(jj,c->in_room);
	check_falling_obj(jj, c->in_room);
      } else
	assert(FALSE);
    } else {
      /*
       **  hmm..  it isn't in the object it says it is in.
       **  don't extract it.
       */
      c->contains = 0;
      log("Memory lost in ObjFromCorpse.");
      return(TRUE);
    }
  }
  extract_obj(c);
}




int ClassSpecificStuff( struct char_data *ch)
{
int i;

  if ( HasClass(ch, CLASS_WARRIOR)  || HasClass(ch, CLASS_MONK) ||
       HasClass(ch,CLASS_BARBARIAN) || HasClass(ch, CLASS_PALADIN) ||
       HasClass(ch,CLASS_RANGER))   {
       	// reset all attacks to 1.0
		ch->mult_att = 1.0;

	// apply modifiers for warrior-type classes
	if (HasClass(ch, CLASS_BARBARIAN))     {
		ch->mult_att+=(MIN(30,(GET_LEVEL(ch, BARBARIAN_LEVEL_IND)))*.05);
	}
	else if (HasClass(ch, CLASS_RANGER))     {
		ch->mult_att+=(MIN(30,(GET_LEVEL(ch, RANGER_LEVEL_IND)))*.05);
	}
	else if (HasClass(ch, CLASS_PALADIN))     {
		ch->mult_att+=(MIN(30,(GET_LEVEL(ch, PALADIN_LEVEL_IND)))*.05);
		if (GET_ALIGNMENT(ch) >= 350) {
      			SET_BIT(ch->specials.affected_by,AFF_DETECT_EVIL);
      			SET_BIT(ch->specials.affected_by,AFF_PROTECT_FROM_EVIL);
     		}
    	}
	else if (HasClass(ch, CLASS_WARRIOR))     {
      		ch->mult_att+=(MIN(30,(GET_LEVEL(ch, WARRIOR_LEVEL_IND)))*.05);
	}
	// special case for monks
	else {
     		if (HasClass(ch, CLASS_MONK)) {
			ch->mult_att+= (GET_LEVEL(ch, MONK_LEVEL_IND)/16.0);
     		}

    		/* fix up damage stuff */
      		switch(GET_LEVEL(ch, MONK_LEVEL_IND)) {
      			case 1:
      			case 2:
      			case 3:
				ch->specials.damnodice = 1;
				ch->specials.damsizedice = 3;
				break;
      			case 4:
      			case 5:
				ch->specials.damnodice = 1;
				ch->specials.damsizedice = 4;
				break;
      			case 6:
      			case 7:
      			case 8:
      			case 9:
      			case 10:
      			case 11:
				ch->specials.damnodice = 1;
				ch->specials.damsizedice = 6;
				break;
      			case 12:
      			case 13:
      			case 14:
				ch->specials.damnodice = 2;
				ch->specials.damsizedice = 3;
				break;
      			case 15:
      			case 16:
      			case 17:
      			case 18:
      			case 19:
				ch->specials.damnodice = 2;
				ch->specials.damsizedice = 4;
				break;
      			case 20:
      			case 21:
				ch->specials.damnodice = 3;
				ch->specials.damsizedice = 3;
				break;
      			case 22:
      			case 23:
      			case 24:
      			case 25:
      			case 26:
				ch->specials.damnodice = 3;
				ch->specials.damsizedice = 4;
				break;
      			case 27:
      			case 28:
      			case 29:
				ch->specials.damnodice = 4;
				ch->specials.damsizedice = 3;
				break;
      			case 30:
      			case 31:
      			case 32:
      			case 33:
      			case 34:
				ch->specials.damnodice = 4;
				ch->specials.damsizedice = 4;
				break;
      			case 35:
      			case 36:
				ch->specials.damnodice = 4;
				ch->specials.damsizedice = 5;
				break;
      			case 37:
      			case 38:
      			case 39:
      			case 40:
      			case 41:
      			case 42:
      			case 43:
      			case 44:
				ch->specials.damnodice = 5;
				ch->specials.damsizedice = 4;
				break;
      			case 45:
     			case 46:
      			case 47:
      			case 48:
      			case 49:
				ch->specials.damnodice = 6;
				ch->specials.damsizedice = 5;
				break;
      			case 50:
				ch->specials.damnodice = 8;
				ch->specials.damsizedice = 4;
				break;
      			default:
				ch->specials.damnodice=1;
				ch->specials.damsizedice = 2;
				break;
      		}
    	}


}


      /* other stuff.. immunities, etc, are set here */

  if (HasClass(ch, CLASS_MONK)) {
/*
*/
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 10)
      SET_BIT(ch->M_immune, IMM_HOLD);
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 18)
      SET_BIT(ch->immune, IMM_CHARM);
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 22)
      SET_BIT(ch->M_immune, IMM_POISON);
    if (GET_LEVEL(ch, MONK_LEVEL_IND) > 36)
      SET_BIT(ch->M_immune, IMM_CHARM);


  } else {
    if (HasClass(ch, CLASS_DRUID)) {
      if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 14) {
	SET_BIT(ch->immune, IMM_CHARM);
      }
      if (GET_LEVEL(ch, DRUID_LEVEL_IND) >= 32) {
	SET_BIT(ch->M_immune, IMM_POISON);
      }
    }

  }
}


