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

    for (d = descriptor_list; d; d = d->next) {
      person=(d->original?d->original:d->character);

		if(person) {
			if(real_roomp(person->in_room)->zone == zonenr) {
				count++;
			}
		}
	}
	return count;
}
/* part of royal rumble proc */

int count_People_in_room(int room) {
	  struct char_data *i;
	  int count=0;
		for (i=real_roomp(room)->people; i; i = i->next_in_room) {
			if(i) //this counts the number of people in just this room.
				count++;
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

//	if (IS_NPC(ch) || IS_IMMORTAL(ch))
//		return(FALSE);

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
			log("woah, you dare fight me? dead you are");
			act("$N is engulfed by $n!", FALSE, ch, 0, t, TO_NOTVICT);
			act("You are engulfed by $n, and are quickly disassembled",FALSE,ch,0,t,TO_VICT);
			act("$N is quickly reduced to a bloody pile of bones by $n",FALSE,ch,0,t,TO_NOTVICT);
			GET_HIT(ch) -= GET_HIT(t);
			die(t,NULL);
			/* find the corpse and destroy it */
			rp = real_roomp(ch->in_room);
			if (!rp) {
				log("invalid room in creeping death?! oddness!");
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
		}
		if (GET_HIT(ch) < 0) {
			log("death due to lack of hps");
			act("$n dissipates, you breathe a sigh of relief.",FALSE,ch,0,0,TO_ROOM);
			extract_char(ch);
			return(TRUE);
		}
		return(TRUE);
	}

	/* the generic is the direction of travel */
	if (number(0,1)==0) {  /* move */
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
				die(t,NULL);
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
		log("finished finding targets, wait for next func call");
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
int generate_legend_statue(struct char_data *ch, int cmd, char *arg, struct char_data *mob, int type)
{
	struct obj_data *obj;
	struct char_data *player;
	struct extra_descr_data *ed;
	char buf[254],name[254],shdesc[254],desc[254],exkey[254],exdesc[500];
	int itype = 0, rnum = 0, ammt = 0;
	FILE *fl;
	struct my_char_data {
		struct char_file_u grunt;  		/* contained in structs.h */
//	struct char_file_u dummy;
//		struct char_file_u_new grunt2;	/* contained here */
//		short AXE;
	}**dummy;


	printf("Does this run??");

	log("start proc");

	/* seems easiest to let this place update each time the zone inits */
	/* cycle through this for every player found in playerfile? */
	if (!(fl = fopen(PLAYER_FILE, "r"))) {
		log("Can not open playerfile in statue generation proc.");
		return(TRUE);
	}

	log("opened pfile");

	dummy=(struct my_char_data **)malloc(5500 * sizeof(dummy));
	if(dummy==NULL) {
		log("empty dummy in statue generation");
		return(TRUE);
	}

	log("shaped dummy?");

	for (;!feof(fl);) {
//		fread(&dummy, sizeof(struct char_file_u), 1, fl);

		sprintf(buf,"ammt: %d",ammt);
		log(buf);

		dummy[ammt]=(struct my_char_data *)malloc(sizeof(struct my_char_data));
		log("dummied again");
		if (!feof(fl)) {
			if(strcmp(dummy[ammt]->grunt.name,"111111") && strcmp(dummy[ammt]->grunt.name,"")) {
				/* determine if this player has a 10, 20 or 40k kill count */
				sprintf(buf,"found dummy: %s",dummy[ammt]->grunt.name);
				log(buf);
				if(dummy[ammt]->grunt./*specials.*/m_kills >= 10000) {
					if(dummy[ammt]->grunt./*specials.*/m_kills >= 40000) {
						itype = LEGEND_BIOGRAPHY;
						rnum = number(701,724);
						sprintf(name,"biography tome %s",dummy[ammt]->grunt.name);
						sprintf(shdesc,"a biography of %s",dummy[ammt]->grunt.name);
						sprintf(desc,"A large tome lies here, titled 'The Biography of %s'.",dummy[ammt]->grunt.name);
						sprintf(exkey,"biography tome %s",dummy[ammt]->grunt.name);
						sprintf(exdesc,"This book is a treatise on the life and accomplishments of %s.\n\rIt is an extensive volume, detailing many a feat. Most impressive.\n\r",dummy[ammt]->grunt.name);
					} else if(dummy[ammt]->grunt./*specials.*/m_kills >= 20000) {
						itype = LEGEND_PAINTING;
						rnum = number(726,735);
						sprintf(name,"painting %s",dummy[ammt]->grunt.name);
						sprintf(shdesc,"a painting of %s",dummy[ammt]->grunt.name);
						sprintf(desc,"On the wall, one can admire a painting of %s, slaying a fearsome beast.",dummy[ammt]->grunt.name);
						sprintf(exkey,"painting %s",dummy[ammt]->grunt.name);
						sprintf(exdesc,"%s is in the process of slaying a fearsome beast.\n\rTruly, %s is one of the greaters of these times.\n\r",dummy[ammt]->grunt.name,dummy[ammt]->grunt.name);
					} else {
						itype = LEGEND_STATUE;
						rnum = number(726,735);
						sprintf(name,"statue %s",dummy[ammt]->grunt.name);
						sprintf(shdesc,"a statue of %s",dummy[ammt]->grunt.name);
						sprintf(desc,"A statue of the legendary %s has been erected here.",dummy[ammt]->grunt.name);
						sprintf(exkey,"statue %s",dummy[ammt]->grunt.name);
						sprintf(exdesc,"This is a statue of %s, the legendary slayer.\n\r",dummy[ammt]->grunt.name);
					}
					if(type == 0) {
						log("Oddness in statue generation, no type found");
						return(TRUE);
					}
					if(rnum == 0) {
						log("Oddness in statue generation, no rnum found");
						return(TRUE);
					}
					/* load the generic item */
					obj = read_object(itype, REAL);
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
						obj->ex_description = ed;
						/* define the keywords */
						CREATE(ed->keyword, char, strlen(exkey) + 1);
						strcpy(ed->keyword, exkey);
						/* define the description */
						CREATE(ed->description, char, strlen(exdesc) + 1);
						strcpy(ed->description, exdesc);
					}
					/* and finally place it in a room */
					obj_to_room(obj,rnum);
				log("dummy had enough kills");
				}
				ammt++;
			}
		}
	}
	log("end for.. exit func");
}

#define CLIMB_ROOM 696
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
				send_to_char("One would have a marvellous view when high up in the canopy.\n\r",ch);
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

		act("$n knocks on the big door and then suddently, the big door opens up.",TRUE,ch,0,0,TO_ROOM);
		return(TRUE);
	}

	ch_printf(ch,"You knock on the big wooden door but nothing seems to happen.\n\r");
	return(TRUE);
}