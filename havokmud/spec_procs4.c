#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "protos.h"
/* #include "mail.h" 	 moved to protos.h */

/*   external vars  */
/*   external vars  */
  extern struct skillset warriorskills[];
  extern struct skillset scwarskills[];
  extern struct skillset thiefskills[];
  extern struct skillset barbskills[];
  extern struct skillset bardskills[];
  extern struct skillset monkskills[];
  extern struct skillset mageskills[];
  extern struct skillset clericskills[];
  extern struct skillset druidskills[];
  extern struct skillset paladinskills[];
  extern struct skillset rangerskills[];
  extern struct skillset psiskills[];
  extern struct skillset styleskillset[];
  extern struct skillset necroskills[];
extern struct room_data *world;
extern struct char_data *character_list;
extern struct descriptor_data *descriptor_list;
extern struct index_data *obj_index;
extern struct time_info_data time_info;
extern struct index_data *mob_index;
extern struct weather_data weather_info;
	extern int top_of_world;
	extern struct int_app_type int_app[26];
extern int RacialMax[][MAX_CLASS];

extern struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern struct spell_info_type spell_info[];
extern char *dirs[];
extern int rev_dir[];
extern struct zone_data *zone_table;

/* two global integers for Sentinel's cog room procedure */
extern int cog_sequence;
int chest_pointer = 0;



/* @Name: This is the chest part of the royal rumble proc.
 * @description: Once there is only 1 person left in the arena, the chest will open.
 * @Author: Greg Hovey (Banon)
 * @Assigned to obj/mob/room: Obj(51152)
*/
#define RUMBLE_ZONE 188
int chestproc(struct char_data *ch, int cmd, char *argument, struct obj_data *obj, int type)
{
     char buf[MAX_INPUT_LENGTH+80];
	 int count;

  if(cmd != 99) //open chest
    return(FALSE);

   dlog("in chestproc");

	if(countPeople(RUMBLE_ZONE)>1)  {
		//ch_printf(ch,"You can't open the chest yet, there is still people in the battle arena!%d", countPeople(RUMBLE_ZONE));
		ch_printf(ch,"The chest seems to resist your touch...",ch);
		return(TRUE);
	} else
		return(FALSE);
}

/* @Name: CountPeople in zone Function
 * @description: This function counts the number of people in a zone.
 * @Author: Greg Hovey (Banon)
 * @Used: used in chestproc and preparationproc
*/
int countPeople(int zonenr) {
   int count=0;
   struct descriptor_data *d;
   struct char_data *person;
/*
   for(d = descriptor_list; d; d = d->next)
   {
      person=(d->original?d->original:d->character);
		if(person) {
			if(person->in_room > 0) {
				if(real_roomp(person->in_room)->zone == zonenr) {
					count++;
				}
			}
		}
	}
	return count;
*/
  for(d = descriptor_list; d; d = d->next)
   {
      person=(d->original?d->original:d->character);

      if(person && real_roomp(person->in_room))
      {
         if(real_roomp(person->in_room)->zone == zonenr)
            count++;
      }
   }

	return count;

}
/* part of royal rumble proc */

int count_People_in_room(int room)
{
   struct char_data *i;
   int count=0;
	if(real_roomp(room))
   {
      for(i=real_roomp(room)->people; i; i = i->next_in_room)
      {
         if(i) //this counts the number of people in just this room.
            count++;
      }
   }
   return count;
}



/* @Name: This is the chest part of the royal rumble proc.
 * @description: Once there is only 1 person left in the arena, the chest will open.
 * @Author: Greg Hovey (Banon)
 * @Assigned to obj/mob/room: Room(51151)
*/
#define THE_HOUR  17
int preperationproc(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

void do_close(struct char_data *ch, char *argument, int cmd);
	  struct char_data *i;
	  int count=0, rand=0,x=0, zone=0;
		long curr_time;
		struct tm *t_info;


	if (cmd!=224)   //pull
		return(FALSE);


 curr_time = time(0);
  t_info = localtime(&curr_time);

	curr_time = time(NULL);
	//ch_printf(ch,"Well.. look at the time.. its %d ",t_info->tm_hour);




	count = count_People_in_room(ch->in_room);
	zone = countPeople(RUMBLE_ZONE);

	if(((zone-count )==1 || (zone-count)==0) && t_info->tm_hour==THE_HOUR){

		if(!IS_SET(rp->dir_option[2]->exit_info, EX_LOCKED) || !IS_SET(rp->dir_option[2]->exit_info, EX_CLOSED)) {
			do_close(ch, "door", 0);
			raw_lock_door( ch, EXIT(ch,2), 2);

		}
		ch_printf(ch,"You knock on the big wooden door and then slowly, it opens.\n\r");

		//send_to_char("1 person in arena.. boot someone new",ch);
		rand=number(0,count-1);

				for (i=real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
					if(i && x==rand) {
						send_to_char("You pull the rope of the gong.\n\r",ch);
						send_to_zone("$c000BThe gong sounds as someone new gets pushed into the arena.$c000w\n\r",ch);
						send_to_char("$c000BThe gong sounds as someone new gets pushed into the arena.$c000w\n\r",ch);
						char_from_room(i);
						send_to_char("You blink for a second and find yourself elsewhere.\n\r",i);

						char_to_room(i,number(51153,51158));
						do_look(i,"",0);
						return(TRUE);
					}
					x++;

 				}
	} else {
		//ch_printf(ch,"Number of people in zone:%d   Number of people in room:%d",zone, count);
		ch_printf(ch,"You ring the gong but nothing happens.\n\r");
		do_zload(ch,"188",0);
	}

 	return(TRUE);
}

/* Stuff for Heximal's new zones -Lennya 20030315*/
int riddle_exit(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	struct room_direction_data *exitp, *back;
	char guess[MAX_STRING_LENGTH +30], answer[MAX_STRING_LENGTH +30];
	char doorname[MAX_STRING_LENGTH +30], buf[MAX_STRING_LENGTH +30];
	char buffer[MAX_STRING_LENGTH +30];
	int doordir = 0;

	/* initialize info for different rooms here */
	/* use (0,1,2,3,4,5) for dirs (north,east,south,west,up,down) */
			if (ch->in_room == 50962) {
		sprintf(answer,"coffin");
		sprintf(doorname,"portal");
		doordir = 0;
	} else 	if (ch->in_room == 50963) {
		sprintf(answer,"fire");
		sprintf(doorname,"portal");
		doordir = 0;
	} else 	if (ch->in_room == 50964) {
		sprintf(answer,"rainbow");
		sprintf(doorname,"portal");
		doordir = 0;
	}

	rp = real_roomp(ch->in_room);
	exitp = rp->dir_option[doordir];
	if (!IS_SET(exitp->exit_info, EX_CLOSED))
		return(FALSE); /* if door is open, no need for proc */



	if (cmd == 258) { /* hey now, this one can't be opened with force! */
		sprintf(buf,"You charge at the %s, only to be repelled by a strange force.\n\r" , doorname);
		send_to_char(buf,ch);
		send_to_char("You are overcome by a strange exhaustion.\n\r", ch);
		sprintf(buf, "$n charges at the %s but is repelled by a strange force and collapses.", doorname);
		act(buf, FALSE, ch, 0, 0, TO_ROOM);
		GET_HIT(ch) -= number(1, 10)*2;
		if (GET_HIT(ch) < 0)
			GET_HIT(ch) = 0;
  		GET_POS(ch) = POSITION_STUNNED;
		return(TRUE);
	}

	if (cmd == 99) { /* open */
		only_argument(arg,buffer);
		if (*buffer) {
			if (str_cmp(buffer,doorname)) {
				return(FALSE);
			} else { /* open portal */
				sprintf(buf,"You attempt to open the %s, but a strange force prevents\n\r" , doorname);
				send_to_char(buf,ch);
				send_to_char("you from touching it. There must be another way to open it.\n\r", ch);
				sprintf(buf, "$n attemtps to open the %s, but seems unable to touch it.", doorname);
				act(buf, FALSE, ch, 0, 0, TO_ROOM);
				return(TRUE);
			}
		} else {
			return(FALSE);
		}
	}

	if (cmd == 17) { /* say */
		only_argument(arg,guess);
		if (*guess) {
			if (!(str_cmp(guess,answer))) {
				/* open the exit in this room */
				rp = real_roomp(ch->in_room);
				exitp = rp->dir_option[doordir];
				REMOVE_BIT(exitp->exit_info, EX_CLOSED);
				act("$n whispers something in a low voice, and the light in the room dims.",FALSE,ch,0,0,TO_ROOM);
				sprintf(buf, "You softly whisper '%s' and the light in the room dims.\n\r",answer);
				send_to_char(buf,ch);
				sprintf(buf,"The %s silently opens.\n\r",doorname);
				send_to_room(buf,ch->in_room);
				/* gotta open the exit at the other side as well */
				if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[doordir]]) && (back->to_room == ch->in_room)) {
      				REMOVE_BIT(back->exit_info, EX_CLOSED);
      				send_to_room(buf, exitp->to_room);
				}
				return(TRUE);
			} else {
				return(FALSE); /* wrong guess, too bad - just say the word */
			}
		} else {
			return(FALSE); /* nothing to say? that's a wrong answer for sure */
		}
	}
	return(FALSE);
}

int greed_disabler(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

	char buf[MAX_STRING_LENGTH +30];

	if (cmd == 60) { /* drop */
   		act("Just when $n is about to drop something, $e seems to change $s mind.", FALSE, ch, 0, 0, TO_ROOM);
        act("Drop something, with all these filthy thieves around? Better not, they're bound to steal it!",FALSE,ch,0,0,TO_CHAR);
        return (TRUE);
	}
	return(FALSE);
}

int lag_room(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

	char buf[MAX_STRING_LENGTH +30];

	if (cmd) { /* if a player enters ANY command, they'll experience two rounds of lag */
		if (IS_NPC(ch) || IS_IMMORTAL(ch))
			return(FALSE);
   		act("$n yawns and stretches $s muscles, such activity!", FALSE, ch, 0, 0, TO_ROOM);
        act("Yawning, you decide it's time to put in a bit of effort.",FALSE,ch,0,0,TO_CHAR);
        act("Though you sure are entitled to some rest after this.",FALSE,ch,0,0,TO_CHAR);
   		WAIT_STATE(ch, PULSE_VIOLENCE*2);
   		return(FALSE);
	}
}

int vampiric_embrace(struct char_data *ch, struct char_data *vict)
{
	struct obj_data *obj;
	int dam;

	if (IsImmune(vict, IMM_DRAIN))
		return(FALSE);

	if (ch->equipment[WIELD]) {
		obj = ch->equipment[WIELD];
		act("$c0008The negative aura surrounding your $p lashes out at $N, draining some of $S life.", FALSE, ch, obj, vict, TO_CHAR);
		act("$c0008The negative aura surrounding $n's $p lashes out at $N, draining some of $S life.", FALSE, ch, obj, vict, TO_NOTVICT);
		act("$c0008The negative aura surrounding $n's $p lashes out at you, draining some of your life.", FALSE, ch, obj, vict, TO_VICT);
	} else {
		act("$c0008The negative aura surrounding your hands lashes out at $N, draining some of $S life.", FALSE, ch, 0, vict, TO_CHAR);
		act("$c0008The negative aura surrounding $n's hands lashes out at $N, draining some of $S life.", FALSE, ch, 0, vict, TO_NOTVICT);
		act("$c0008The negative aura surrounding $n's hands lashes out at you, draining some of your life.", FALSE, ch, 0, vict, TO_VICT);
	}
	dam = dice(2,8);
	if(IsResist(vict, IMM_DRAIN)) /* half damage for resist */
		dam >>= 1;
	GET_HIT(ch) += dam;
	GET_HIT(vict) -= dam;
	return(FALSE);
}


int creeping_death( struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	struct char_data *t, *next;
	struct room_data *rp;
	struct obj_data *co, *o;

	if (cmd)
		return(FALSE);

	if (check_peaceful(ch,0)) {
		act("$n dissipates, you breathe a sigh of relief.",FALSE,ch,0,0,TO_ROOM);
		extract_char(ch);
		return(TRUE);
	}

	if (ch->specials.fighting && IS_SET(ch->specials.act,ACT_SPEC)) {  /* kill */
		t = ch->specials.fighting;
		if (t->in_room == ch->in_room) {
//			log("woah, you dare fight me? dead you are");
			act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
			act("You are engulfed by $n, and are quickly disassembled.",FALSE,ch,0,t,TO_VICT);
			act("$N is quickly reduced to a bloody pile of bones by $n.",FALSE,ch,0,t,TO_NOTVICT);
			GET_HIT(ch) -= GET_HIT(t);
			die(t,'\0');
			/* find the corpse and destroy it */
			rp = real_roomp(ch->in_room);
			if (!rp) {
				log("invalid room in creeping death?! oddness!");
				return(FALSE);
			}
			for (co = rp->contents; co; co = co->next_content) {
				if (IS_CORPSE(co))  { /* assume 1st corpse is victim's */
					while (co->contains) {
						o = co->contains;
						obj_from_obj(o);
						obj_to_room(o, ch->in_room);
					}
					extract_obj(co); /* remove the corpse */
				}
			}
		}
		if (GET_HIT(ch) < 0) {
//			log("death due to lack of hps");
			act("$n dissipates, you breathe a sigh of relief.",FALSE,ch,0,0,TO_ROOM);
			extract_char(ch);
			return(TRUE);
		}
		return(TRUE);
	}

	/* the generic is the direction of travel */
	if (number(0,2)==0) {  /* move */
		if (!ValidMove(ch, (ch->generic)-1)) {
			/* not sure why generic is dir+1, but the -1 fixes it -Lennya 20030405 */
			if(number(0,2)!=0) { /* 66% chance it dies */
				act("$n dissipates, you breathe a sigh of relief.",FALSE,ch,0,0,TO_ROOM);
				extract_char(ch);
				return(FALSE);
			} else
				return(FALSE);
		} else {
			do_move(ch, "\0", ch->generic);
			return(FALSE);
		}
	} else {
		/* make everyone with any brains flee */
		for (t = real_roomp(ch->in_room)->people; t; t = next) {
			next = t->next_in_room;
			if (t != ch) {
				if (!saves_spell(t, SAVING_PETRI)) {
					do_flee(t, "", 0);
				}
			}
		}
		/* find someone in the room to flay */
		for (t = real_roomp(ch->in_room)->people; t; t = next) {
			if (!t) {
				log("found no mobiles in creeping death?! oddness!");
				return(FALSE);
			}
			next = t->next_in_room;
			if (!IS_IMMORTAL(t) && t != ch  && number(0,1)==0  && IS_SET(ch->specials.act,ACT_SPEC)) {
				act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
				act("You are engulfed by $n, and are quickly disassembled.",FALSE,ch,0,t,TO_VICT);
				act("$N is quickly reduced to a bloody pile of bones by $n.",FALSE,ch,0,t,TO_NOTVICT);
				GET_HIT(ch) -= GET_HIT(t);
				die(t,'\0');
				/* find the corpse and destroy it */
				rp = real_roomp(ch->in_room);
				if (!rp) {
					log("invalid room called in creeping death?! oddness!");
					return(FALSE);
				}
				for (co = rp->contents; co; co = co->next_content) {
					if (IS_CORPSE(co))  {  /* assume 1st corpse is victim's */
						while (co->contains) {
							o = co->contains;
							obj_from_obj(o);
							obj_to_room(o, ch->in_room);
						}
						extract_obj(co);  /* remove the corpse */
					}
				}
				if (GET_HIT(ch) < 0) {
					act("$n dissipates, you breathe a sigh of relief.",FALSE,ch,0,0,TO_ROOM);
					extract_char(ch);
					return(TRUE);
				}
				break;  /* end the loop */
			}
		}
//		log("finished finding targets, wait for next func call");
	}
}

int Deshima(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	struct char_data *vict, *next, *i;
	struct room_data *rp;
	char buf[MAX_STRING_LENGTH +30];

	/* Deshima was meant to paralyze through immunity */

	if (!ch)
		return(FALSE);

	if (ch->in_room < 0)
		return(FALSE);

	rp = real_roomp(ch->in_room);

	if (!rp)
		return(FALSE);

	if (cmd || !AWAKE(ch))
		return(FALSE);

	if ((GET_DEX(ch)) != 21)
		GET_DEX(ch) = 21; /* Deshima wants to have a nice dex value - how about 21? */

    if (!ch->specials.fighting)
    	return(FALSE);
    else { /* we're fighting */
		vict = ch->specials.fighting;
		if (!IS_AFFECTED(vict, AFF_PARALYSIS)) {
			if (!(IS_IMMORTAL(vict)) && (ch != vict)) {
				switch(number(0,3)) { /* let's see if he got lucky (25%) */
					case 1: /* he got lucky */
						act("$c0008Deshima's eyes glow a dark c$c000rr$c000Rims$c000ro$c0008n as he gazes into $n's eyes.",FALSE,vict,0,0,TO_NOTVICT);
						act("$c0008$n's muscles seem to contract, but he manages to break away from Deshima's gaze in time.$c0007",FALSE,vict,0,0,TO_NOTVICT);
						sprintf(buf, "$c0008Deshima's eyes glow a dark c$c000rr$c000Rims$c000ro$c0008n as he gazes into your soul.\n\r");
						send_to_char(buf,vict);
						sprintf(buf, "$c0008You feel your muscles turn to stone, but manage to break the eye contact in time.$c0007\n\r");
						send_to_char(buf,vict);
						break;
					default: /* let's nail that bugger! */
						act("$c0008Deshima's eyes glow a dark c$c000rr$c000Rims$c000ro$c0008n as he gazes into $n's eyes.",FALSE,vict,0,0,TO_NOTVICT);
						act("$c0008$n's muscles turn all rigid, making him unable to fight.$c0007",FALSE,vict,0,0,TO_NOTVICT);
						sprintf(buf, "$c0008Deshima's eyes glow a dark c$c000rr$c000Rims$c000ro$c0008n as he gazes into your soul.\n\r");
						send_to_char(buf,vict);
						sprintf(buf, "$c0008You feel your muscles turn to stone, and lose the ability to fight back.$c0007\n\r");
						send_to_char(buf,vict);
						SET_BIT(vict->specials.affected_by, AFF_PARALYSIS);
						break;
				}
			}
		} else { /* he's paralyzed, let's look for a new victim */
			for (i = rp->people; i; i = next) {
				next = i->next_in_room;
				if (!(IS_IMMORTAL(i)) && (ch != i) && !IS_AFFECTED(i, AFF_PARALYSIS)) {
					/* We got a fresh target on our hands here, let's start picking on him */
					sprintf(buf,"$c0008Having effectively disabled %s, Deshima turns his attention to %s.$c0007",GET_NAME(vict),GET_NAME(i));
					act(buf,FALSE,i,0,0,TO_ROOM);
					sprintf(buf,"$c0008Having effectively disabled %s, Deshima turns his attention to you!$c0007\n\r",GET_NAME(vict));
					send_to_char(buf, i);
					stop_fighting(ch);
					set_fighting(ch, i);
					return(FALSE); /* exit the for loop */
				}
			}
			/* Found no valid victim to switch to, let's continue clobbering this paralyzed vict */
		}
	}
}

/*
#define GET_RSTR(ch)     ((ch)->abilities.str)

#define GET_RADD(ch)     ((ch)->abilities.str_add)

#define GET_RDEX(ch)     ((ch)->abilities.dex)

#define GET_RINT(ch)     ((ch)->abilities.intel)

#define GET_RWIS(ch)     ((ch)->abilities.wis)

#define GET_RCON(ch)     ((ch)->abilities.con)

#define GET_RCHR(ch)     ((ch)->abilities.chr);
*/

int TrainingGuild(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type) {
	char name[32];
	const struct skillset traininglist[] = {
	  { "hitpoints",    1,      2},
	  { "movement",  	2,      1},
	  { "mana",         3,      2},
	  { "constitution",	4,		 10},
	  { "strength",     5,      10},
	  { "dexterity",    6,      10},
	  { "charisma",     7,      5},
	  { "intelligence", 8,      12},
	  { "wisdom",       9,      12},
	  { "None",		    -1,	    -1}

	};
	int x=0, stat=0;

	if(!AWAKE(ch) || IS_NPC(ch))
		return(FALSE);

	if(cmd!=582 && cmd!=59)
		return(FALSE);



	if(cmd==582 && !*arg) { //list if no argument
		ch_printf(ch,"$c000B%-15s     %-3s\n\r------------------------\n\r","Stat","Cost");
		while(traininglist[x].level!=-1) {
		   ch_printf(ch,"$c000W %-15s     %-3d\n\r",traininglist[x].name, traininglist[x].level);
			x++;
		}
		return;
	} else
		sprintf(name,"%s",GET_NAME(mob));
		if(cmd==582) { //train

			//ch_printf(ch," Lets train your stats\n\r");

			while(traininglist[x].level!=-1) {
				if(is_abbrev(arg,traininglist[x].name)) {
					stat= x+1;
						if(GET_PRAC(ch) - traininglist[x].level < 0) {
							ch_printf(ch,"$c000P%s tells you 'You don't have enough practice sessions to learn %s.'\n\r",name,traininglist[x].name);
							return (TRUE);
						}

					break;
				}

				x++;

			}


			switch(stat) {
			 	case 1:
			 		GET_PRAC(ch) -= traininglist[stat-1].level;
			 		ch->points.max_hit ++; //GET_MAX_HIT(ch) = GET_MAX_HIT(ch) + 1;
			 		ch_printf(ch,"$c000P%s tells you 'Hey, take a drink of this! Its good for ya!!'\n\r$c000w%s hands you a foul looking health drink and you swig it down. (+1 HP)\n\r",name,name);
			 		break;
			 	case 2:
				 	GET_PRAC(ch)-= traininglist[stat-1].level;
					ch->points.max_move++;//GET_MAX_MOVE(ch) = GET_MAX_MOVE(ch) + 1;

				 	ch_printf(ch,"$c000P%s tells you 'Hey, take a swig of this!!!'\n\r$c000w%s hands you a high protein energy drink and you drink it down.(+1 Move)\n\r",name,name);
				 	break;
			 	case 3:
				 	GET_PRAC(ch)-= traininglist[stat-1].level;
					ch->points.max_mana ++;//GET_MAX_MANA(ch) = GET_MAX_MANA(ch) + 1;

				 	ch_printf(ch,"$c000P%s tells you 'This mystical drink should do it!!!'\n\r$c000wHe hands you a mystical potion and you chug it down.(+1 Mana)\n\r",name);
				 	break;
			 	case 4:
				 	if(GET_RCON(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.'\n\r",name, traininglist[stat-1].name,name);
					} else {
						GET_RCON(ch) = GET_RCON(ch)+1;
						GET_CON(ch)= GET_CON(ch)+1;

						GET_PRAC(ch) -= traininglist[stat-1].level;
					 	ch_printf(ch,"$c000P%s tells you 'Lets train your con!!!'\n\r$c000wYou heed his advice and go for a jog around the room.(+1 Con)\n\r",name);
					}
				 	break;
			 	case 5:
				 	if(GET_RSTR(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.'\n\r",name ,traininglist[stat-1].name,name);
					} else {
						GET_PRAC(ch) -= traininglist[stat-1].level;
						GET_RSTR(ch) = GET_RSTR(ch)+1;
						GET_STR(ch)  = GET_STR(ch)+1;

				 		ch_printf(ch,"You start lifting some weights.  You feel stronger!!!(+1 Str)'\n\r",name);
					}
				 	break;
			 	case 6:
				 	if(GET_RDEX(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.'\n\r",name, traininglist[stat-1].name,name);
					} else {
					 	GET_PRAC(ch) -= traininglist[stat-1].level;
					 	GET_RDEX(ch)= GET_RDEX(ch)+1;;
					 	GET_DEX(ch)= GET_DEX(ch)+1;
					 	ch_printf(ch,"%s shows you some stretches.  You mimic them!!! (+1 Dex)\n\r",name);
					}
					break;
			 	case 7:
					if(GET_RCHR(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.'\n\r",name, traininglist[stat-1].name,name);
					} else {
					 	GET_PRAC(ch) -= traininglist[stat-1].level;
					 	GET_RCHR(ch) = GET_RCHR(ch)+1;
					 	GET_CHR(ch)= GET_CHR(ch)+1;
					 	ch_printf(ch,"%s gives you some lessons in manners!!(+1 Chr)\n\r",name);
					}
					break;

			 	case 8:
			 		if(GET_RINT(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.\n\r",name, traininglist[stat-1].name,name);
					} else {
			 			GET_PRAC(ch) -= traininglist[stat-1].level;
			 			GET_RINT(ch) =GET_RINT(ch)+1;
			 			GET_INT(ch)= GET_INT(ch)+1;
			 			ch_printf(ch,"%s gives you a strange old book to read. You read it!!(+1 Int)\n\r",name);
					}
				 	break;
			 	case 9:
			 		if(GET_RWIS(ch) >= 18) {
						ch_printf(ch,"$c000P%s tells you 'I cannot train your %s any further.'\n\r",name, traininglist[stat-1].name,name);
					} else {
						GET_PRAC(ch) -= traininglist[stat-1].level;
						GET_RWIS(ch) = GET_RWIS(ch)+1;
						GET_WIS(ch)= GET_WIS(ch)+1;
					 	ch_printf(ch,"%s sits down and brings out the old chess board!!\n\rYou and him have a chat and play a few games.(+1 Wis)'\n\r",name);
					}
				 	break;
			 	default:
					ch_printf(ch,"$c000P%s tells you 'I'm not quite sure how to train that.'\n\r",name);
					break;
			}

	}

  }

int mermaid(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	struct char_data *i, *next;
	struct affected_type af;
	int r_num = 0;
	char buf[128];

	if (cmd || !AWAKE(ch))
		return(FALSE);

	if (GET_POS(ch)<POSITION_SITTING)
      return(FALSE);

    if (check_soundproof(ch))
    	return(FALSE);

	if (check_nomagic(ch, 0, 0))
		return(FALSE);

	/*if ch is fighting, don't fire */
	if (ch->specials.fighting)
		return(FALSE);

	/* if ch already has a follower, don't fire */
	if (ch->followers)
		return(FALSE);

	/* ch not fighting, let's look for a victim */
	if (ch->in_room > -1) {
		/* there's victims, let's see if we can harrass one */
	    for (i = real_roomp(ch->in_room)->people; i; i = next) {
			next = i->next_in_room;
			if ((GET_RACE(i) == RACE_HUMAN) && (GET_SEX(i) == SEX_MALE)) {
				if (!IS_NPC(i) && !IS_LINKDEAD(i) && !IS_IMMORTAL(i)
								&& !affected_by_spell(i,SPELL_CHARM_PERSON)) {
					if (!IsImmune(i, IMM_CHARM)) {
						if (!saves_spell(i, SAVING_PARA)) { /* didn't make his save, his ass is mine! */
							act("$n sings a beautiful song, oh my.. Your heart is sold to $m.",FALSE,ch,0,i,TO_VICT);
							act("$n hums a merry tune while looking $N in the eyes.",TRUE,ch,0,i,TO_NOTVICT);
							act("$N grins like the moron he is. $n's charms enchanted him.",TRUE,ch,0,i,TO_NOTVICT);
							if (i->master)
								  stop_follower(i);
							add_follower(i, ch);
							af.type      = SPELL_CHARM_PERSON;
							af.duration  = 24;
							af.modifier  = 0;
							af.location  = 0;
							af.bitvector = AFF_CHARM;
							affect_to_char(i, &af);
							return(TRUE);
						} else { /* made his save, give him some notice */
							act("$n sings a beautiful song, oh my.. You're almost willing to follow $m.",FALSE,ch,0,i,TO_VICT);
							act("$n hums a merry tune while looking $N in the eyes.",TRUE,ch,0,i,TO_NOTVICT);
						}
					} else { /* victim imm:charm, make him pay! */
						act("$n sings a beautiful song. At the end, you applaud and give $m some coins.",FALSE,ch,0,i,TO_VICT);
						act("$n hums a merry tune while looking $N in the eyes.",TRUE,ch,0,i,TO_NOTVICT);
						act("When it's finished, $N gives $m a round of applause and hands over some coins.",TRUE,ch,0,i,TO_NOTVICT);
						sprintf(buf,"20 coins %s",GET_NAME(ch));
						do_give(i,buf,0);
					}
				} /* npc, linkdead, immortal, or already charmed */
			} /* not a male human */
		} /* end for */
	} /* feh, noone here to harass */
}

/* procs for the King's Grove */
#define LEGEND_STATUE 701
#define LEGEND_PAINTING 703
#define LEGEND_BIOGRAPHY 702
int generate_legend_statue()//struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *obj;
	struct char_data *tmp;
	struct char_file_u player;
	struct extra_descr_data *ed;
	char buf[254],name[254],shdesc[254],desc[254],exdesc[500];
	int i = 0, itype = 0, rnum = 0;
	extern int top_of_p_table;
	extern struct player_index_element *player_table;

	/* Determine number of pfiles. Add one for last player made, though that
	 * one isn't very likely to have enough kills. Still, we wanna be thorough. */
	for(i=0;i<top_of_p_table+1;i++) {
		/* load up each of them */
		if (load_char((player_table + i)->name, &player) > -1) {
			/* store to a tmp char that we can deal with */
			CREATE(tmp, struct char_data,1);
			clear_char(tmp);
			store_to_char(&player, tmp);
			/* are they entitled to an item? */
			if(tmp->specials.m_kills >= 10000) {
				/* coolness, the are! Determine the item. */
				if(tmp->specials.m_kills >= 40000) {
					itype = LEGEND_BIOGRAPHY;
					rnum = number(2932,2934);
					sprintf(name,"biography tome %s",GET_NAME(tmp));
					sprintf(shdesc,"a biography of %s",GET_NAME(tmp));
					sprintf(desc,"A large tome lies here, titled 'The Biography of %s'.",GET_NAME(tmp));
					sprintf(exdesc,"This book is a treatise on the life and accomplishments of %s.\n\rIt is an extensive volume, detailing many a feat. Most impressive.",GET_NAME(tmp));
				} else if(tmp->specials.m_kills >= 20000) {
					itype = LEGEND_PAINTING;
					rnum = number(2926,2931);
					sprintf(name,"painting %s",GET_NAME(tmp));
					sprintf(shdesc,"a painting of %s",GET_NAME(tmp));
					sprintf(desc,"On the wall, one can admire a painting of %s, slaying a fearsome beast.",GET_NAME(tmp));
					sprintf(exdesc,"%s is in the process of slaying a fearsome beast.\n\rTruly, %s is one of the greatest of these times.",GET_NAME(tmp),GET_NAME(tmp));
				} else {
					itype = LEGEND_STATUE;
					rnum = number(2901,2924);
					sprintf(name,"statue %s",GET_NAME(tmp));
					sprintf(shdesc,"a statue of %s",GET_NAME(tmp));
					sprintf(desc,"A statue of the legendary %s has been erected here.",GET_NAME(tmp));
					sprintf(exdesc,"This is a statue of %s, the legendary slayer.",GET_NAME(tmp));
				}
				if(itype == 0) {
					log("Oddness in statue generation, no type found");
					return(TRUE);
				}
				if(rnum == 0) {
					log("Oddness in statue generation, no rnum found");
					return(TRUE);
				}
				/* load the generic item */
				if (obj = read_object(itype, VIRTUAL)) {
					/* and string it up a bit */
					if(obj->short_description) {
						free(obj->short_description);
						obj->short_description = strdup(shdesc);
					}
					if(obj->description) {
						free(obj->description);
						obj->description = strdup(desc);
					}
					if(obj->name) {
						free(obj->name);
						obj->name = strdup(name);
					}
					if(obj->ex_description) {
						log("trying to string invalid item in statue generation");
						return(TRUE);
					} else {
						/* create an extra desc structure for the object */
						CREATE(ed, struct extra_descr_data, 1);
						ed->next = obj->ex_description;
						obj->ex_description = ed;
						/* define the keywords */
						CREATE(ed->keyword, char, strlen(name) + 1);
						strcpy(ed->keyword, name);
						/* define the description */
						CREATE(ed->description, char, strlen(exdesc) + 1);
						strcpy(ed->description, exdesc);
					}
					/* and finally place it in a room */
					obj_to_room(obj,rnum);
				}
			}
			free(tmp);
		} else {
			log("screw up bigtime in load_char");
			return(TRUE);
		}
	}
	sprintf(buf,"processed %d pfiles for legend statue check", top_of_p_table+1);
	log(buf);
}

#define BERRYBASE 711
int pick_berries(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[MAX_INPUT_LENGTH];
	int affect = 1;
	int berry = 0;
	struct obj_data *obj;

	if (!ch)
		return(FALSE);
	if (!cmd)
		return(FALSE);
	if (cmd != 155)
		return(FALSE);
	else {
		only_argument(arg,buf);
  		if(*buf) {
			if(!(str_cmp("berry",buf)) || !(str_cmp("berries",buf))) {
				if(number(0,4)) {
					switch(GET_WIS(ch)+number(0,10)) {
						case 1:
						case 2:
						case 3:
						case 4:
						case 5:
						case 6:
						case 7:
							affect = 0;
							break;
						case 21:
						case 22:
						case 23:
						case 24:
						case 25:
							affect = 2;
							break;
						case 26:
						case 27:
						case 28:
							affect = 3;
							break;
						case 29:
							affect = 4;
							break;
						default:
							affect = 1;
							break;
					}
					berry = BERRYBASE + affect;
					act("You pick some berries.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n picks some berries.", FALSE, ch, 0, 0, TO_ROOM);
					WAIT_STATE(ch, PULSE_VIOLENCE);
					if ((real_object(berry)) >= 0) {
						obj = read_object(berry, VIRTUAL);
						obj_to_char(obj, ch);
					} else {
						log("no berries found for pick_berries");
					}
					return(TRUE);
				} else {
					act("You try to pick some berries, but hurt yourself on a thorn.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n tries to pick some berries, but hurts $mself on a thorn.", FALSE, ch, 0, 0, TO_ROOM);
					WAIT_STATE(ch, PULSE_VIOLENCE);
					return(TRUE);
				}
			}
		}
		return(FALSE);
	}
	return(FALSE);
}

#define ACORN 733
int pick_acorns(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[MAX_INPUT_LENGTH];
	int affect = 1;
	int berry = 0;
	struct obj_data *obj;

	if (!ch)
		return(FALSE);
	if (!cmd)
		return(FALSE);
	if (cmd != 155)
		return(FALSE);
	else {
		only_argument(arg,buf);
  		if(*buf) {
			if(!(str_cmp("acorn",buf)) || !(str_cmp("acorns",buf))) {
				if(number(0,2)) {
					act("You pick a delicious looking acorn.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n picks an acorn.", FALSE, ch, 0, 0, TO_ROOM);
					WAIT_STATE(ch, PULSE_VIOLENCE);
					if (obj = read_object(ACORN, VIRTUAL)) {
						obj_to_char(obj, ch);
					} else {
						log("no acorns found for pick_acorns");
					}
					return(TRUE);
				} else {
					act("You try to pick an acorn, but the one you want hangs just out of your reach.", FALSE, ch, 0, 0, TO_CHAR);
					act("$n tries to pick an acorn, but can't reach it.", FALSE, ch, 0, 0, TO_ROOM);
					WAIT_STATE(ch, PULSE_VIOLENCE);
					return(TRUE);
				}
			}
		}
		return(FALSE);
	}
	return(FALSE);
}


int legendfountain(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[MAX_INPUT_LENGTH];
	int level = 50;


	if (cmd != 11) /* drink */
		return(FALSE);

	else if (cmd==11) { /* drink */
		if(GET_COND(ch,THIRST)>18) {
			send_to_char("You don't feel thirsty, and decide against it.\n\r", ch);
			return(TRUE);
		}
		if(GET_COND(ch,FULL)>18) {
			send_to_char("You're too full to drink.\n\r", ch);
			return(TRUE);
		}

		send_to_char("You drink from the Fountain of Legends.\n\r", ch);
		act("$n drinks from the Fountain of Legends.", FALSE, ch, 0, 0, TO_ROOM);
//		GET_COND(ch,THIRST) = 24;
		GET_COND(ch,FULL)+=1;
		if(GET_COND(ch,FULL)>18) {
			act("Too much water has sated your appetite.",FALSE,ch,0,0,TO_CHAR);
			return(TRUE);
		}

		/* now for the magic spellup bonus */
		switch (number(0, 30)) {
			case 1:
				cast_refresh(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 2:
				cast_stone_skin(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 3:
				cast_cure_serious(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 4:
				cast_cure_light(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 5:
				cast_armor(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 6:
				cast_bless(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 7:
				cast_invisibility(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 8:
				cast_strength(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 9:
				cast_remove_poison(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 10:
				cast_true_seeing(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 11:
				cast_flying(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 12:
				cast_fly_group(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 13:
				cast_cure_critic(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 14:
				cast_heal(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 15:
				cast_second_wind(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 16:
				cast_water_breath(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 17:
				cast_shield(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 18:
				cast_find_traps(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 19:
				cast_protection_from_evil_group(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 20:
				cast_protection_from_evil(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 21:
				cast_sanctuary(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 22:
				cast_sense_life(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			case 23:
				cast_aid(level, ch, "", SPELL_TYPE_SPELL, ch, 0);
				break;
			default:
				send_to_char("Tasty stuff, no?\n\r", ch);
				break;
		}
		return(TRUE);
	}
}

#define GNOME_HOME 2897
#define GNOME_MOB  705
int gnome_home(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[254];
	struct char_data *gnome;

	if(cmd != 429) { /* knock */
		return(FALSE);
	}

	if(!ch)
		return(FALSE);

	if(!ch->in_room)
		return(FALSE);

	rp = real_roomp(ch->in_room);

	if(!rp)
		return(FALSE);

	only_argument(arg, buf);

	if(*buf) {
		if(!(str_cmp("door",buf)) || !(str_cmp("Door", buf)) || !(str_cmp("DOOR", buf))) { /* knock door */
			if(get_char_vis_world(ch,"gnome female collector",NULL)) {
				send_to_char("But it's already open!\n\r",ch);
				return(TRUE);
			} else if(gnome = read_mobile(GNOME_MOB, VIRTUAL)) {
				send_to_char("You courteously knock on the little door, and it opens.\n\r",ch);
				char_to_room(gnome, ch->in_room);
				send_to_char("Out comes a tiny gnomish woman, who peeps up at you.\n\r",ch);
				send_to_char("She says, 'You got anything for me? I'm getting desperate.'\n\r",ch);
				send_to_char("She looks at you in a meaningful way, obviously expecting something.\n\r",ch);
				return(TRUE);
			} else {
				log("could not find GNOME_MOB in gnome_home proc");
			}
		}
	}
	return(FALSE);
}

#define COLLECTIBLE_1 3998
#define COLLECTIBLE_2 47961
#define COLLECTIBLE_3 51838
#define COLLECTIBLE_4 51839
#define REWARD_GNOME    720
int gnome_collector(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[254], obj_name[120], vict_name[120];
	struct char_data *gnome, *tmp_ch, *j, *receiver;
	struct obj_data *obj, *i, *reward, *next;
	int obj_num = 0, x = 0;
	int HasCollectibles[4] = {0,0,0,0};
	int winner = 0, found = 0;

	if(!ch)
		return(FALSE);

	if(!AWAKE(ch))
		return(FALSE);

	if(ch->in_room)
		rp = real_roomp(ch->in_room);
	else {
		log("weirdness in gnome_collector, char not in a room");
		return(FALSE);
	}

	if(!rp) {
		log("weirdness in gnome_collector, char's room does not exist");
		return(FALSE);
	}

	/* let's make sure gnome doesn't get killed or robbed */
	if(!IS_SET(rp->room_flags,PEACEFUL))
		SET_BIT(rp->room_flags, PEACEFUL);

	if(!(gnome = get_char_room("gnome female collector",ch->in_room))) {
		sprintf(buf,"gnome_collector proc is attached to a mob without proper name, in room %d",ch->in_room);
		log(buf);
		return(FALSE);
	}

	if(!gnome) {
		log("weirdness in gnome_collector, gnome found but not assigned");
		return(FALSE);
	}

	if(!IS_NPC(gnome)) {
		log("weirdness in gnome_collector, gnome is not a mob");
		return(FALSE);
	}

	/* if there's no pcs present in room, I'm gonna get going, get some work done mate! */

	j = rp->people;
	found = 0;

	while(j && !found) {
		if(IS_PC(j)) {
			/* first PC in room (should be only cuz it's tunnel 1) receives prize, if winner */
			receiver = j;
			found = 1;
		}
		j = j->next_in_room;
	}

	if(!found) { /* no pcs in room! what am I doin here?! let's get lost */
		act("$n steps into $s little door, and it seals shut behind $s.",FALSE, gnome,0,0, TO_ROOM);
		while(gnome->carrying)
			extract_obj(gnome->carrying);
		extract_char(gnome);
		return(TRUE);
	}

	if(!IS_SET(gnome->specials.act, ACT_SENTINEL))
		SET_BIT(gnome->specials.act, ACT_SENTINEL);

	if(i = gnome->carrying) {
		for(i; i; i = i->next_content) {
			if(obj_index[i->item_number].virtual == COLLECTIBLE_1) {
				HasCollectibles[0] = 1;
			} else if(obj_index[i->item_number].virtual == COLLECTIBLE_2) {
				HasCollectibles[1] = 1;
			} else if(obj_index[i->item_number].virtual == COLLECTIBLE_3) {
				HasCollectibles[2] = 1;
			} else if(obj_index[i->item_number].virtual == COLLECTIBLE_4) {
				HasCollectibles[3] = 1;
			}
		}
		winner = 1;
		for (x = 0; x < 4; x++) {
			if(HasCollectibles[x]==0) {
				winner = 0;
			}
		}
		if(winner) {
			act("$n says, 'Woop, I got everyhitng i need now! Thank you ever so much.",FALSE, gnome,0,0, TO_ROOM);
			if(reward = read_object(REWARD_GNOME, VIRTUAL)) {
				act("I would express my gratitude by presenting you with this magical ring.",FALSE, gnome,0,0, TO_ROOM);
				act("I came across it in an ancient traveller's corpse, back in the day when",FALSE, gnome,0,0, TO_ROOM);
				act("I still got around. I never quite figured out its use, but I'm sure it's",FALSE, gnome,0,0, TO_ROOM);
				act("more than it seems. I hope you will make good use of it.'\n\r",FALSE, gnome,0,0, TO_ROOM);
				act("$N gives $p to $n.\n\r",FALSE, receiver, reward, gnome, TO_ROOM);
				act("$N gives $p to you.\n\r",FALSE, receiver, reward, gnome, TO_CHAR);

				obj_to_char(reward, receiver);
			}
			act("$n says, 'Right, gotta get going now, I'm impatient to start my experiments.",FALSE, gnome,0,0, TO_ROOM);
			act("$n steps into $s little door, and it seals shut behind $s.",FALSE, gnome,0,0, TO_ROOM);
			/* extract carried items if any */
			while(gnome->carrying)
				extract_obj(gnome->carrying);
			extract_char(gnome);
			return(TRUE);
		}
	}

	/* talk does nothing, she's silent */
	if(cmd == 531) {
		only_argument(arg,buf);
		if(*buf) {
			if(tmp_ch = get_char_room_vis(ch, buf)) {
				if(tmp_ch == gnome) { /* talk gnome */
					ch_printf(ch,"%s looks at you in a meaningful way, but stays silent.\n\r",gnome->player.short_descr);
					return(TRUE);
				}
			}
		}
		return(FALSE);
	}
	/* give */
	if (cmd == 72) {
		/* determine the correct obj */
		arg=one_argument(arg,obj_name);
		if (!*obj_name)
			return(FALSE);

		if (!(obj = get_obj_in_list_vis(ch, obj_name, ch->carrying)))
			return(FALSE);

		obj_num = obj_index[obj->item_number].virtual;

		arg=one_argument(arg, vict_name);

		if(!*vict_name)
			return(FALSE);

		if (!(tmp_ch = get_char_room_vis(ch, vict_name)))
			return(FALSE);

		if(tmp_ch != gnome)
			return(FALSE);

		/* found an object, and the correct person to give it to */

		if (gnome->specials.fighting) {
			send_to_char("Not while she is fighting!\n\r",ch);
			return(TRUE);
		}

		/* correct object? */
		if (obj_num != COLLECTIBLE_1 &&
			obj_num != COLLECTIBLE_2 &&
			obj_num != COLLECTIBLE_3 &&
			obj_num != COLLECTIBLE_4) {
			/* nope */
			ch_printf(ch,"%s doesn't seem to be interested in that sort of junk.\n\r",gnome->player.short_descr);
			return(TRUE);
		} else {
			act("$n says, 'Woah, good stuff! I've been looking for this thing for ages.'",FALSE, gnome,0,0, TO_ROOM);
			act("You give $p to $N.",FALSE, ch,obj,gnome, TO_CHAR);
			act("$n gives $p to $N.",FALSE, ch,obj,gnome, TO_ROOM);
			obj_from_char(obj);
			obj_to_char(obj, gnome);
			return(TRUE);
		}
	}
	return(FALSE);
}

#define QUEST_POTION 718
int qp_potion(struct char_data *ch, int cmd, char *arg)
{
	char buf[254], buffer[254];
	struct obj_data *found;
	struct obj_data *obj;
	int pot_rnr = 0;
	int has_pot = 0;

	if (cmd != 206) /* quaff */
		return(FALSE);

	only_argument(arg,buf);

	if(!(obj = get_obj_in_list_vis(ch,buf,ch->carrying))) {
		act("You do not have that item.",FALSE,ch,0,0,TO_CHAR);
		return (TRUE);
	}
	pot_rnr = real_object(QUEST_POTION);
	for (found = ch->carrying; found; found = found->next_content) {
		if (has_pot != 1) {
			if (found->item_number == pot_rnr)
				has_pot = 1;
			else
				has_pot = 0;
		}
	}

	if (has_pot == 1) {
		if (obj->item_number != pot_rnr)
			return(FALSE);
		act("$n quaffs $p.",TRUE,ch,obj,0,TO_ROOM);
		act("You quaff $p which dissolves.",FALSE,ch,obj,0,TO_CHAR);
		act("You have claimed your quest point!",FALSE,ch,obj,0,TO_CHAR);
		sprintf(buf,"%s just quaffed a quest potion.\n\r",GET_NAME(ch));
		qlog(buf);
		ch->player.q_points++;
		do_save(ch, "", 0);
		extract_obj(obj);
		return(TRUE);
	} else {
		return(FALSE);
	}
}

#define CLIMB_ROOM 2896
int climb_room(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	char buf[254], buffer[254];
	struct obj_data *obj;

	if (cmd == 5) { /*up*/
		act("How did ya want to get up there?",FALSE,ch,0,0,TO_CHAR);
		return(TRUE);
	}

	if (cmd == 15) { /*look*/
		only_argument(arg,buf);
		if(*buf) {
			if(!(str_cmp("up", buf)) || !(str_cmp("u", buf)) || !(str_cmp("Up", buf))) {
				send_to_char("One would have a marvelous view when high up in the canopy.\n\r",ch);
				return(TRUE);
			}
		}
	}

	if (cmd == 287) { /* climb */
		only_argument(arg,buf);
  		if(*buf) {
    		if(!(str_cmp("tree",buf)) || !(str_cmp("Tree",buf))) { /* climb tree */
				act("Freeing your hands, you climb up the tree.\n\r", FALSE, ch, 0, 0, TO_CHAR);
				act("$n stows away his weaponry and climbs up the tree.", FALSE, ch, 0, 0, TO_ROOM);
				/* remove weapon */
				if (ch->equipment[WIELD]) {
					obj=ch->equipment[WIELD];
					if ((obj = unequip_char(ch,WIELD))!=NULL)   {
						obj_to_char(obj, ch);
					}
				}
				/* remove held */
				if (ch->equipment[HOLD]) {
					obj=ch->equipment[HOLD];
					if ((obj = unequip_char(ch,HOLD))!=NULL)   {
						obj_to_char(obj, ch);
					}
				}
				/* remove light */
				if (ch->equipment[WEAR_LIGHT]) {
				obj=ch->equipment[WEAR_LIGHT];
					if ((obj = unequip_char(ch,WEAR_LIGHT))!=NULL)   {
						obj_to_char(obj, ch);
					}
				}
          		char_from_room(ch);
          		char_to_room(ch,CLIMB_ROOM);
          		act("$n climbs up from below.", FALSE, ch, 0, 0, TO_ROOM);
          		do_look(ch, "", 0);
          		return(TRUE);
			}
			sprintf(buffer,"You don't see any %s to climb around here.\n\r",buf);
			send_to_char(buffer,ch);
			return(TRUE);
		}
		send_to_char("Climb what?\n\r",ch);
		return(TRUE);
	}
	return(FALSE);
}

#define SHARPENING_STONE 735
void do_sharpen(struct char_data *ch, char *argument, int cmd)
{
	struct obj_data *obj, *cmp, *stone;
	char buf[254], buff[123];
	int w_type = 0;

	if(!ch || !cmd)
		return;

	if(cmd != 601) /* sharpen */
		return;

	if(ch->specials.fighting) {
		send_to_char("In the middle of a fight?! Hah.\n\r",ch);
		return;
	}

	if(ch->equipment) {
		if(stone = ch->equipment[HOLD]) {
			if(obj_index[stone->item_number].virtual != SHARPENING_STONE) {
				send_to_char("How can you sharpen stuff if you're not holding a sharpening stone?\n\r",ch);
				return;
			}
		} else {
			send_to_char("How can you sharpen stuff if you're not holding a sharpening stone?\n\r",ch);
			return;
		}
	}

	/* is holding the stone */
	if(!argument) {
		send_to_char("Sharpen what?\n\r",ch);
		return;
	}

	only_argument(argument,buf);
	if(!*buf) {
		send_to_char("Sharpen what?\n\r",ch);
		return;
	} else {
		if(obj = get_obj_in_list_vis(ch, buf, ch->carrying)) {
			if((ITEM_TYPE(obj) == ITEM_WEAPON)) {
				/* can only sharpen edged weapons */
				switch(obj->obj_flags.value[3]) {
					case 0  : w_type = TYPE_SMITE; break;
					case 1  : w_type = TYPE_STAB;  break;
					case 2  : w_type = TYPE_WHIP; break;
					case 3  : w_type = TYPE_SLASH; break;
					case 4  : w_type = TYPE_SMASH; break;
					case 5  : w_type = TYPE_CLEAVE; break;
					case 6  : w_type = TYPE_CRUSH; break;
					case 7  : w_type = TYPE_BLUDGEON; break;
					case 8  : w_type = TYPE_CLAW; break;
					case 9  : w_type = TYPE_BITE; break;
					case 10 : w_type = TYPE_STING; break;
					case 11 : w_type = TYPE_PIERCE; break;
					case 12 : w_type = TYPE_BLAST; break;
					case 13 : w_type = TYPE_IMPALE; break;
					case 14 : w_type = TYPE_RANGE_WEAPON; break;
					default : w_type = TYPE_HIT; break;
				}

				if ((w_type >= TYPE_PIERCE && w_type <= TYPE_STING)
					|| (w_type >= TYPE_CLEAVE && w_type <= TYPE_STAB)
					|| w_type == TYPE_IMPALE) {

					if(obj->obj_flags.value[2] == 0) {
						sprintf(buf,"%s tried to sharpen a weapon with invalid value: %s, vnum %d.",GET_NAME(ch),obj->short_description,obj->item_number);
						log(buf);
						return;
					}

					if(!(cmp = read_object(obj->item_number, REAL))) {
						log("Could not load comparison weapon in do_sharpen");
						return;
					}

					if(cmp->obj_flags.value[2] == 0) {
						sprintf(buf,"%s tried to sharpen a weapon with invalid value: %s, vnum %d.",GET_NAME(ch),obj->short_description,obj->item_number);
						log(buf);
						extract_obj(cmp);
						return;
					}

					if(cmp->obj_flags.value[2] == obj->obj_flags.value[2]) {
						send_to_char("That item has no need of your attention.\n\r",ch);
						extract_obj(cmp);
						return;
					} else {
						obj->obj_flags.value[2] = cmp->obj_flags.value[2];
						if(GET_POS(ch) > POSITION_RESTING)
							do_rest(ch, "", -1);
						sprintf(buf,"%s diligently starts to sharpen %s.",GET_NAME(ch),obj->short_description);
						act(buf,FALSE,ch,0,0,TO_ROOM);
						sprintf(buf,"You diligently sharpen %s.\n\r",obj->short_description);
						send_to_char(buf,ch);
						extract_obj(cmp);
						WAIT_STATE(ch, PULSE_VIOLENCE*2);
						return;
					}
				} else {
					send_to_char("You can only sharpen edged or pointy weapons.\n\r",ch);
					return;
				}
			} else {
				send_to_char("You can only sharpen weapons.\n\r",ch);
				return;
			}
		} else {
			send_to_char("You don't seem to have that.\n\r",ch);
			return;
		}
	}
	return;
}

/* end King's Grove */

/* Sentinel's Zone */

/* chests in room 51161
they type push cog in each of the room numbers in the right order will open
a different chest.

1. push cog in r#s 51195, 51204, 51231, 51258, 51300 opens vnum 51168.
2. push cog in r#s 51204, 51231, 51258, 51300, 51195 opens vnum 51169.
3. push cog in r#s 51231, 51258, 51300, 51195, 51204 opens vnum 51170.
4. push cog in r#s 51258, 51300, 51195, 51204, 51231 opens vnum 51171.
5. push cog in r#s 51300, 51258, 51231, 51204, 51195 opens vnum 51172.

If they push the cogs in the wrong order, then they have to come back after
repop and try again.

After they push each cog, should make some noise like: As the cog moves
into place, the hum of the machinery grows deafening.

If they push in wrong order, should have some message saying they failed
like: As the cog moves into place, the hum of the machinery ceases.

Sentinel
*/

#define CHESTS_ROOM 51161
int cog_room(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{
	struct obj_data *obj;
	char cogname[254];

	if(!cmd || !ch)
		return(FALSE);

	if(cmd != 374)
		return(FALSE);

	if(!(ch->in_room))
		return(FALSE);

	if(!*arg)
		return(FALSE);

	only_argument(arg, cogname);


	if(!strcmp(cogname, "cog") || !strcmp(cogname, "Cog") || !strcmp(cogname, "COG")) {
		if(cog_sequence < 0) {
			/* alas, you made an error earlier */
			send_to_char("Nothing seems to happen. the gears must have jammed somewhere.\n\r",ch);
			return(TRUE);
		} else if(cog_sequence == 0) {
			/* we got a new chance. determine chest by this room */
			switch(ch->in_room) {
				case 51195:
					chest_pointer = 51168;
					break;
				case 51204:
					chest_pointer = 51169;
					break;
				case 51231:
					chest_pointer = 51170;
					break;
				case 51258:
					chest_pointer = 51171;
					break;
				case 51300:
					chest_pointer = 51172;
					break;
				default:
					chest_pointer = 0;
					cog_sequence = 0;
					break;
			}
			cog_sequence++;
			send_to_room("As the cog moves into place, the hum of machinery running fills the air.\n\r",ch->in_room);
			return(TRUE);

		} else if(cog_sequence == 1) {
			if( (ch->in_room == 51204 && chest_pointer == 51168) ||
				(ch->in_room == 51231 && chest_pointer == 51169) ||
				(ch->in_room == 51258 && chest_pointer == 51170) ||
				(ch->in_room == 51300 && chest_pointer == 51171) ||
				(ch->in_room == 51258 && chest_pointer == 51172)) {

				cog_sequence++;
				send_to_room("As the cog moves into place, the hum of of the machines becomes louder.\n\r",ch->in_room);
			} else {
				cog_sequence = -1;
				send_to_room("As the cog moves into place, the hum of of the machines creaks and stops.\n\r",ch->in_room);
			}
			return(TRUE);

		} else if(cog_sequence == 2) {
			if( (ch->in_room == 51231 && chest_pointer == 51168) ||
				(ch->in_room == 51258 && chest_pointer == 51169) ||
				(ch->in_room == 51300 && chest_pointer == 51170) ||
				(ch->in_room == 51195 && chest_pointer == 51171) ||
				(ch->in_room == 51231 && chest_pointer == 51172)) {

				cog_sequence++;
				send_to_room("As the cog moves into place, the hum of of the machines becomes louder.\n\r",ch->in_room);
			} else {
				cog_sequence = -1;
				send_to_room("As the cog moves into place, the hum of of the machines creaks and stops.\n\r",ch->in_room);
			}
			return(TRUE);

		} else if(cog_sequence == 3) {
			if( (ch->in_room == 51258 && chest_pointer == 51168) ||
				(ch->in_room == 51300 && chest_pointer == 51169) ||
				(ch->in_room == 51195 && chest_pointer == 51170) ||
				(ch->in_room == 51204 && chest_pointer == 51171) ||
				(ch->in_room == 51204 && chest_pointer == 51172)) {

				cog_sequence++;
				send_to_room("As the cog moves into place, the hum of of the machines becomes louder.\n\r",ch->in_room);
			} else {
				cog_sequence = -1;
				send_to_room("As the cog moves into place, the hum of of the machines creaks and stops.\n\r",ch->in_room);
			}
			return(TRUE);

		} else if(cog_sequence == 4) {
			if( (ch->in_room == 51300 && chest_pointer == 51168) ||
				(ch->in_room == 51195 && chest_pointer == 51169) ||
				(ch->in_room == 51204 && chest_pointer == 51170) ||
				(ch->in_room == 51231 && chest_pointer == 51171) ||
				(ch->in_room == 51195 && chest_pointer == 51172)) {

				cog_sequence = -1;
				send_to_room("As the cog moves into place, the machine grinds to a halt and a loud click can be heard nearby.\n\r",ch->in_room);
				/* unlock and open chest */
				rp = real_roomp(CHESTS_ROOM);
				if(!rp) {
					log("no room found for chest storage in cog_room proc");
					return(TRUE);
				}
				for(obj = rp->contents; obj; obj = obj->next_content) {
					if(obj_index[obj->item_number].virtual == chest_pointer) {
						if(IS_SET(obj->obj_flags.value[1], CONT_LOCKED))
							REMOVE_BIT(obj->obj_flags.value[1], CONT_LOCKED);
						if (IS_SET(obj->obj_flags.value[1], CONT_CLOSED))
							REMOVE_BIT(obj->obj_flags.value[1], CONT_CLOSED);
						send_to_room("The machine grinds to a halt and one of the chests emits a loud click.\n\r",CHESTS_ROOM);
						return(TRUE);
					}
				}
				log("Couldn't find appropriate chest in CHESTS_ROOM");
				return(TRUE);
			} else {
				cog_sequence = -1;
				send_to_room("As the cog moves into place, the hum of of the machines creaks and stops.\n\r",ch->in_room);
			}
			return(TRUE);
		} else {
			log("got to bad spot in cog_room");
			return(TRUE);
		}
	}
	return(FALSE); /* trying to push something else than cog */
}

/* End Sentinel's Zone */

/* Rocky's Zone Stuff */
int close_doors(struct char_data *ch, struct room_data *rp, int cmd)
{
	struct room_direction_data *exitp, *back;
	char doorname[MAX_STRING_LENGTH +30], buf[MAX_STRING_LENGTH +30];
	char buffer[MAX_STRING_LENGTH +30];
	int doordir = 0;
	char *dir_name[] = {
       "to the north",
       "to the east",
       "to the south",
       "to the west",
       "above",
       "below"};

	/* initialize info for different rooms here */
	/* use (0,1,2,3,4,5) for dirs (north,east,south,west,up,down) */
			if (ch->in_room == 17429) {
		sprintf(doorname,"door");
		doordir = 3;
	} else 	if (ch->in_room == 17430) {
		sprintf(doorname,"door");
		doordir = 3;
	} else 	if (ch->in_room == 17431) {
		sprintf(doorname,"door");
		doordir = 3;
	} else 	if (ch->in_room == 17432) {
		sprintf(doorname,"door");
		doordir = 3;
	}

	rp = real_roomp(ch->in_room);
	exitp = rp->dir_option[doordir];

	if (IS_SET(exitp->exit_info, EX_CLOSED)) /* already closed, no need to run */
		return(FALSE);

	if (cmd) { /* when command is given, boom, door closed  and locked on this side */
		SET_BIT(exitp->exit_info, EX_CLOSED);
		SET_BIT(exitp->exit_info, EX_LOCKED);
		sprintf(buf,"The %s %s slams shut.\n\r\n\r",doorname, dir_name[doordir]);
		send_to_room(buf,ch->in_room);

		/* other side closes too, but not locked */
		if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[doordir]]) && (back->to_room == ch->in_room)) {
			SET_BIT(back->exit_info, EX_CLOSED);
			sprintf(buf,"The %s %s slams shut.\n\r",doorname, dir_name[rev_dir[doordir]]);
			send_to_room(buf, exitp->to_room);
		}

		return(FALSE);

	}
}

/* Ashamael's Citystate of Tarantis */
#define NIGHTWALKER 49600
#define SPAWNROOM 49799
#define DEST_ROOM 49460
#define REAVER_RM 49701
#define WAITROOM 49601

#define TARANTIS_PORTAL 49701
#define REAVER_PORTAL 49460

/* Proc that makes a portal spawn at nighttime 8pm, and close at dawn 5pm. */
int portal_regulator(struct char_data *ch, struct room_data *rp, int cmd)
{
	struct room_data *spawnroom;
	struct char_data *nightwalker;
	struct obj_data *obj;
	extern struct time_info_data time_info;
	char buffer[MAX_STRING_LENGTH +30];
	int check = 0;

	if(time_info.hours < 20 && time_info.hours > 5) { /* it should not be there */
		rp = real_roomp(WAITROOM);
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == TARANTIS_PORTAL) {
					send_to_room("$c0008The dark portal suddenly turns sideways, shrinks to a mere sliver, and disappears completely!\n\r",WAITROOM);
					extract_obj(obj);
				}
			}
		}
		rp = real_roomp(REAVER_RM);
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == TARANTIS_PORTAL) {
					send_to_room("$c0008The dark portal suddenly turns sideways, shrinks to a mere sliver, and disappears completely!\n\r",REAVER_RM);
					extract_obj(obj);
				}
			}
		}
		rp = real_roomp(DEST_ROOM);
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == REAVER_PORTAL) {
					send_to_room("$c0008The dark portal suddenly turns sideways, shrinks to a mere sliver, and disappears completely!\n\r",DEST_ROOM);
					extract_obj(obj);
				}
			}
		} /* all portals are gone, now do the transfer mob thing*/
		if (time_info.hours == 9) {
			spawnroom = real_roomp(SPAWNROOM);
			if (!spawnroom) {
				log("No nightwalker spawnroom found, blame Ash.");
				return(FALSE);
			}
			while (spawnroom->people) {
				nightwalker = spawnroom->people;
				char_from_room(nightwalker);
				char_to_room(nightwalker, WAITROOM);
			}
		}
	} else { /* portals should appear */
		rp = real_roomp(WAITROOM);
		check = 0;
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == TARANTIS_PORTAL) {
					check = 1;
				}
			}
			if(!check) {
				if(obj = read_object(TARANTIS_PORTAL, VIRTUAL)) {
					send_to_room("$c0008A sliver of darkness suddenly appears. It widens, turns sideways, and becomes a portal!\n\r",WAITROOM);
					obj_to_room(obj,WAITROOM);
				}
			}
		}
		rp = real_roomp(REAVER_RM);
		check = 0;
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == TARANTIS_PORTAL) {
					check = 1;
				}
			}
			if(!check) {
				if(obj = read_object(TARANTIS_PORTAL, VIRTUAL)) {
					send_to_room("$c0008A sliver of darkness suddenly appears. It widens, turns sideways, and becomes a portal!\n\r",REAVER_RM);
					obj_to_room(obj,REAVER_RM);
				}
			}
		}
		rp = real_roomp(DEST_ROOM);
		check = 0;
		if(rp) {
			for(obj = rp->contents; obj; obj = obj->next_content) {
				if(obj_index[obj->item_number].virtual == REAVER_PORTAL) {
					check = 1;
				}
			}
			if(!check) {
				if(obj = read_object(REAVER_PORTAL, VIRTUAL)) {
					send_to_room("$c0008A sliver of darkness suddenly appears. It widens, turns sideways, and becomes a portal!\n\r",DEST_ROOM);
					obj_to_room(obj,DEST_ROOM);
				}
			}
		}
	}
	return(FALSE);
}
#if 0
/* Proc that makes a door open at nighttime 8pm, and close at dawn 5pm. */
int timed_door(struct char_data *ch, struct room_data *rp, int cmd)
{
	struct room_direction_data *exitp, *back;
	struct room_data *spawnroom;
	struct char_data *nightwalker;
	extern int rev_dir[];
	extern struct time_info_data time_info;
	char doorname[MAX_STRING_LENGTH +30], buf[MAX_STRING_LENGTH +30];
	char buffer[MAX_STRING_LENGTH +30];
	int doordir = 0;
	char *dir_name[] = {
       "to the north",
       "to the east",
       "to the south",
       "to the west",
       "above",
       "below"};

	/* initialize exit info */
	sprintf(doorname,"door");
	doordir = 2;

	rp = real_roomp(WAITROOM);

	if(!rp) {
		log("No room found for timed_door proc");
		return(FALSE);
	}

	exitp = rp->dir_option[doordir];

	if(!exitp) {
		log("No exitp in said dir? Make that exit, Ash!");
		return(FALSE);
	}

	if(!exitp->exit_info) {
		log("No exit_info in said dir? Ash must have made a really weird room.");
		return(FALSE);
	}

	if (IS_SET(exitp->exit_info, EX_CLOSED)) { /* it's closed, should it be open? */
		if(time_info.hours > 19 || time_info.hours < 5) { /* it should indeed */
			if(SET_BIT(exitp->exit_info, EX_LOCKED))
				REMOVE_BIT(exitp->exit_info, EX_LOCKED);
			REMOVE_BIT(exitp->exit_info, EX_CLOSED);
			sprintf(buf,"The %s %s creaks open.\n\r",doorname, dir_name[doordir]);
			send_to_room(buf, WAITROOM);
			/* open other side as well */
			if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[doordir]]) && (back->to_room == WAITROOM)) {
				if(SET_BIT(back->exit_info, EX_CLOSED)) {
					if(SET_BIT(exitp->exit_info, EX_LOCKED))
						REMOVE_BIT(exitp->exit_info, EX_LOCKED);
					REMOVE_BIT(back->exit_info, EX_CLOSED);
					sprintf(buf,"The %s %s creaks open.\n\r",doorname, dir_name[rev_dir[doordir]]);
					send_to_room(buf, exitp->to_room);
				}
			}
		} else if (time_info.hours == 9) { /* they should all be fried by now */
			spawnroom = real_roomp(SPAWNROOM);
			if (!spawnroom) {
				log("No nightwalker spawnroom found, blame Ash.");
				return(FALSE);
			}
//			log("transferring nightwalkers");
			while (spawnroom->people) {
				nightwalker = spawnroom->people;
				char_from_room(nightwalker);
				char_to_room(nightwalker, WAITROOM);
			}
		}
	} else { /* it's open. should it be closed? */
		if(4 < time_info.hours && time_info.hours < 20) { /* yah man, let's close that bugger */
			SET_BIT(exitp->exit_info, EX_CLOSED);
			if(!SET_BIT(exitp->exit_info, EX_LOCKED))
				SET_BIT(exitp->exit_info, EX_LOCKED);
			sprintf(buf,"The %s %s slams shut.\n\r",doorname, dir_name[doordir]);
			send_to_room(buf, WAITROOM);
			/* close other side as well */
			if (exit_ok(exitp, &rp) && (back = rp->dir_option[rev_dir[doordir]]) && (back->to_room == WAITROOM)) {
				if(!SET_BIT(back->exit_info, EX_CLOSED)) {
					SET_BIT(back->exit_info, EX_CLOSED);
					if(!SET_BIT(exitp->exit_info, EX_LOCKED))
						SET_BIT(exitp->exit_info, EX_LOCKED);
				}
				sprintf(buf,"The %s %s slams shut.\n\r",doorname, dir_name[rev_dir[doordir]]);
				send_to_room(buf, exitp->to_room);
			}
		}
	}
	return(FALSE);
}
#endif

#define ING_1	720
#define ING_2	728
#define ING_3	729
#define ING_4	730
#define ING_5	44180

#define SMITH_SHIELD 727

int master_smith(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char buf[254];
	struct char_data *yeelorn;
	struct room_data *rp;
	struct obj_data *obj, *i, *obj1, *obj2, *obj3, *obj4, *obj5;
	int found = 0;
	int found1  = 0;
	int found2  = 0;
	int found3  = 0;
	int found4  = 0;
	int found5  = 0;
	char name1[254], name2[254], name3[254], name4[254], name5[254];

	extern struct obj_data  *object_list;

	if(!ch)
		return(FALSE);

	if(!cmd)
		return(FALSE);

	if (IS_NPC(ch))
		return(FALSE);

	if(ch->in_room)
		rp = real_roomp(ch->in_room);

	if(!rp)
		return(FALSE);

	if(!(yeelorn = FindMobInRoomWithFunction(ch->in_room, master_smith))) {
		log("proc assigned to a mob that cannot be found?!");
		return(FALSE);
	}

	if(cmd == 72) { /* give */
		send_to_char("Giving away your stuff for free? Better rethink this.\n\r",ch);
		return(TRUE);
	}

	if(cmd == 86) { /* ask */
		send_to_char("Yeelorn says, 'Aye, yer curious if yev got summat that I could use, eh? Right, lemme\n\r",ch);
		send_to_char("rummage through yer packs and see if there's aught I kin use fer raw material.'\n\r",ch);
		send_to_char("\n\r",ch);
		send_to_char("Yeelorn searches through your bags, mumbling to himself about people toting along\n\r",ch);
		send_to_char("all kinds of useless junk, and should they not put a little more thought to what\n\r",ch);
		send_to_char("they're packing and what a tasty looking loaf of pipeweed is that.'\n\r",ch);
		send_to_char("\n\r",ch);

		/* check items */
		for (i = object_list; i; i = i->next) {
			if (obj_index[i->item_number].virtual == ING_1) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch)
					found1 = 1;
			}
			if (obj_index[i->item_number].virtual == ING_2) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch)
					found2 = 1;
			}
			if (obj_index[i->item_number].virtual == ING_3) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch)
					found3 = 1;
			}
			if (obj_index[i->item_number].virtual == ING_4) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch)
					found4 = 1;
			}
			if (obj_index[i->item_number].virtual == ING_5) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch)
					found5 = 1;
			}
		}

		if(!found1 && !found2 && !found3 && !found4 && !found5) { /* nothing found */
				send_to_char("When he's finished, Yeelorn says, 'Hmm, no, ye gots nothin of interest to me, matey.\n\r",ch);
				send_to_char("Come back when ya got some. Quit wasting me time now, there's work to do.'\n\r",ch);
				return(TRUE);
		} else if(found1 && found2 && found3 && found4 && found5) { /* has everything, tell him the price */
				send_to_char("When he's finished, Yeelorn exclaims, 'Woah mate, I see ye'ns got everything I be needin\n\r",ch);
				send_to_char("to fix you up with a pretty shield. The shield of thought, to start out with. Then we\n\r",ch);
				send_to_char("gots to reinforce it with a dragon bone and a shield spell, enamel it with the silver\n\r",ch);
				send_to_char("from the sheet, and finally, we bring it to life with the runes from the ring. Aye, that\n\r",ch);
				send_to_char("will be a nice piece of work. Course, I'll have to charge ye a builder's fee. I'd think\n\r",ch);
				send_to_char("a million coins would do nicely. So. Whatcha think? You wanna buy this service from me?'\n\r",ch);
				return(TRUE);
		} else if(found1 || found2 || found3 || found4 || found5) { /* has at least one item */
			send_to_char("When he's finished, Yeelorn says, 'Hmm, I see ye've got yerself some pretty items.",ch);
			if(found1 == 1) {
				send_to_char("\n\rThis runed ring here, for instance. With the aid of them runes, I may be able to embue\n\r",ch);
				send_to_char("a creation of mine with some powerful magicks.  Aye, sure looks promisin.",ch);
			}
			if(found2 == 1) {
				send_to_char("\n\rThis here dragon bone looks like the right kind to reinforce a certain piece of armor\n\r",ch);
				send_to_char("with.  Oh golly, I bet I could do somethin funky with that.",ch);
			}
			if(found3 == 1) {
				send_to_char("\n\rWhat a pretty silver sheet ye've got there, mate.  Could put that to some good use\n\r",ch);
				send_to_char("if I were to have to pretty up a little summat or another.",ch);
			}
			if(found4 == 1) {
				send_to_char("\n\rHmmm, a scroll of shield.. I could use that when hammering some crafty bit of armor.",ch);
			}
			if(found5 == 1) {
				send_to_char("\n\rA shield with a mind of its own, eh? That looks int'resting. Bet I could sharpen up\n\r",ch);
				send_to_char("that mind a wee bit. Aye, but I'd need some more materials to do so..",ch);
			}
			/* maybe they'll get a hint */
			switch(number(1,20)) {
				case 1:
					if(!found1)
						send_to_char("\n\r\n\rThe other day, I heard there's this tiny gnome who's nicked herself a useful ring.",ch);
					break;
				case 2:
					if(!found2)
						send_to_char("\n\r\n\rYa know, dragonbone is ever a useful ingredient for crafty pieces of armor.",ch);
					break;
				case 3:
					if(!found3)
						send_to_char("\n\r\n\rI just ran out of silver filigree too. Maybe ye kin find me a sheet or two?",ch);
					break;
				case 4:
					if(!found4)
						send_to_char("\n\r\n\rAye, iffen ye want to imbue the metal with real power, ye'd need a scroll of spell too.",ch);
					break;
				case 5:
					if(!found5)
						send_to_char("\n\r\n\rHeard there's a shield with a mind of its own. Wouldn't that be something to see, eh?",ch);
					break;
				default:
					break;
			}
			send_to_char("'\n\r",ch);
		}
		return(TRUE);
	}

	if(cmd == 56) { /* buy */
		obj1 = obj2 = obj3 = obj4 = obj5 = 0;
		for (i = object_list; i; i = i->next) {
			if (obj_index[i->item_number].virtual == ING_1) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch){
					obj1 = i;
					found = 1;
				}
			}
			if (obj_index[i->item_number].virtual == ING_2) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch) {
					obj2 = i;
					found = 1;
				}
			}
			if (obj_index[i->item_number].virtual == ING_3) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch) {
					obj3 = i;
					found = 1;
				}
			}
			if (obj_index[i->item_number].virtual == ING_4) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch) {
					obj4 = i;
					found = 1;
				}
			}
			if (obj_index[i->item_number].virtual == ING_5) {
				obj = i;
				while(obj->in_obj)
					obj = obj->in_obj;
				if(obj->carried_by == ch || obj->equipped_by == ch) {
					obj5 = i;
					found = 1;
				}
			}
			if(found) { /* transfer items to inventory for easy work later */
				if(i->equipped_by) {
					obj = unequip_char(ch, i->eq_pos);
					obj_to_char(obj, ch);
				} else if(i->carried_by) {
					/* is okay */
				} else if(i->in_obj) {
					obj_from_obj(i);
					obj_to_char(i, ch);
				} else {
					log("where is this item!?! bad spot in master_smith");
					send_to_char("Ugh, something wrong with this proc, sorry.\n\r",ch);
					return(TRUE);
				}
			}
			found = 0;
		}

		if(obj1 && obj2 && obj3 && obj4 && obj5) {
			/* got all the items.. how about a million coins? */
			if(GET_GOLD(ch) < 1000000) {
				send_to_char("Yeelorn says, 'Aye laddie, ye've got yerself the items, but yer still some money short.'\n\r",ch);
				return(TRUE);
			} else {
				GET_GOLD(ch) -= 1000000;
				if(obj1->carried_by) {
					obj_from_char(obj1);
					extract_obj(obj1);
				}
				if(obj2->carried_by) {
					obj_from_char(obj2);
					extract_obj(obj2);
				}
				if(obj3->carried_by) {
					obj_from_char(obj3);
					extract_obj(obj3);
				}
				if(obj4->carried_by) {
					obj_from_char(obj4);
					extract_obj(obj4);
				}
				if(obj5->carried_by) {
					obj_from_char(obj5);
					extract_obj(obj5);
				}

				if (obj = read_object(SMITH_SHIELD, VIRTUAL)) {
					obj_to_char(obj, ch);
					send_to_char("You give your items to Yeelorn, along with an incredible heap of coins.\n\r",ch);
					send_to_char("Yeelorn pokes up his forge, and starts heating the shield. Once it's red hot,\n\r",ch);
					send_to_char("he bends a few edges, bangs a few times, and the dragonbone collar is firmly\n\r",ch);
					send_to_char("attached. He then proceeds with folding the silver sheet over the shield, and\n\r",ch);
					send_to_char("heats it some more. More hammering melds the silver with the shield and bone,\n\r",ch);
					send_to_char("making it look rather impressive. Then, Yeelorn places it in a barrel ot brine,\n\r",ch);
					send_to_char("causing great clouds of noxious fumes to fill the air. Next he orates the prayer\n\r",ch);
					send_to_char("from the scroll, which bursts into flame while the spell is sucked into the\n\r",ch);
					send_to_char("shield. Once more he heats it up, and when it's about to go to pieces from the\n\r",ch);
					send_to_char("heat, he takes it out and presses the runed ring in the center of it. The shield\n\r",ch);
					send_to_char("seems to shudder. That must have been your imagination. Shields are not alive.\n\r",ch);
					send_to_char("Or are they?\n\r",ch);
					send_to_char("Once the shield has cooled down, Yeelorn takes a rag, and polishes it til it\n\r",ch);
					send_to_char("shines as bright as full moon. He giggles as he hands it over.\n\r",ch);
					send_to_char("'Bout time yeh got on with yer adventuring' he says, as he winks at you.\n\r",ch);
				} else {
					log("could not load up prize for master_smith proc");
					send_to_char("Ugh, something wrong with this proc, sorry..\n\r",ch);
				}
			}
			return(TRUE);
		} /* not all items are in posession, buy does nothing special */
	}
	return(FALSE);
}

int nightwalker(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	char buf[254];
	struct char_data *freshmob;
	struct room_data *rp;
	struct obj_data *obj;
	int roomnr = 0, found = 0;

	if(!ch)
		return(FALSE);

	if (!IS_NPC(ch))
		return(FALSE);

	/* if event death, do the die thing and load up a new mob at a spawn_room */
	if (type == EVENT_DEATH) {
		freshmob = read_mobile(real_mobile(NIGHTWALKER),REAL);
    	char_to_room(freshmob, SPAWNROOM);
    	act("$n crumbles to a pile of dust, but a tiny cloud seems to escape.", FALSE, ch, 0, 0, TO_ROOM);
		return(TRUE);
	}
	/* else if light && not in spawn_room && not in a DARK room then
	 * burn to a cinder, and load up a new one in the spawn room */
	if(ch->in_room != SPAWNROOM && ch->in_room != WAITROOM && (!IS_SET(real_roomp(ch->in_room)->room_flags, DARK))) {
		if (time_info.hours > 6 && time_info.hours < 19) {
	    	act("A young ray of sunlight peeps over the horizon and strikes $n.", FALSE, ch, 0, 0, TO_ROOM);
	    	act("The power of the beam instantly reduces $n to a pile of dust!", FALSE, ch, 0, 0, TO_ROOM);
			GET_HIT(ch) = -1;
			die(ch, '\0');
			return(TRUE);
		}
	}

	/* Make them enter portal if they're in the right spot */
	if(ch->in_room) {
		roomnr = ch->in_room;
		if(roomnr == WAITROOM) {
			if(rp = real_roomp(roomnr)) {
				for(obj = rp->contents; obj; obj = obj->next_content) {
					if(obj_index[obj->item_number].virtual == TARANTIS_PORTAL) {
						do_enter(ch, "portal", 7);
						return(FALSE);
					}
				}
			}
		}
	}
	return(FALSE);
}

/* End Tarantis */

/* start shopkeeper .. this to make shops easier to build  -Lennya  20030731*/
int shopkeeper(struct char_data *ch, int cmd, char *arg, struct char_data *shopkeeper, int type)
{
	struct room_data *rp;
	struct char_data *j, *next;
	struct obj_data *obj, *cond_ptr[50], *store_obj;
	char buf[120], itemname[120], newarg[100];
	float modifier = 1.0;
	int cost = 0, chr = 1, k, i = 0, stop = 0, num = 1, rnum = 0;
	int tot_cost = 0, cond_top = 0, cond_tot[50], found=FALSE;

	extern struct str_app_type str_app[];


	if(!ch)
		return(FALSE);

//	if(!cmd)
//		return(FALSE);

	if(!AWAKE(ch) || IS_NPC(ch))
		return(FALSE);

	/* define the shopkeeper */
	if(ch->in_room)
		rp = real_roomp(ch->in_room);
	else {
		log("weirdness in shopkeeper, char not in a room");
		return(FALSE);
	}

	if(!rp) {
		log("weirdness in shopkeeper, char's room does not exist");
		return(FALSE);
	}

	/* let's make sure shopkeepers don't get killed or robbed */
	if(!IS_SET(rp->room_flags,PEACEFUL))
		SET_BIT(rp->room_flags, PEACEFUL);

	if (cmd !=  59 &&	/* list */
		cmd !=  56 &&	/* buy */
		cmd !=  93 &&	/* offer */
		cmd !=  57)		/* sell */
		return(FALSE);

	for(j = rp->people; j; next) {
		next = j->next_in_room;
		if(j->specials.proc == PROC_SHOPKEEPER) {
			shopkeeper = j;
			break;
		}
	}

	if(!shopkeeper) {
		log("weirdness in shopkeeper, shopkeeper assigned but not found");
		return(FALSE);
	}

	if(!IS_NPC(shopkeeper)) {
		log("weirdness in shopkeeper, shopkeeper is not a mob");
		return(FALSE);
	}

	if(!IS_SET(shopkeeper->specials.act, ACT_SENTINEL))
		SET_BIT(shopkeeper->specials.act, ACT_SENTINEL);

	/* players with 14 chr pay avg price */
	chr = GET_CHR(ch);
	if(chr < 1)
		chr = 1;
	modifier = (float)14/chr;

	/* list */
	if(cmd == 59) {
		ch_printf(ch,"This is what %s currently has on store:\n\r\n\r",shopkeeper->player.short_descr);
		send_to_char("  Count  Item                                       Price\n\r",ch);
		    send_to_char("$c0008*---------------------------------------------------------*\n\r",ch);
		obj = shopkeeper->carrying;
		if(!obj) {
			send_to_char("$c0008|$c0007        Nothing.                                         $c0008|\n\r",ch);
		} else {

			for (obj=shopkeeper->carrying; obj; obj = obj->next_content) {
				if (CAN_SEE_OBJ(ch, obj)) {
					if (cond_top < 50) {
						found = FALSE;
						for (k=0;(k < cond_top && !found); k++) {
							if (cond_top > 0) {
								if ((obj->item_number == cond_ptr[k]->item_number) &&
									(obj->short_description && cond_ptr[k]->short_description &&
									(!strcmp(obj->short_description,cond_ptr[k]->short_description)))) {
									cond_tot[k] += 1;
									found=TRUE;
								}
							}
						}
						if (!found) {
							cond_ptr[cond_top] = obj;
							cond_tot[cond_top] = 1;
							cond_top+=1;
						}
					} else {
						cost = (int) obj->obj_flags.cost * modifier;
						if(cost <0)
							cost = 0;
						cost += 1000; /* Trader's fee = 1000 */
						ch_printf(ch,"$c0008|$c0007    1   %-41s %6d $c0008|\n\r",obj->short_description, cost);
					}
				}
			}

			if (cond_top) {
				for (k=0; k<cond_top; k++) {
					if (cond_tot[k] > 1) {
						cost = (int) cond_ptr[k]->obj_flags.cost * modifier;
						if(cost <0)
							cost = 0;
						cost += 1000; /* Trader's fee = 1000 */
						ch_printf(ch,"$c0008|$c0007 %4d   %-41s %6d $c0008|\n\r",cond_tot[k],cond_ptr[k]->short_description, cost);
					} else {
						cost = (int) cond_ptr[k]->obj_flags.cost * modifier;
						if(cost <0)
							cost = 0;
						cost += 1000; /* Trader's fee = 1000 */
						ch_printf(ch,"$c0008|$c0007    1   %-41s %6d $c0008|\n\r",cond_ptr[k]->short_description, cost);
					}
				}
			}
		}
		send_to_char("$c0008*---------------------------------------------------------*\n\r",ch);
	}

	/* buy */
	if(cmd == 56) {
		only_argument(arg,itemname);
		if(!*itemname) {
			send_to_char("Buy what?\n\r",ch);
			return(TRUE);
		} else {
			if ((num = getabunch(itemname,newarg))!=FALSE) {
				strcpy(itemname,newarg);
			}
			if (num < 1)
				num = 1;
			rnum = 0;
			stop = 0;
			i = 1;

			while (i <= num && stop == 0) {
				if (obj = get_obj_in_list_vis(ch, itemname, shopkeeper->carrying)) {
					cost = (int) obj->obj_flags.cost * modifier;
					if(cost <0)
						cost = 0;
					cost += 1000; /* Trader's Fee is 1000 */

					if(GET_GOLD(ch) < cost) {
						send_to_char("Alas, you cannot afford that.\n\r",ch);
						stop = 1;
					} else if ((IS_CARRYING_N(ch) + 1) > (CAN_CARRY_N(ch))) {
						ch_printf(ch,"%s : You can't carry that many items.\n\r", obj->short_description);
						stop = 1;
					} else if ((IS_CARRYING_W(ch) + (obj->obj_flags.weight)) > CAN_CARRY_W(ch)) {
						ch_printf(ch,"%s : You can't carry that much weight.\n\r", obj->short_description);
						stop = 1;
					} else if (GET_LEVEL(ch,BARBARIAN_LEVEL_IND) !=0 &&
										anti_barbarian_stuff(obj) && GetMaxLevel(ch)<LOW_IMMORTAL) {
						send_to_char("You sense magic on the object and think better of buying it.\n\r",ch);
						stop = 1;
					} else {
						obj_from_char(obj);
						obj_to_char(obj, ch);
						GET_GOLD(ch) -= cost;
						GET_GOLD(shopkeeper) += cost;
						store_obj = obj;
						i++;
						tot_cost += cost;
						rnum++;
					}
				} else if (rnum > 0) {
					ch_printf(ch,"Alas, %s only seems to have %d %ss on store.\n\r",shopkeeper->player.short_descr, rnum, itemname);
					stop = 1;
				} else {
					ch_printf(ch,"Alas, %s doesn't seem to stock any %ss..\n\r",shopkeeper->player.short_descr, itemname);
					stop = 1;

				}
			}
		}

		if(rnum == 1) {
			ch_printf(ch,"You just bought %s for %d coins.\n\r",store_obj->short_description, cost);
			act("$n buys $p from $N.", FALSE, ch, obj, shopkeeper, TO_ROOM);
		} else if(rnum > 1) {
			ch_printf(ch,"You just bought %d items for %d coins.\n\r", rnum, tot_cost);
			act("$n buys some stuff from $N.", FALSE, ch, obj, shopkeeper, TO_ROOM);
		}
	}

	/* sell */
	if(cmd == 57) {
		only_argument(arg,itemname);
		if(!*itemname) {
			send_to_char("Sell what?\n\r",ch);
			return(TRUE);
		} else {
			if (obj = get_obj_in_list_vis(ch, itemname, ch->carrying)) {
				cost = (int) obj->obj_flags.cost/(3*modifier);
				if(cost < 400) {
					ch_printf(ch,"%s doesn't buy worthless junk like that.\n\r",shopkeeper->player.short_descr);
					return(TRUE);
				} else {
					if(GET_GOLD(shopkeeper) < cost) {
						ch_printf(ch, "Alas, %s cannot afford that right now.\n\r",shopkeeper->player.short_descr);
						return(TRUE);
					} else {
						obj_from_char(obj);
						obj_to_char(obj, shopkeeper);
						ch_printf(ch,"You just sold %s for %d coins.\n\r",obj->short_description, cost);
						act("$n sells $p to $N.", FALSE, ch, obj, shopkeeper, TO_ROOM);
						GET_GOLD(ch) += cost;
						GET_GOLD(shopkeeper) -= cost;
						return(TRUE);
					}
				}
			} else {
				ch_printf(ch,"Alas, you don't seem to have any %ss to sell.\n\r",itemname);
				return(TRUE);
			}
		}
	}

	/* offer */
	if(cmd == 93) {
		only_argument(arg,itemname);
		if(!*itemname) {
			ch_printf(ch,"What would you like to offer to %s?\n\r",shopkeeper->player.short_descr);
			return(TRUE);
		} else {
			if (obj = get_obj_in_list_vis(ch, itemname, ch->carrying)) {
				cost = (int) obj->obj_flags.cost/(3*modifier);
				if(cost < 400) {
					ch_printf(ch,"%s doesn't buy worthless junk like that.\n\r",shopkeeper->player.short_descr);
					return(TRUE);
				} else {
					ch_printf(ch,"%s is willing to pay you %d coins for %s.\n\r",shopkeeper->player.short_descr,cost,obj->short_description);
					return(TRUE);
				}
			} else {
				ch_printf(ch,"You don't seem to have any %ss.\n\r",itemname);
				return(TRUE);
			}
		}
	}
	return(TRUE);
}


/* end shopkeeper */

int WeaponsMaster(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	extern const struct skillset weaponskills[];
 	int count = 0;
	char buf[256], buffer[MAX_STRING_LENGTH];
	static int percent = 0;
	static int x=0; //for loop
	int i = 0; //while loop

#if 1
	if(!AWAKE(ch) || IS_NPC(ch))
		return(FALSE);

	//170->Practice,164->Practise, 243->gain
	if (cmd==164 || cmd == 170) {

		if(!OnlyClass(ch,CLASS_WARRIOR)) {
			send_to_char("Only single class warriors can learn weapons styles proficiently.",ch);
			return(FALSE);
		}


		if(!*arg && (cmd == 170 || cmd == 164)) { /* practice||practise, without argument */
			sprintf(buffer,"You have got %d practice sessions left.\n\r\n\r",ch->specials.spells_to_learn);
			sprintf(buf,"You can practice any of these weapon styles:\n\r\n\r");
			strcat(buffer,buf);
			x = 50;
			/* list by level, so new skills show at top of list */
				i=0;
				while(weaponskills[i].level != -1) {
					if (weaponskills[i].level <= x) {
						sprintf(buf,"$c000B[$c000W%-2d$c000B]$c000W %-30s %-15s",weaponskills[i].level,
								weaponskills[i].name,
								ch->skills[weaponskills[i].skillnum].learned >0? "$c000Y($c000BPracticed$c000Y)$c000w":"$c000Y($c000RUnknown$c000Y)$c000w");
						if (IsSpecialized(ch->skills[weaponskills[i].skillnum].special))
							strcat(buf," (special)");
						strcat(buf," \n\r");
						if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
							break;
						strcat(buffer, buf);
						strcat(buffer, "\r");
					}
					i++;
				}

			page_string(ch->desc, buffer, 1);
			return(TRUE);
		} else {
			x=0;
			while (weaponskills[x].level != -1) {
				if(is_abbrev(arg,weaponskills[x].name)) {  //!str_cmp(arg,n_skills[x])){
					if(weaponskills[x].level > 51) {
						send_to_char("$c0013[$c0015Weapon Master$c0013] tells you"
								" 'You're not experienced enough to learn this weapon.'",ch);
						return(TRUE);
					}

					if(ch->skills[weaponskills[x].skillnum].learned > 0) {
						//check if skill already practiced
						send_to_char("$c0013[$c0015Weapon Master$c0013] tells you"
									 " 'You already look quite knowledgeable of that weapon.'\n\r",ch);
						return(TRUE);
					}

					if(ch->specials.spells_to_learn <=0) {
						send_to_char("$c0013[$c0015Weapon Master$c0013] tells you"
									" 'You don't have enough practice points.'\n\r",ch);
						return(TRUE);
					}

					sprintf(buf,"%s shows you how to use the %s correctly.\n\r",GET_NAME(mob),weaponskills[x].name);
					send_to_char(buf,ch);
					ch->specials.spells_to_learn--;

					if(!IS_SET(ch->skills[weaponskills[x].skillnum].flags,SKILL_KNOWN)) {
						SET_BIT(ch->skills[weaponskills[x].skillnum].flags,SKILL_KNOWN);
						SET_BIT(ch->skills[weaponskills[x].skillnum].flags,SKILL_KNOWN);
					}
					percent=ch->skills[weaponskills[x].skillnum].learned+int_app[GET_INT(ch)].learn;
					ch->skills[weaponskills[x].skillnum].learned = MIN(95,percent);
					if(ch->skills[weaponskills[x].skillnum].learned > 0)
						send_to_char("'You are now a master of this art.'\n\r",ch);
					return(TRUE);
				}
				x++;
			}
			send_to_char("$c0013[$c0015Weapon Master$c0013] tells you '"
							"I do not know of that skill!'\n\r",ch);
			return(TRUE);
		}
	}
#endif
	return (FALSE);
}
int knockproc(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type) {

		long curr_time;
		struct tm *t_info;

	log("Knockproc");
	if (cmd!=429)   //knock
		return(FALSE);


 curr_time = time(0);
  t_info = localtime(&curr_time);

	curr_time = time(NULL);
	//ch_printf(ch,"Well.. look at the time.. its %d ",t_info->tm_hour);


	if(t_info->tm_hour != THE_HOUR && (IS_SET(rp->dir_option[0]->exit_info, EX_LOCKED)))  {
		do_zload(ch,"188",0);
		raw_unlock_door( ch, EXIT(ch,0), 0);
		open_door(ch, 0);
		ch_printf(ch,"You knock on the big wooden door and then slowly, it opens.\n\r");

		act("$n knocks on the big door and then suddenly, the big door opens up.",TRUE,ch,0,0,TO_ROOM);
		return(TRUE);
	}

	ch_printf(ch,"You knock on the big wooden door but nothing seems to happen.\n\r");
	return(TRUE);
}








int NecromancerGuildMaster(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	int count = 0;
	char buf[256], buffer[MAX_STRING_LENGTH];
	static int percent = 0;
	static int x=0; //for loop
	int i = 0; //while loop
	struct char_data *guildmaster;

#if 1
	if(!AWAKE(ch) || IS_NPC(ch))
		return(FALSE);

	//170->Practice,164->Practise, 243->gain
	if (cmd==164 || cmd == 170 || cmd == 243) {

		if (!HasClass(ch, CLASS_NECROMANCER)) {
			send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you"
							" 'You're not a necromancer.'\n\r",ch);
			return(TRUE);
		}
		if(!mob) {
			guildmaster = FindMobInRoomWithFunction(ch->in_room, NecromancerGuildMaster);
		} else {
			guildmaster = mob;
		}
		if (GET_LEVEL(ch,NECROMANCER_LEVEL_IND) > GetMaxLevel(guildmaster)) {
			send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you"
							" 'You must learn from another, I can no longer train you.'\n\r",ch);
			return(TRUE);
		}

		//gain
		if(cmd == 243 && GET_EXP(ch)<titles[NECROMANCER_LEVEL_IND][GET_LEVEL(ch,NECROMANCER_LEVEL_IND)+1].exp) {
			send_to_char("Your not ready to gain yet!",ch);
			return (FALSE);
		} else {
			if(cmd == 243) {  //gain
				GainLevel(ch,NECROMANCER_LEVEL_IND);
				return (TRUE);
			}
		}

		if(!*arg && (cmd == 170 || cmd == 164)) { /* practice||practise, without argument */
			sprintf(buffer,"You have got %d practice sessions left.\n\r\n\r",ch->specials.spells_to_learn);
			sprintf(buf,"You can practice any of these spells and skills:\n\r\n\r");
			strcat(buffer,buf);
			x = GET_LEVEL(ch,NECROMANCER_LEVEL_IND);
			/* list by level, so new skills show at top of list */
			while (x != 0) {
				while(necroskills[i].level != -1) {
					if (necroskills[i].level == x) {
						sprintf(buf,"[%-2d] %-30s %-15s",necroskills[i].level,
								necroskills[i].name,how_good(ch->skills[necroskills[i].skillnum].learned));
						if (IsSpecialized(ch->skills[necroskills[i].skillnum].special))
							strcat(buf," (special)");
						strcat(buf," \n\r");
						if (strlen(buf)+strlen(buffer) > (MAX_STRING_LENGTH*2)-2)
							break;
						strcat(buffer, buf);
						strcat(buffer, "\r");
					}
					i++;
				}
				i=0;
				x--;
			}
			page_string(ch->desc, buffer, 1);
			return(TRUE);
		} else {
			x=0;
			while (necroskills[x].level != -1) {
				if(is_abbrev(arg,necroskills[x].name)) {  //!str_cmp(arg,n_skills[x])){
					if(necroskills[x].level > GET_LEVEL(ch,NECROMANCER_LEVEL_IND)) {
						send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you"
								" 'You're not experienced enough to learn this skill.'",ch);
						return(TRUE);
					}

					if(ch->skills[necroskills[x].skillnum].learned > 45) {
						//check if skill already practiced
						send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you"
									 " 'You must learn from experience and practice to get"
									 " any better at that skill.'\n\r",ch);
						return(TRUE);
					}

					if(ch->specials.spells_to_learn <=0) {
						send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you"
									" 'You don't have enough practice points.'\n\r",ch);
						return(TRUE);
					}

					sprintf(buf,"You practice %s for a while.\n\r",necroskills[x].name);
					send_to_char(buf,ch);
					ch->specials.spells_to_learn--;

					if(!IS_SET(ch->skills[necroskills[x].skillnum].flags,SKILL_KNOWN)) {
						SET_BIT(ch->skills[necroskills[x].skillnum].flags,SKILL_KNOWN);
						SET_BIT(ch->skills[necroskills[x].skillnum].flags,SKILL_KNOWN_NECROMANCER);
					}
					percent=ch->skills[necroskills[x].skillnum].learned+int_app[GET_INT(ch)].learn;
					ch->skills[necroskills[x].skillnum].learned = MIN(95,percent);
					if(ch->skills[necroskills[x].skillnum].learned >= 95)
						send_to_char("'You are now a master of this art.'\n\r",ch);
					return(TRUE);
				}
				x++;
			}
			send_to_char("$c0013[$c0015The Necromancer Guildmaster$c0013] tells you '"
							"I do not know of that skill!'\n\r",ch);
			return(TRUE);
		}
	}
#endif
	return (FALSE);
}

int generic_guildmaster(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	int count = 0;
	char buf[256], buffer[MAX_STRING_LENGTH], classname[120];
	static int percent = 0;
	static int x=0;
	int i = 0, class = 0, level_ind = 0, level = 0;
	struct char_data *guildmaster, *j, *next;
	struct room_data *rp;
	struct skillset skillz[] = {};
	long s_known;
	int end = 0;

	if(!AWAKE(ch) || IS_NPC(ch))
		return(FALSE);

	if(!ch->in_room)
		return(FALSE);

	if(!(rp = real_roomp(ch->in_room)))
		return(FALSE);

	for(j = rp->people; j; next) {
		next = j->next_in_room;
		if(j->specials.proc == PROC_GUILDMASTER) {
			guildmaster = j;
			break;
		}
	}

	if(!guildmaster) {
		log("weirdness in generic_guildmaster, assigned but not found");
		return(FALSE);
	}

	if(!IS_NPC(guildmaster)) {
		log("weirdness in guildmaster, is not a mob");
		return(FALSE);
	}

	if (cmd!=164 && cmd != 170 && cmd != 243)
		return(FALSE);


	/* let's see which GM we got here */
	if(IS_SET(guildmaster->specials.act, ACT_MAGIC_USER)) {
		MageGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_CLERIC)) {
		ClericGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_WARRIOR)) {
		WarriorGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_THIEF)) {
		ThiefGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_DRUID)) {
		DruidGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_MONK)) {
		monk_master(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_BARBARIAN)) {
		barbarian_guildmaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_PALADIN)) {
		PaladinGuildmaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_RANGER)) {
		RangerGuildmaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_PSI)) {
		PsiGuildmaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_BARD)) {
		BardGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else if(IS_SET(guildmaster->specials.act, ACT_NECROMANCER)) {
		NecromancerGuildMaster(ch, cmd, 0, guildmaster, 0);
	} else {
		log("guildmaster proc attached to mobile without a class, autoset to warrior");
		SET_BIT(guildmaster->specials.act, ACT_WARRIOR);
	}
	return(TRUE);
}

int troll_regen(struct char_data *ch)
{
	assert(ch);

	if(GET_HIT(ch) >= GET_MAX_HIT(ch))
		return(FALSE);

	if(number(0,2))
		return;

	GET_HIT(ch) += number(1,3);
	act("$n's wounds seem to close of their own.",FALSE, ch, 0, 0, TO_ROOM);
	act("Your wounds close of their own accord.",FALSE, ch, 0, 0, TO_CHAR);
}

int remort_guild(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type) {

	void do_mobTell2(struct char_data *ch,struct char_data *mob, char *sentence);
	int classcombos[] = {
		CLASS_MAGIC_USER+CLASS_CLERIC+CLASS_WARRIOR+CLASS_THIEF,
	    CLASS_DRUID+CLASS_RANGER+CLASS_WARRIOR,
		CLASS_SORCERER+CLASS_CLERIC+CLASS_WARRIOR+CLASS_THIEF,
		CLASS_RANGER+CLASS_DRUID,
		CLASS_PSI+CLASS_WARRIOR+CLASS_THIEF,
		CLASS_NECROMANCER+CLASS_WARRIOR+CLASS_THIEF,
		-1
	};
	int x, hasclass=0, num=0, choose=-1, avg=0;
	extern const char *class_names[];

	char classname[128];


	if(cmd!=605)
		return(FALSE);

	only_argument(arg, classname);

	if(!*classname) {
		do_mobTell2(ch, mob, "A class you must choose!");
		return(TRUE);
	}



	/*lets count what classes ch has now */

	for (x=0; x < MAX_CLASS; x++) {
	 	if (HasClass(ch, pc_num_class(x))) {
			hasclass+=pc_num_class(x);
			num+=1;
			avg+=ch->player.level[x];
	 	}
		if(is_abbrev(classname, class_names[x])) {
			choose=x;
			//ch_printf(ch,"You choose class# x%d as a new class.pc%d.\n\r",x,pc_num_class(x));
		}
	}

	if(avg/num<50 || GET_LEADERSHIP_EXP(ch) < 50000000) { /* Average level is 50?? they maxxed out?? */
		do_mobTell2(ch,mob,"You don't look strong enough or worthy enough to be in my presences.");
		return (TRUE);
	}
	/*didn't find a class argument */
	if(choose==-1) {
		do_mobTell2(ch, mob, "That profession is unknown to me!");
		return(TRUE);
	}
	/* Check to see if they have that class */
	if(HasClass(ch, pc_num_class(choose))) {
		do_mobTell2(ch, mob, "You already know enough about that class!");
		return(TRUE);
	}
	/*See if they have too many classes*/
	if(num>2) {
		do_mobTell2(ch, mob, "I'm afraid you already have too many professions!");
		return(TRUE);
	}

	hasclass+=pc_num_class(choose);

	x=0;
	while(classcombos[x] !=-1) {
		if(IS_SET(classcombos[x], hasclass)) {
			do_mobTell2(ch, mob, "You may now know the art of this class!");
			ch_printf(ch,"You just obtained a new class!!! %d",pc_num_class(choose));
			ch->player.class=hasclass;
			do_restore(ch, GET_NAME(ch), 0);
			ch->specials.remortclass=choose;

			ch->points.max_hit=ch->points.max_hit / GET_CON(ch);


			/*reset the char */
			for (x=0; x < MAX_CLASS; x++) {
	 			if (HasClass(ch, pc_num_class(x))) { //set all classes to level 1
					ch->player.level[x]=1;

				} else
					ch->player.level[x]=0;
			}
			GET_EXP(ch)=1;
			GET_LEADERSHIP_EXP(ch)=1;
			return(TRUE);
		}
		x++;
	}
	/* No class combos found */
	do_mobTell2(ch, mob, "You can't multi-class with that class!");
	return(TRUE);

}

