
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>

#include "protos.h"

#define NOT !
#define AND &&
#define OR ||

#define STATE(d) ((d)->connected)
#define MAX_CMD_LIST 497
#define STAT_SWORD(x) ((x >= 1 && x <= 3) ? "-]>>>" : ((x >= 4 && x<= 6) ? "-]>>>>" : ((x >= 7 && x<= 9) ? "-]>>>>>" : ((x >= 10 && x<= 12) ? "-]>>>>>>" : ((x >= 13 && x<= 15) ? "-]>>>>>>>" : ((x >= 16 && x<18) ? "-]>>>>>>>>" : ((x >= 18) ? "-]>>>>>>>>>" : "ERROR! PLS REPORT!")))))))

extern const struct title_type titles[MAX_CLASS][ABS_MAX_LVL];
extern const char  *RaceName[];
extern const int RacialMax[MAX_RACE+1][MAX_CLASS];
extern char motd[MAX_STRING_LENGTH];
extern char wmotd[MAX_STRING_LENGTH];
extern struct char_data *character_list;
extern struct player_index_element *player_table;
extern int top_of_p_table;
extern struct index_data *mob_index;
extern struct index_data *obj_index;
extern char *pc_class_types[];
#if HASH
extern struct hash_header room_db;
#else
extern struct room_data *room_db;
#endif

extern long SystemFlags;         /* used for making teleport/astral/stuff not work */
extern long TempDis;

unsigned char echo_on[]  = {IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0'};
unsigned char echo_off[] = {IAC, WILL, TELOPT_ECHO, '\0'};
int Silence = 0;
int plr_tick_count=0;
int MAX_NAME_LENGTH = 11;/*Max length of a characters name*/


/* should be moved to protos.h at in a future release... */
void do_wclean(struct char_data *ch, char *argument, int cmd);
void do_setobjmax(struct char_data *ch, char *argument, int cmd);
void do_seeobjmax(struct char_data *ch, char *argument, int cmd);
void do_spend(struct char_data *ch, char *argument, int cmd);
void bugmail(struct char_data *ch, char *argument, int cmd);
void do_see_points(struct char_data *ch, char *argument, int cmd);
void do_reward(struct char_data *ch, char *argument, int cmd);
void do_punish(struct char_data *ch, char *argument, int cmd);
void do_set_spy(struct char_data *ch, char *argument, int cmd);

void do_wizset(struct char_data *ch, char *argument, int cmd);
void do_home(struct char_data *ch, char *argument, int cmd);
void do_lgos(struct char_data *ch, char *argument, int cmd);
int show_race_choice(struct descriptor_data *d);
void do_glance(struct char_data *ch, char *argument, int cmd);
void do_startarena(struct char_data *ch, char *argument, int cmd);
void do_arena(struct char_data *ch, char *argument, int cmd);
void do_whoarena(struct char_data *ch, char *argument, int cmd);

/* this is how we tell which race gets which class ! */
/* to add a new class seletion add the CLASS_NAME above the */
/* message 'NEW CLASS SELETIONS HERE'  */

	/* these are the allowable PC races */
const int race_choice[]= {
  RACE_DWARF,
  RACE_MOON_ELF,
  RACE_GOLD_ELF,
  RACE_WILD_ELF,
  RACE_AVARIEL,
  RACE_HUMAN,
  RACE_ROCK_GNOME,
  RACE_FOREST_GNOME,
  RACE_HALFLING,
  RACE_HALF_ELF,
  RACE_HALF_ORC,
  RACE_HALF_OGRE,
  RACE_HALF_GIANT,
  RACE_DROW,            /* bad guys here */
/*
  RACE_ORC,
  RACE_GOBLIN,
  RACE_TROLL,
  RACE_DARK_DWARF,
*/
  -1
  };

/* WARNING: do not remove the 0 at the end of the const, will cause your */
/*          mud to die hard! */

const int avariel_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_CLERIC+CLASS_WARRIOR,
	CLASS_MAGIC_USER+CLASS_WARRIOR,
	CLASS_MAGIC_USER+CLASS_CLERIC,
	CLASS_MAGIC_USER+CLASS_CLERIC+CLASS_WARRIOR,
0
};

const int moon_elf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	//CLASS_BARD,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */
	0
};

const int gold_elf_class_choice[]={
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */

0
};
const int sea_elf_class_choice[]={
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */

0
};
const int wild_elf_class_choice[]={
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PALADIN,
	CLASS_RANGER,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
   /* NEW CLASS SELECTIONS HERE */

0
};

const int dwarf_class_choice[]= {
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_PALADIN,
CLASS_RANGER,
CLASS_WARRIOR+CLASS_THIEF,
CLASS_WARRIOR+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int halfling_class_choice[]= {
CLASS_MAGIC_USER,
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_THIEF+CLASS_WARRIOR,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int rock_gnome_class_choice[]= {
CLASS_MAGIC_USER,
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_WARRIOR+CLASS_MAGIC_USER,
CLASS_MAGIC_USER+CLASS_THIEF,
CLASS_WARRIOR+CLASS_THIEF,
CLASS_MAGIC_USER+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int forest_gnome_class_choice[]= {
CLASS_MAGIC_USER,
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_RANGER,
CLASS_DRUID,
CLASS_RANGER+CLASS_DRUID,
CLASS_CLERIC+CLASS_THIEF,
CLASS_WARRIOR+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int human_class_choice[]= {
CLASS_MAGIC_USER,
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_DRUID,
CLASS_MONK,
CLASS_BARBARIAN,
CLASS_PALADIN,
CLASS_RANGER,
CLASS_PSI,
CLASS_WARRIOR+CLASS_DRUID,
CLASS_WARRIOR+CLASS_THIEF,
CLASS_WARRIOR+CLASS_MAGIC_USER,
CLASS_WARRIOR+CLASS_CLERIC,
CLASS_WARRIOR+CLASS_CLERIC+CLASS_MAGIC_USER,
CLASS_THIEF+CLASS_CLERIC+CLASS_MAGIC_USER,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int half_elf_class_choice[]= {
CLASS_MAGIC_USER,
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_DRUID,
CLASS_MONK,
CLASS_PALADIN,
CLASS_RANGER,
CLASS_PSI,
CLASS_WARRIOR+CLASS_MAGIC_USER,
CLASS_WARRIOR+CLASS_THIEF,
CLASS_MAGIC_USER+CLASS_THIEF,
CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_THIEF,
CLASS_CLERIC+CLASS_MAGIC_USER,
CLASS_WARRIOR+CLASS_MAGIC_USER+CLASS_CLERIC,
CLASS_WARRIOR+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int half_orc_class_choice[]= {
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_CLERIC+CLASS_THIEF,
CLASS_WARRIOR+CLASS_CLERIC+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int half_ogre_class_choice[]= {
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_CLERIC+CLASS_WARRIOR,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int half_giant_class_choice[]= {
CLASS_WARRIOR,
CLASS_BARBARIAN,
	/* NEW CLASS SELECTIONS HERE */
0
};


const int orc_class_choice[]= {
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_CLERIC,
CLASS_BARBARIAN,
CLASS_THIEF+CLASS_CLERIC,
/* new class below here */

0
};

const int goblin_class_choice[]= {
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_CLERIC,
CLASS_BARBARIAN,
CLASS_WARRIOR+CLASS_THIEF,
/* new class below here */

0
};

const int troll_class_choice[]= {
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_CLERIC,
CLASS_BARBARIAN,
CLASS_WARRIOR+CLASS_CLERIC,
/* new clases below here */

0
};

const int default_class_choice[]= {
 CLASS_WARRIOR,
 CLASS_THIEF,
 CLASS_CLERIC,
 CLASS_MAGIC_USER,
 CLASS_BARBARIAN,
 CLASS_PALADIN,
 CLASS_RANGER,
 CLASS_PSI,
 //CLASS_BARD,
/* new clases below here */

0
};

const int dark_dwarf_class_choice[]= {
CLASS_CLERIC,
CLASS_WARRIOR,
CLASS_THIEF,
CLASS_WARRIOR+CLASS_THIEF,
CLASS_WARRIOR+CLASS_CLERIC,
	/* NEW CLASS SELECTIONS HERE */
0
};

const int dark_elf_class_choice[]= {
	CLASS_MAGIC_USER,
	CLASS_CLERIC,
	CLASS_WARRIOR,
	CLASS_THIEF,
	CLASS_DRUID,
	CLASS_MONK,
	CLASS_PSI,
	CLASS_WARRIOR+CLASS_MAGIC_USER,
	CLASS_WARRIOR+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_THIEF,
	CLASS_MAGIC_USER+CLASS_WARRIOR+CLASS_THIEF,
	/* NEW CLASS SELECTIONS HERE */
	CLASS_CLERIC+CLASS_MAGIC_USER,
	0
};

void do_cset(struct char_data *ch, char *arg, int cmd);
void do_auth(struct char_data *ch, char *arg, int cmd); /* jdb 3-1 */


char *fill[]=
{ "in",
    "from",
    "with",
    "the",
    "on",
    "at",
    "to",
    "\n"
    };




int search_block(char *arg, char **list, bool exact)
{
  register int i,l;

  /* Make into lower case, and get length of string */
  for(l=0; *(arg+l); l++)
    *(arg+l)=LOWER(*(arg+l));

  if (exact) {
    for(i=0; **(list+i) != '\n'; i++)
      if (!strcmp(arg, *(list+i)))
	return(i);
  } else {
    if (!l)
      l=1; /* Avoid "" to match the first available string */
    for(i=0; **(list+i) != '\n'; i++)
      if (!strncmp(arg, *(list+i), l))
	return(i);
  }

  return(-1);
}


int old_search_block(char *argument,int begin,int length,char **list,int mode)
{
  int guess, found, search;


  /* If the word contain 0 letters, then a match is already found */
  found = (length < 1);

  guess = 0;

  /* Search for a match */

  if(mode)
    while ( NOT found AND *(list[guess]) != '\n' )
      {
	found=(length==strlen(list[guess]));
	for(search=0;( search < length AND found );search++)
	found=(*(argument+begin+search)== *(list[guess]+search));
	guess++;
      } else {
	while ( NOT found AND *(list[guess]) != '\n' ) {
	  found=1;
	  for(search=0;( search < length AND found );search++)
	    found=(*(argument+begin+search)== *(list[guess]+search));
	  guess++;
	}
      }

  return ( found ? guess : -1 );
}

void command_interpreter(struct char_data *ch, char *argument)
{

  char buf[200];
  extern int no_specials;
  NODE *n;
  char buf1[255], buf2[255];

if (HasClass(ch,TempDis) && GetMaxLevel(ch) < 58 && IS_PC(ch)) {
	send_to_char("Sorry, we are tracking down a bug and this class or one of your classes is disabled.\n\r",ch);
	return;
}

  REMOVE_BIT(ch->specials.affected_by, AFF_HIDE);
  
  /* Removed by Greg Hovey  (FALSE) */
  if(IS_AFFECTED2(ch,AFF2_AFK) && FALSE) {
    act("$c0006$n has returned to $s keyboard", TRUE, ch, 0, 0, TO_ROOM);
    act("$c0006You return to the keyboard.", TRUE, ch, 0, 0, TO_CHAR);
    REMOVE_BIT(ch->specials.affected_by2, AFF2_AFK);
    if (ch->pc)
      REMOVE_BIT(ch->pc->comm,COMM_AFK);
  }

  if (MOUNTED(ch)) {
    if (ch->in_room != MOUNTED(ch)->in_room)
      Dismount(ch, MOUNTED(ch), POSITION_STANDING);
  }

  /*
   *  a bug check.
   */
  if (!IS_NPC(ch)) {
    int i, found=FALSE;
    if ((!ch->player.name[0]) || (ch->player.name[0]<' ')) {
	log("Error in character name.  Changed to 'Error'");
if (ch->player.name)
	free(ch->player.name);
	ch->player.name = (char *)malloc(6);
	strcpy(ch->player.name, "Error");
	return;
    }
    strcpy(buf, ch->player.name);
    for (i = 0; i< strlen(buf) && !found; i++) {
      if (buf[i]<65) {
	found = TRUE;
      }
    }
    if (found) {
	log("Error in character name.  Changed to 'Error'");
if (ch->player.name)
	free(ch->player.name);
	ch->player.name = (char *)malloc(6);
	strcpy(ch->player.name, "Error");
	return;
    }
  }
 if(!*argument || *argument == '\n') {
    return;
 }
 else if(!isalpha(*argument)) {
   buf1[0] = *argument;
   buf1[1] = '\0';
   if((argument + 1))
     strcpy(buf2, (argument + 1));
   else
     buf2[0] = '\0';
 }
 else {
   register int i;
   half_chop(argument, buf1, buf2);
   i = 0;
   while(buf1[i] != '\0') {
       buf1[i] = LOWER(buf1[i]);
       i++;
     }
 }

/* New parser by DM */
 if(*buf1)
   n = FindValidCommand(buf1);
 else n = NULL;
/*
  cmd = old_search_block(argument,begin,look_at,command,0);
*/
  if (!n) {
    send_to_char("Pardon?\n\r", ch);
    return;
  }

  if ( GetMaxLevel(ch)<n->min_level )   {
    send_to_char("Pardon?\n\r", ch);
    return;
  }

  if ((n->func != 0))
  {
    if ((!IS_AFFECTED(ch, AFF_PARALYSIS)) || (n->min_pos <= POSITION_STUNNED))
    {
      if( GET_POS(ch) < n->min_pos )
      {
	switch(GET_POS(ch))
	  {
	  case POSITION_DEAD:
	    send_to_char("Lie still; you are DEAD!!! :-( \n\r", ch);
	    break;
	  case POSITION_INCAP:
	  case POSITION_MORTALLYW:
	    send_to_char(
		"You are in a pretty bad shape, unable to do anything!\n\r",
			 ch);
	    break;

	  case POSITION_STUNNED:
	    send_to_char(
	   "All you can do right now, is think about the stars!\n\r", ch);
	    break;
	  case POSITION_SLEEPING:
	    send_to_char("In your dreams, or what?\n\r", ch);
	    break;
	  case POSITION_RESTING:
	    send_to_char("Nah... You feel too relaxed to do that..\n\r",
			 ch);
	    break;
	  case POSITION_SITTING:
	    send_to_char("Maybe you should get on your feet first?\n\r",ch);
	    break;
	  case POSITION_FIGHTING:
	    send_to_char("No way! You are fighting for your life!\n\r", ch);
	    break;
	  case POSITION_STANDING:
	    send_to_char("Fraid you can't do that\n\r", ch);
	    break;
	  } /* switch */
      } else
      {

 /* They can't move, must have pissed off an immo!         */
 /* make sure polies can move, some mobs have this bit set */

 if (IS_SET(ch->specials.act, PLR_FREEZE) ) {
   if (IS_SET(ch->specials.act, ACT_POLYSELF) || IS_PC(ch)) {
     send_to_char("You have been frozen in your steps, you cannot do a thing!\n\r",ch);
     return;
    }
   }

	if (!no_specials && special(ch, n->number, buf2)) {
	  return;
	  }

	  if(n->log) {
	    sprintf(buf,"%s:%s",ch->player.name, argument);
	    slog(buf);
	  }


		/* so you can log mobs if ya need to */
#if LOG_MOB
	   if (!IS_PC(ch) && !IS_SET(ch->specials.act,ACT_POLYSELF)) {
	      sprintf(buf,"[%d] <%s>:%s", ch->in_room,ch->player.name, argument);
	      slog(buf);
	    }
#endif

#if 1           /* disabled for now UNCOMMENT SOON */
		/* to log all pc's */
	if (IS_SET(SystemFlags,SYS_LOGALL)) {
	   if (IS_PC(ch) || IS_SET(ch->specials.act,ACT_POLYSELF)) {
	      sprintf(buf,"[%d] %s:%s", ch->in_room,ch->player.name, argument);
	      slog(buf);
	    }
	  } else        /* user flagged as log person */
	   if(IS_AFFECTED2(ch, AFF2_LOG_ME)) {
	      sprintf(buf,"[%d] %s:%s", ch->in_room,ch->player.name, argument);
	     slog(buf);
	   } else               /* we log ALL immortals */
	  if ((GetMaxLevel(ch)>=LOW_IMMORTAL)&&(GetMaxLevel(ch)<60)) {
	      sprintf(buf,"[%d] %s:%s", ch->in_room,ch->player.name, argument);
	    slog(buf);
	  }

	 /* end logging stuff */
#endif

	  if (GET_GOLD(ch) > 2000000) {
	    sprintf(buf,"%s:%s",fname(ch->player.name),argument);
	    slog(buf);
	  }

	((*n->func)
	 (ch, buf2, n->number));
      }
      return;
    } else {
      send_to_char(" You are paralyzed, you can't do much!\n\r",ch);
      return;
    }
  }
  if (n && (n->func == 0))
send_to_char("Sorry, but that command has yet to be implemented...\n\r",ch);
  else
send_to_char("Pardon? \n\r", ch);
}

void argument_interpreter(char *argument,char *first_arg,char *second_arg )
{
  int look_at, begin;

  begin = 0;

  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);

      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(first_arg + look_at)='\0';
      begin += look_at;

    }
  while( fill_word(first_arg));

  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);

      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to second_arg */
	*(second_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(second_arg + look_at)='\0';
      begin += look_at;

    }
  while( fill_word(second_arg));
}

/*Return three arguments from initial string*/
void three_arg(char *argument,char *first_arg, char *second_arg,
               char  *third_arg )
{
  int look_at, begin;

  begin = 0;

  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);

      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(first_arg + look_at)='\0';
      begin += look_at;

    }
  while( fill_word(first_arg));

  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);

      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to second_arg */
	*(second_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(second_arg + look_at)='\0';
      begin += look_at;

    }
  while( fill_word(second_arg));
  do
    {
      /* Find first non blank */
      for ( ;*(argument + begin ) == ' ' ; begin++);

      /* Find length of first word */
      for ( look_at=0; *(argument+begin+look_at)> ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to third_arg */
	*(third_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(third_arg + look_at)='\0';
      begin += look_at;

    }
  while( fill_word(third_arg));
}

int is_number(char *str)
{
/*   int look_at; */

  if(*str=='\0')
    return(0);
  else if (newstrlen(str) > 8)
    return(0);
  else if((atoi(str)==0) && (str[0] != '0'))
    return(0);
  else return(1);

/*  for(look_at=0;*(str+look_at) != '\0';look_at++)
    if((*(str+look_at)<'0')||(*(str+look_at)>'9'))
      return(0);
  return(1); */
}

/*  Quinn substituted a new one-arg for the old one.. I thought returning a
    char pointer would be neat, and avoiding the func-calls would save a
    little time... If anyone feels pissed, I'm sorry.. Anyhow, the code is
    snatched from the old one, so it outta work..

    void one_argument(char *argument,char *first_arg )
    {
    static char dummy[MAX_STRING_LENGTH];

    argument_interpreter(argument,first_arg,dummy);
    }

    */


/* find the first sub-argument of a string, return pointer to first char in
   primary argument, following the sub-arg                                  */
char *one_argument(char *argument, char *first_arg )
{
  int begin,look_at;

  begin = 0;

  do    {
      /* Find first non blank */
      for ( ;isspace(*(argument + begin)); begin++);

      /* Find length of first word */
      for (look_at=0; *(argument+begin+look_at) > ' ' ; look_at++)

	/* Make all letters lower case,
	   AND copy them to first_arg */
	*(first_arg + look_at) =
	  LOWER(*(argument + begin + look_at));

      *(first_arg + look_at)='\0';
      begin += look_at;
    }
  while (fill_word(first_arg));

  return(argument+begin);
}



void only_argument(char *argument, char *dest)
{
  while (*argument && isspace(*argument))
    argument++;
  strcpy(dest, argument);
}




int fill_word(char *argument)
{
  return ( search_block(argument,fill,TRUE) >= 0);
}





/* determine if a given string is an abbreviation of another */
int is_abbrev(char *arg1, char *arg2)
{
  if (!*arg1)
    return(0);

  for (; *arg1; arg1++, arg2++)
    if (LOWER(*arg1) != LOWER(*arg2))
      return(0);

  return(1);
}




/* return first 'word' plus trailing substring of input string */
void half_chop(char *string, char *arg1, char *arg2)
{
  for (; isspace(*string); string++);

  for (; !isspace(*arg1 = *string) && *string; string++, arg1++);

  *arg1 = '\0';

  for (; isspace(*string); string++);

  for (; *arg2 = *string; string++, arg2++);
}



int special(struct char_data *ch, int cmd, char *arg)
{
  register struct obj_data *i;
  register struct char_data *k;
  int j;


  if (ch->in_room == NOWHERE) {
    char_to_room(ch, 3001);
    return;
  }

  /* special in room? */
  if (real_roomp(ch->in_room)->funct)
    if ((*real_roomp(ch->in_room)->funct)(ch, cmd, arg, real_roomp(ch->in_room), PULSE_COMMAND))
      return(1);

  /* special in equipment list? */
  for (j = 0; j <= (MAX_WEAR - 1); j++)
 if (ch->equipment[j] && ch->equipment[j]->item_number>=0)  {

  if (IS_SET(ch->equipment[j]->obj_flags.extra_flags,ITEM_ANTI_SUN))
    AntiSunItem(ch, cmd, arg, ch->equipment[j], PULSE_COMMAND);
	if (ch->equipment[j]) {
	  if (obj_index[ch->equipment[j]->item_number].func)
	  if ((*obj_index[ch->equipment[j]->item_number].func )
	      (ch, cmd, arg, ch->equipment[j], PULSE_COMMAND))
	   return(TRUE);
	  }              /* if ch->equipment[j] */
	}        /* item_number >=0 */


  /* special in inventory? */
  for (i = ch->carrying; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func)(ch, cmd, arg, i, PULSE_COMMAND))
	  return(1);

  /* special in mobile present? */
  for (k = real_roomp(ch->in_room)->people; k; k = k->next_in_room)
    if ( IS_MOB(k) )
      if (mob_index[k->nr].func)
	if ((*mob_index[k->nr].func)(ch, cmd, arg, k, PULSE_COMMAND))
	  return(1);

  /* special in object present? */
  for (i = real_roomp(ch->in_room)->contents; i; i = i->next_content)
    if (i->item_number>=0)
      if (obj_index[i->item_number].func)
	if ((*obj_index[i->item_number].func)(ch, cmd, arg, i, PULSE_COMMAND))
	  return(1);

  return(0);
}

void assign_command_pointers ()
{
  InitRadix();
  AddCommand("north", do_move, 1, POSITION_STANDING, 0);
  AddCommand("east", do_move, 2, POSITION_STANDING, 0);
  AddCommand("south", do_move, 3, POSITION_STANDING, 0);
  AddCommand("west", do_move, 4, POSITION_STANDING, 0);
  AddCommand("up", do_move, 5, POSITION_STANDING, 0);
  AddCommand("down", do_move, 6, POSITION_STANDING, 0);
  AddCommand("enter",do_enter,7,POSITION_STANDING,0);
  AddCommand("exits",do_exits,8,POSITION_RESTING,0);
  AddCommand("kiss",do_action,9,POSITION_RESTING,1);
  AddCommand("get",do_get,10,POSITION_RESTING,1);
  AddCommand("drink",do_drink,11,POSITION_RESTING,1);
  AddCommand("eat",do_eat,12,POSITION_RESTING,1);
  AddCommand("wear",do_wear,13,POSITION_RESTING,0);
  AddCommand("wield",do_wield,14,POSITION_RESTING,1);
  AddCommand("look",do_look,15,POSITION_RESTING,0);
  AddCommand("score",do_score,16,POSITION_SLEEPING,0);
#if 1
  AddCommand("say",do_new_say,17,POSITION_RESTING,0);
#else
  AddCommand("say",do_say,17,POSITION_RESTING,0);
#endif
  AddCommand("shout",do_shout,18,POSITION_RESTING,2);
  AddCommand("tell",do_tell,19,POSITION_RESTING,0);
  AddCommand("inventory",do_inventory,20,POSITION_DEAD,0);
  AddCommand("qui",do_qui,21,POSITION_DEAD,0);
  AddCommand("bounce",do_action,22,POSITION_STANDING,0);
  AddCommand("smile",do_action,23,POSITION_RESTING,0);
  AddCommand("dance",do_action,24,POSITION_STANDING,0);
  AddCommand("kill",do_kill,25,POSITION_FIGHTING,1);
  AddCommand("cackle",do_action,26,POSITION_RESTING,0);
  AddCommand("laugh",do_action,27,POSITION_RESTING,0);
  AddCommand("giggle",do_action,28,POSITION_RESTING,0);
  AddCommand("shake",do_action,29,POSITION_RESTING,0);
  AddCommand("puke",do_action,30,POSITION_RESTING,0);
  AddCommand("growl",do_action,31,POSITION_RESTING,0);
  AddCommand("scream",do_action,32,POSITION_RESTING,0);
  AddCommand("insult",do_insult,33,POSITION_RESTING,0);
  AddCommand("comfort",do_action,34,POSITION_RESTING,0);
  AddCommand("nod",do_action,35,POSITION_RESTING,0);
  AddCommand("sigh",do_action,36,POSITION_RESTING,0);
  AddCommand("sulk",do_action,37,POSITION_RESTING,0);
  AddCommand("help",do_help,38,POSITION_DEAD,0);
  AddCommand("who",do_who,39,POSITION_DEAD,0);
  AddCommand("emote",do_emote,40,POSITION_SLEEPING,0);
  AddCommand("echo",do_echo,41,POSITION_SLEEPING,1);
  AddCommand("stand",do_stand,42,POSITION_RESTING,0);
  AddCommand("sit",do_sit,43,POSITION_RESTING,0);
  AddCommand("rest",do_rest,44,POSITION_RESTING,0);
  AddCommand("sleep",do_sleep,45,POSITION_SLEEPING,0);
  AddCommand("wake",do_wake,46,POSITION_SLEEPING,0);
  AddCommand("force",do_force,47,POSITION_SLEEPING,57);
  AddCommand("transfer",do_trans,48,POSITION_SLEEPING,54);
  AddCommand("hug",do_action,49,POSITION_RESTING,0);
  AddCommand("snuggle",do_action,50,POSITION_RESTING,0);
  AddCommand("cuddle",do_action,51,POSITION_RESTING,0);
  AddCommand("nuzzle",do_action,52,POSITION_RESTING,0);
  AddCommand("cry",do_action,53,POSITION_RESTING,0);
  AddCommand("news",do_news,54,POSITION_SLEEPING,0);
  AddCommand("equipment",do_equipment,55,POSITION_SLEEPING,0);
  AddCommand("buy",do_not_here,56,POSITION_STANDING,0);
  AddCommand("sell",do_not_here,57,POSITION_STANDING,0);
  AddCommand("value",do_value,58,POSITION_RESTING,0);
  AddCommand("list",do_not_here,59,POSITION_STANDING,0);
  AddCommand("drop",do_drop,60,POSITION_RESTING, 1);
  AddCommand("goto",do_goto,61,POSITION_SLEEPING,0);
  AddCommand("weather",do_weather,62,POSITION_RESTING,0);
  AddCommand("read",do_read,63,POSITION_RESTING,0);
  AddCommand("pour",do_pour,64,POSITION_STANDING,0);
  AddCommand("grab",do_grab,65,POSITION_RESTING,0);
  AddCommand("remove",do_remove,66,POSITION_RESTING,0);
  AddCommand("put",do_put,67,POSITION_RESTING,0);
  AddCommand("shutdow",do_shutdow,68,POSITION_DEAD,SILLYLORD);
  AddCommand("save",do_save,69,POSITION_STUNNED,0);
  AddCommand("hit",do_hit,70,POSITION_FIGHTING,1);
  AddCommand("string",do_string,71,POSITION_SLEEPING,53);
  AddCommand("give",do_give,72,POSITION_RESTING,1);
  AddCommand("quit",do_quit,73,POSITION_DEAD,0);
  AddCommand("stat",do_stat,74,POSITION_DEAD,53);
  AddCommand("guard",do_guard,75,POSITION_STANDING,1);
  AddCommand("time",do_time,76,POSITION_DEAD,0);
  AddCommand("load",do_load,77,POSITION_DEAD,SAINT);
  AddCommand("purge",do_purge,78,POSITION_DEAD,53);
  AddCommand("shutdown",do_shutdown,79,POSITION_DEAD,SILLYLORD);
  AddCommand("idea",do_idea,80,POSITION_DEAD,0);
  AddCommand("typo",do_typo,81,POSITION_DEAD,0);
  AddCommand("bug",do_bug,82,POSITION_DEAD,0);
  AddCommand("whisper",do_whisper,83,POSITION_RESTING,0);
  AddCommand("cast",do_cast,84,POSITION_SITTING,1);
  AddCommand("at",do_at,85,POSITION_DEAD,51);
  AddCommand("ask",do_ask,86,POSITION_RESTING,0);
  AddCommand("order",do_order,87,POSITION_RESTING,1);
  AddCommand("sip",do_sip, 88,POSITION_RESTING,0);
  AddCommand("taste",do_taste,89,POSITION_RESTING,0);
  AddCommand("snoop",do_snoop,90,POSITION_DEAD,55);
  AddCommand("follow",do_follow,91,POSITION_RESTING,0);
  AddCommand("rent",do_not_here,92,POSITION_STANDING,1);
  AddCommand("offer",do_not_here,93,POSITION_STANDING,1);
  AddCommand("poke",do_action,94,POSITION_RESTING,0);
  AddCommand("advance",do_advance,95,POSITION_DEAD,IMPLEMENTOR);
  AddCommand("accuse",do_action,96,POSITION_SITTING,0);
  AddCommand("grin",do_action,97,POSITION_RESTING,0);
  AddCommand("bow",do_action,98,POSITION_STANDING,0);
  AddCommand("open",do_open,99,POSITION_SITTING,0);
  AddCommand("close",do_close,100,POSITION_SITTING,0);
  AddCommand("lock",do_lock,101,POSITION_SITTING,0);
  AddCommand("unlock",do_unlock,102,POSITION_SITTING,0);
  AddCommand("leave",do_leave,103,POSITION_STANDING,0);
  AddCommand("applaud",do_action,104,POSITION_RESTING,0);
  AddCommand("blush",do_action,105,POSITION_RESTING,0);
  AddCommand("burp",do_action,106,POSITION_RESTING,0);
  AddCommand("chuckle",do_action,107,POSITION_RESTING,0);
  AddCommand("clap",do_action,108,POSITION_RESTING,0);
  AddCommand("cough",do_action,109,POSITION_RESTING,0);
  AddCommand("curtsey",do_action,110,POSITION_STANDING,0);
  AddCommand("fart",do_action,111,POSITION_RESTING,0);
  AddCommand("flip",do_action,112,POSITION_STANDING,0);
  AddCommand("fondle",do_action,113,POSITION_RESTING,0);
  AddCommand("frown",do_action,114,POSITION_RESTING,0);
  AddCommand("gasp",do_action,115,POSITION_RESTING,0);
  AddCommand("glare",do_action,116,POSITION_RESTING,0);
  AddCommand("groan",do_action,117,POSITION_RESTING,0);
  AddCommand("grope",do_action,118,POSITION_RESTING,0);
  AddCommand("hiccup",do_action,119,POSITION_RESTING,0);
  AddCommand("lick",do_action,120,POSITION_RESTING,0);
  AddCommand("love",do_action,121,POSITION_RESTING,0);
  AddCommand("moan",do_action,122,POSITION_RESTING,0);
  AddCommand("nibble",do_action,123,POSITION_RESTING,0);
  AddCommand("pout",do_action,124,POSITION_RESTING,0);
  AddCommand("purr",do_action,125,POSITION_RESTING,0);
  AddCommand("ruffle",do_action,126,POSITION_STANDING,0);
  AddCommand("shiver",do_action,127,POSITION_RESTING,0);
  AddCommand("shrug",do_action,128,POSITION_RESTING,0);
  AddCommand("sing",do_action,129,POSITION_RESTING,0);
  AddCommand("slap",do_action,130,POSITION_RESTING,0);
  AddCommand("smirk",do_action,131,POSITION_RESTING,0);
/*  AddCommand("snap",do_action,132,POSITION_RESTING,0);*/
  AddCommand("sneeze",do_action,133,POSITION_RESTING,0);
  AddCommand("snicker",do_action,134,POSITION_RESTING,0);
  AddCommand("sniff",do_action,135,POSITION_RESTING,0);
  AddCommand("snore",do_action,136,POSITION_SLEEPING,0);
  AddCommand("spit",do_action,137,POSITION_STANDING,0);
  AddCommand("squeeze",do_action,138,POSITION_RESTING,0);
  AddCommand("stare",do_action,139,POSITION_RESTING,0);
/*  AddCommand("strut",do_action,140,POSITION_STANDING,0);*/
  AddCommand("thank",do_action,141,POSITION_RESTING,0);
  AddCommand("twiddle",do_action,142,POSITION_RESTING,0);
  AddCommand("wave",do_action,143,POSITION_RESTING,0);
  AddCommand("whistle",do_action,144,POSITION_RESTING,0);
  AddCommand("wiggle",do_action,145,POSITION_STANDING,0);
  AddCommand("wink",do_action,146,POSITION_RESTING,0);
  AddCommand("yawn",do_action,147,POSITION_RESTING,0);
  AddCommand("snowball",do_action,148,POSITION_STANDING,51);
  AddCommand("write",do_write,149,POSITION_STANDING,1);
  AddCommand("hold",do_grab,150,POSITION_RESTING,1);
  AddCommand("flee",do_flee,151,POSITION_SITTING,1);
  AddCommand("sneak",do_sneak,152,POSITION_STANDING,1);
  AddCommand("hide",do_hide,153,POSITION_RESTING,1);
  AddCommand("backstab",do_backstab,154,POSITION_STANDING,1);
  AddCommand("pick",do_pick,155,POSITION_STANDING,1);
  AddCommand("steal",do_steal,156,POSITION_STANDING,1);
  AddCommand("bash",do_bash,157,POSITION_FIGHTING,1);
  AddCommand("rescue",do_rescue,158,POSITION_FIGHTING,1);
  AddCommand("kick",do_kick,159,POSITION_FIGHTING,1);
  AddCommand("french",do_action,160,POSITION_RESTING,0);
  AddCommand("comb",do_action,161,POSITION_RESTING,0);
  AddCommand("massage",do_action,162,POSITION_RESTING,0);
  AddCommand("tickle",do_action,163,POSITION_RESTING,0);
  AddCommand("practice",do_practice,164,POSITION_RESTING,1);
  AddCommand("pat",do_action,165,POSITION_RESTING,0);
  AddCommand("examine",do_examine,166,POSITION_SITTING,0);
  AddCommand("take",do_get,167,POSITION_RESTING,1); /* TAKE */
  AddCommand("info",do_info,168,POSITION_SLEEPING,0);
#if 1
  AddCommand("'",do_new_say,169,POSITION_RESTING,0);
#else
  AddCommand("'",do_say,169,POSITION_RESTING,0);
#endif
  AddCommand("practise",do_practice,170,POSITION_RESTING,1);
  AddCommand("curse",do_action,171,POSITION_RESTING,0);
  AddCommand("use",do_use,172,POSITION_SITTING,1);
  AddCommand("where",do_where,173,POSITION_DEAD,1);
  AddCommand("levels",do_levels,174,POSITION_DEAD,0);
  /* AddCommand("reroll",do_reroll,175,POSITION_DEAD,SILLYLORD); */

  AddCommand("pray",do_pray,176,POSITION_SITTING,1);

  AddCommand(",",do_emote,177,POSITION_SLEEPING,0);
  AddCommand("beg",do_action,178,POSITION_RESTING,0);
  /*  AddCommand("bleed",do_not_here,179,POSITION_RESTING,0); */
  AddCommand("cringe",do_action,180,POSITION_RESTING,0);
  AddCommand("daydream",do_action,181,POSITION_SLEEPING,0);
  AddCommand("fume",do_action,182,POSITION_RESTING,0);
  AddCommand("grovel",do_action,183,POSITION_RESTING,0);
  AddCommand("hop",do_action,184,POSITION_RESTING,0);
  AddCommand("nudge",do_action,185,POSITION_RESTING,0);
  AddCommand("peer",do_action,186,POSITION_RESTING,0);
  AddCommand("point",do_action,187,POSITION_RESTING,0);
  AddCommand("ponder",do_action,188,POSITION_RESTING,0);
  AddCommand("punch",do_action,189,POSITION_RESTING,0);
  AddCommand("snarl",do_action,190,POSITION_RESTING,0);
  AddCommand("spank",do_action,191,POSITION_RESTING,0);
  AddCommand("steam",do_action,192,POSITION_RESTING,0);
  AddCommand("tackle",do_action,193,POSITION_RESTING,0);
  AddCommand("taunt",do_action,194,POSITION_RESTING,0);
  AddCommand("think",do_commune,195,POSITION_RESTING,LOW_IMMORTAL);
  AddCommand("whine",do_action,196,POSITION_RESTING,0);
  AddCommand("worship",do_action,197,POSITION_RESTING,0);
  AddCommand("yodel",do_action,198,POSITION_RESTING,0);
  AddCommand("brief",do_brief,199,POSITION_DEAD,0);
  AddCommand("wizlist",do_wizlist,200,POSITION_DEAD,0);
  AddCommand("consider",do_consider,201,POSITION_RESTING,0);
  AddCommand("group",do_group,202,POSITION_RESTING,1);
  AddCommand("restore",do_restore,203,POSITION_DEAD,52);
  AddCommand("return",do_return,204,POSITION_RESTING,0);
  AddCommand("switch",do_switch  ,205,POSITION_DEAD,55);
  AddCommand("quaff",do_quaff,206,POSITION_RESTING,0);
  AddCommand("recite",do_recite,207,POSITION_STANDING,0);
  AddCommand("users",do_users,208,POSITION_DEAD,LOW_IMMORTAL);
  AddCommand("pose",do_pose,209,POSITION_STANDING,0);
  AddCommand("noshout",do_noshout,210,POSITION_SLEEPING,54);
  AddCommand("wizhelp",do_wizhelp,211,POSITION_SLEEPING,LOW_IMMORTAL);
  AddCommand("credits",do_credits,212,POSITION_DEAD,0);
  AddCommand("compact",do_compact,213,POSITION_DEAD,0);
  AddCommand(":",do_emote,214,POSITION_SLEEPING,0);
  AddCommand("deafen",do_plr_noshout,215,POSITION_SLEEPING,1);
  AddCommand("slay",do_kill,216,POSITION_STANDING,57);
  AddCommand("wimpy",do_wimp,217,POSITION_DEAD,0);
  AddCommand("junk",do_junk,218,POSITION_RESTING,1);
  AddCommand("deposit",do_not_here,219,POSITION_RESTING,1);
  AddCommand("withdraw",do_not_here,220,POSITION_RESTING,1);
  AddCommand("balance",do_not_here,221,POSITION_RESTING,1);
  AddCommand("nohassle",do_nohassle,222,POSITION_DEAD,LOW_IMMORTAL);
  AddCommand("system",do_system,223,POSITION_DEAD,55);
  AddCommand("pull", do_open_exit,224,POSITION_STANDING,1);
  AddCommand("stealth",do_stealth,225,POSITION_DEAD,LOW_IMMORTAL);
  AddCommand("edit",do_edit,226,POSITION_DEAD,53);
  AddCommand("@",do_set,227,POSITION_DEAD,IMPLEMENTOR);
  AddCommand("rsave",do_rsave,228,POSITION_DEAD,53);
  AddCommand("rload",do_rload,229,POSITION_DEAD,53);
  AddCommand("track",do_track,230,POSITION_DEAD,1);
  AddCommand("siteban",do_wizlock,231,POSITION_DEAD,54);
  AddCommand("highfive",do_highfive,232,POSITION_DEAD,0);
  AddCommand("title",do_title,233,POSITION_DEAD,20);
  AddCommand("whozone",do_who,234,POSITION_DEAD,0);
  AddCommand("assist",do_assist,235,POSITION_FIGHTING,1);
  AddCommand("attribute",do_attribute,236,POSITION_DEAD,5);
  AddCommand("world",do_world,237,POSITION_DEAD,0);
  AddCommand("allspells",do_spells,238,POSITION_DEAD,0);
  AddCommand("breath",do_breath,239,POSITION_FIGHTING,1);
  AddCommand("show",do_show,240,POSITION_DEAD,52);
  AddCommand("debug",do_debug,241,POSITION_DEAD,60);
  AddCommand("invisible",do_invis,242,POSITION_DEAD,LOW_IMMORTAL);
  AddCommand("gain",do_gain,243,POSITION_DEAD,1);

/*  AddCommand("rrload",do_rrload,244,POSITION_DEAD,CREATOR); */

  AddCommand("disarm",do_disarm,245,POSITION_FIGHTING,1);
  AddCommand("bonk",do_action,246,POSITION_SITTING,1);
  AddCommand("chpwd",do_passwd,247,POSITION_SITTING,IMPLEMENTOR);
  AddCommand("fill",do_not_here,248,POSITION_SITTING,0);
  AddCommand("imptest",do_imptest,249,POSITION_SITTING,60);
  AddCommand("shoot",do_fire,250,POSITION_STANDING,  1);
  AddCommand("silence",do_silence,251,POSITION_STANDING, 54);

/*  AddCommand("teams",do_not_here,252,POSITION_STANDING, BIG_GUY);  */

/*  AddCommand("player",do_not_here,253,POSITION_STANDING, BIG_GUY); */
  AddCommand("create",do_create,254,POSITION_STANDING, 53);
  AddCommand("bamfin",do_bamfin,255,POSITION_STANDING, LOW_IMMORTAL);
  AddCommand("bamfout",do_bamfout,256,POSITION_STANDING, LOW_IMMORTAL);
  AddCommand("vis",do_invis,257,POSITION_RESTING,  0);
  AddCommand("doorbash",do_doorbash,258,POSITION_STANDING, 1);
  AddCommand("mosh",do_action,259,POSITION_FIGHTING, 1);

/* alias commands */
  AddCommand("alias",do_alias,260,POSITION_SLEEPING, 1);
  AddCommand("1", do_alias,261,POSITION_DEAD, 1);
  AddCommand("2", do_alias,262,POSITION_DEAD, 1);
  AddCommand("3", do_alias,263,POSITION_DEAD, 1);
  AddCommand("4", do_alias,264,POSITION_DEAD, 1);
  AddCommand("5", do_alias,265,POSITION_DEAD, 1);
  AddCommand("6", do_alias,266,POSITION_DEAD, 1);
  AddCommand("7", do_alias,267,POSITION_DEAD, 1);
  AddCommand("8", do_alias,268,POSITION_DEAD, 1);
  AddCommand("9", do_alias,269,POSITION_DEAD, 1);
  AddCommand("0", do_alias,270,POSITION_DEAD, 1);

  AddCommand("swim", do_swim,271,POSITION_STANDING, 1);
  AddCommand("spy", do_spy,272,POSITION_STANDING, 1);
  AddCommand("springleap",do_springleap,273,POSITION_RESTING, 1);
  AddCommand("quivering palm",do_quivering_palm,274,POSITION_FIGHTING, 30);
  AddCommand("feign death",do_feign_death,275,POSITION_FIGHTING, 1);
  AddCommand("mount",do_mount,276,POSITION_STANDING, 1);
  AddCommand("dismount",do_mount,277,POSITION_MOUNTED, 1);
  AddCommand("ride",do_mount,278,POSITION_STANDING, 1);
  AddCommand("sign",do_sign,279, POSITION_RESTING, 1);

/* had to put this here BEFORE setsev so it would get this and not setsev */
  AddCommand("set",do_set_flags,280,POSITION_DEAD,0);

  AddCommand("first aid",do_first_aid,281, POSITION_RESTING, 1);
  AddCommand("log",do_set_log,282, POSITION_DEAD, 60);
  AddCommand("recall",do_cast,283, POSITION_SITTING, 1);
  AddCommand("reload",reboot_text,284, POSITION_DEAD, BIG_GUY);
  AddCommand("event",do_event,285, POSITION_DEAD, 60);
  AddCommand("disguise",do_disguise,286, POSITION_STANDING, 1);
  AddCommand("climb",do_climb,287, POSITION_STANDING,1);
  AddCommand("beep",do_beep,288, POSITION_DEAD, 51);
  AddCommand("bite",do_action,289, POSITION_RESTING, 1);
  AddCommand("redit", do_redit, 290, POSITION_SLEEPING, 53);
  AddCommand("display", do_display, 291, POSITION_SLEEPING, 1);
  AddCommand("resize", do_resize, 292, POSITION_SLEEPING, 1);
  AddCommand("\"", do_commune, 293, POSITION_SLEEPING, LOW_IMMORTAL);
  AddCommand("#", do_cset, 294, POSITION_DEAD, 59);

  AddCommand("auth", do_auth, 299, POSITION_SLEEPING, LOW_IMMORTAL);

  AddCommand("nogossip",do_plr_nogossip,301,POSITION_RESTING,0);
  AddCommand("gossip",do_gossip,302,POSITION_RESTING,0);
  AddCommand("noauction",do_plr_noauction,303,POSITION_RESTING,0);
  AddCommand("auction",do_auction,304,POSITION_RESTING,0);
  AddCommand("discon",do_disconnect,305,POSITION_RESTING,LOW_IMMORTAL);
  AddCommand("freeze",do_freeze,306,POSITION_SLEEPING,55);
  AddCommand("drain",do_drainlevel,307,POSITION_SLEEPING,IMPLEMENTOR);
  AddCommand("oedit",do_oedit,308,POSITION_DEAD,53);
  AddCommand("report",do_report,309,POSITION_RESTING,1);
  AddCommand("interven",do_god_interven,310,POSITION_DEAD,58);
  AddCommand("gtell",do_gtell,311,POSITION_SLEEPING,1);

  AddCommand("raise",do_action,312,POSITION_RESTING,1);
  AddCommand("tap",do_action,313,POSITION_STANDING,1);
  AddCommand("liege",do_action,314,POSITION_RESTING,1);
  AddCommand("sneer",do_action,315,POSITION_RESTING,1);
  AddCommand("howl",do_action,316,POSITION_RESTING,1);
  AddCommand("kneel",do_action,317,POSITION_STANDING,1);
  AddCommand("finger",do_action,318,POSITION_RESTING,1);
  AddCommand("pace",do_action,319,POSITION_STANDING,1);
  AddCommand("tongue",do_action,320,POSITION_RESTING,1);
  AddCommand("flex",do_action,321,POSITION_STANDING,1);
  AddCommand("ack",do_action,322,POSITION_RESTING,1);
  AddCommand("eh",do_action,323,POSITION_RESTING,1);
  AddCommand("caress",do_action,324,POSITION_RESTING,1);
  AddCommand("cheer",do_action,325,POSITION_RESTING,1);
  AddCommand("jump",do_action,326,POSITION_STANDING,1);

/*  AddCommand("roll",do_action,327,POSITION_RESTING,1);  */

  AddCommand("split",do_split,328,POSITION_RESTING,1);
  AddCommand("berserk",do_berserk,329,POSITION_FIGHTING,1);

  AddCommand("tan",do_tan,330,POSITION_STANDING,0);

  AddCommand("memorize",do_memorize,331,POSITION_RESTING,1);

  AddCommand("find",do_find,332,POSITION_STANDING,1);
  AddCommand("bellow",do_bellow,333,POSITION_FIGHTING,1);
  AddCommand("camouflage",do_hide,334,POSITION_STANDING,1);
  AddCommand("carve",do_carve,335,POSITION_STANDING,1);
  AddCommand("nuke",do_nuke,336,POSITION_DEAD,IMPLEMENTOR);
  AddCommand("skills",do_show_skill,337,POSITION_SLEEPING,0);

 AddCommand("doorway",do_doorway,338,POSITION_STANDING,1);
 AddCommand("portal",do_psi_portal,339,POSITION_STANDING,1);
 AddCommand("summon",do_mindsummon,340,POSITION_STANDING,1);
 AddCommand("canibalize",do_canibalize,341,POSITION_STANDING,1);
 AddCommand("flame",do_flame_shroud,342,POSITION_STANDING,1);
 AddCommand("aura",do_aura_sight,343,POSITION_RESTING,1);
 AddCommand("great",do_great_sight,344,POSITION_RESTING,1);
 AddCommand("psionic invisibility",do_invisibililty,345,POSITION_STANDING,1);
 AddCommand("blast",do_blast,346,POSITION_FIGHTING,1);

/* AddCommand("psionic blast",do_blast,347,POSITION_FIGHTING,0);    */
 AddCommand("medit",do_medit,347,POSITION_DEAD,53);

 AddCommand("hypnotize",do_hypnosis,348,POSITION_STANDING,1);
 AddCommand("scry",do_scry,349,POSITION_RESTING,1);
 AddCommand("adrenalize",do_adrenalize,350,POSITION_STANDING,1);
 AddCommand("brew",do_brew,351,POSITION_STANDING,0);
 AddCommand("meditate",do_meditate,352,POSITION_RESTING,0);
 AddCommand("forcerent",do_force_rent,353,POSITION_DEAD,58);
 AddCommand("warcry",do_holy_warcry,354,POSITION_FIGHTING,0);
 AddCommand("lay on hands",do_lay_on_hands,355,POSITION_RESTING,0);
 AddCommand("blessing",do_blessing,356,POSITION_STANDING,0);
 AddCommand("heroic",do_heroic_rescue,357,POSITION_FIGHTING,0);
 AddCommand("scan",do_scan,358,POSITION_STANDING,IMMORTAL);
 AddCommand("shield",do_psi_shield,359,POSITION_STANDING,0);
 AddCommand("notell",do_plr_notell,360,POSITION_DEAD,0);
 AddCommand("commands",do_command_list,361,POSITION_DEAD,0);
 AddCommand("ghost",do_ghost,362,POSITION_DEAD,58);
 AddCommand("speak",do_speak,363,POSITION_DEAD,0);
 AddCommand("setsev",do_setsev,364, POSITION_DEAD, 51);
 AddCommand("esp",do_esp,365, POSITION_STANDING,0);
 AddCommand("mail",do_not_here,366, POSITION_STANDING,0);
 AddCommand("check",do_not_here,367, POSITION_STANDING,0);
 AddCommand("receive",do_not_here,368, POSITION_STANDING,0);
 AddCommand("telepathy",do_telepathy,369, POSITION_RESTING,0);
 AddCommand("mind",do_cast,370, POSITION_SITTING,0);
 AddCommand("twist", do_open_exit,371, POSITION_STANDING,0);
 AddCommand("turn" , do_open_exit,372, POSITION_STANDING,0);
 AddCommand("lift" , do_open_exit,373, POSITION_STANDING,0);
 AddCommand("push" , do_open_exit,374, POSITION_STANDING,0);
 AddCommand("zload" , do_zload,375, POSITION_STANDING,53);
 AddCommand("zsave" , do_zsave,376, POSITION_STANDING,53);
 AddCommand("zclean" ,do_zclean ,377, POSITION_STANDING,53);
 AddCommand("wrebuild" ,do_WorldSave ,378, POSITION_STANDING,60);
 AddCommand("gwho" ,list_groups ,379, POSITION_DEAD,0);

AddCommand("mforce" ,do_mforce ,380, POSITION_DEAD,53 ); /* CREATOR */
AddCommand("clone" ,do_clone ,381, POSITION_DEAD,53 );

AddCommand("fire" ,do_fire ,382, POSITION_DEAD,0);
AddCommand("throw" ,do_throw ,383, POSITION_SITTING,0);
AddCommand("run" ,do_run ,384, POSITION_STANDING,0);
AddCommand("notch" ,do_weapon_load ,385, POSITION_RESTING,0);
AddCommand("load" ,do_weapon_load ,386, POSITION_RESTING,0);

AddCommand("spot" ,do_scan ,387, POSITION_STANDING,0);
AddCommand("view" ,do_viewfile ,388, POSITION_DEAD,51);
AddCommand("afk" ,do_set_afk,389, POSITION_DEAD,1);

/* lots of Socials */
 AddCommand("adore",do_action,400,POSITION_RESTING,0);
 AddCommand("agree",do_action,401,POSITION_RESTING,0);
 AddCommand("bleed",do_action,402,POSITION_RESTING,0);
 AddCommand("blink",do_action,403,POSITION_RESTING,0);
 AddCommand("blow",do_action,404,POSITION_RESTING,0);
 AddCommand("blame",do_action,405,POSITION_RESTING,0);
 AddCommand("bark",do_action,406,POSITION_RESTING,0);
 AddCommand("bhug",do_action,407,POSITION_RESTING,0);
 AddCommand("bcheck",do_action,408,POSITION_RESTING,0);
 AddCommand("boast",do_action,409,POSITION_RESTING,0);
 AddCommand("chide",do_action,410,POSITION_RESTING,0);
 AddCommand("compliment",do_action,411,POSITION_RESTING,0);
AddCommand("ceyes",do_action,412,POSITION_RESTING,0);
AddCommand("cears",do_action,413,POSITION_RESTING,0);
AddCommand("cross",do_action,414,POSITION_RESTING,0);
AddCommand("console",do_action,415,POSITION_RESTING,0);
AddCommand("calm",do_action,416,POSITION_RESTING,0);
AddCommand("cower",do_action,417,POSITION_RESTING,0);
AddCommand("confess",do_action,418,POSITION_RESTING,0);
AddCommand("drool",do_action,419,POSITION_RESTING,0);
AddCommand("grit",do_action,420,POSITION_RESTING,0);
AddCommand("greet",do_action,421,POSITION_RESTING,0);
AddCommand("gulp",do_action,422,POSITION_RESTING,0);
AddCommand("gloat",do_action,423,POSITION_RESTING,0);
AddCommand("gaze",do_action,424,POSITION_RESTING,0);
AddCommand("hum",do_action,425,POSITION_RESTING,0);
AddCommand("hkiss",do_action,426,POSITION_RESTING,0);
AddCommand("ignore",do_action,427,POSITION_RESTING,0);
AddCommand("interrupt",do_action,428,POSITION_RESTING,0);
AddCommand("knock",do_action,429,POSITION_RESTING,0);
AddCommand("listen",do_action,430,POSITION_RESTING,0);
AddCommand("muse",do_action,431,POSITION_RESTING,0);
AddCommand("pinch",do_action,432,POSITION_RESTING,0);
AddCommand("praise",do_action,433,POSITION_RESTING,0);
AddCommand("plot",do_action,434,POSITION_RESTING,0);
AddCommand("pie",do_action,435,POSITION_RESTING,0);
AddCommand("pleade",do_action,436,POSITION_RESTING,0);
AddCommand("pant",do_action,437,POSITION_RESTING,0);
AddCommand("rub",do_action,438,POSITION_RESTING,0);
AddCommand("roll",do_action,439,POSITION_RESTING,0);
AddCommand("recoil",do_action,440,POSITION_RESTING,0);
AddCommand("roar",do_action,441,POSITION_RESTING,0);
AddCommand("relax",do_action,442,POSITION_RESTING,0);
AddCommand("snap",do_action,443,POSITION_RESTING,0);
AddCommand("strut",do_action,444,POSITION_RESTING,0);
AddCommand("stroke",do_action,445,POSITION_RESTING,0);
AddCommand("stretch",do_action,446,POSITION_RESTING,0);
AddCommand("swave",do_action,447,POSITION_RESTING,0);
AddCommand("sob",do_action,448,POSITION_RESTING,0);
AddCommand("scratch",do_action,449,POSITION_RESTING,0);
AddCommand("squirm",do_action,450,POSITION_RESTING,0);
AddCommand("strangle",do_action,451,POSITION_RESTING,0);
AddCommand("scowl",do_action,452,POSITION_RESTING,0);
AddCommand("shudder",do_action,453,POSITION_RESTING,0);
AddCommand("strip",do_action,454,POSITION_RESTING,0);
AddCommand("scoff",do_action,455,POSITION_RESTING,0);
AddCommand("salute",do_action,456,POSITION_RESTING,0);
AddCommand("scold",do_action,457,POSITION_RESTING,0);
AddCommand("stagger",do_action,458,POSITION_RESTING,0);
AddCommand("toss",do_action,459,POSITION_RESTING,0);
AddCommand("twirl",do_action,460,POSITION_RESTING,0);
AddCommand("toast",do_action,461,POSITION_RESTING,0);
AddCommand("tug",do_action,462,POSITION_RESTING,0);
AddCommand("touch",do_action,463,POSITION_RESTING,0);
AddCommand("tremble",do_action,464,POSITION_RESTING,0);
AddCommand("twitch",do_action,465,POSITION_RESTING,0);
AddCommand("whimper",do_action,466,POSITION_RESTING,0);
AddCommand("whap",do_action,467,POSITION_RESTING,0);
AddCommand("wedge",do_action,468,POSITION_RESTING,0);
AddCommand("apologize",do_action,469,POSITION_RESTING,0);

#if USE_DIMD
AddCommand("dmanage",do_dmanage,480,POSITION_RESTING,IMPLEMENTOR);
AddCommand("drestrict",do_drestrict,481,POSITION_RESTING,55);
AddCommand("dlink",do_dlink,482,POSITION_RESTING,55);
AddCommand("dunlink",do_dunlink,483,POSITION_RESTING,55);
AddCommand("dlist",do_dlist,484,POSITION_RESTING,51);
AddCommand("dwho",do_dwho,485,POSITION_RESTING,51);
AddCommand("dgossip",do_dgossip,486,POSITION_RESTING,51);
AddCommand("dtell",do_dtell,487,POSITION_RESTING,51);
AddCommand("dthink",do_dthink,488,POSITION_RESTING,51);
#endif

AddCommand("sending",do_sending,489,POSITION_STANDING,1);
AddCommand("messenger",do_sending,490,POSITION_STANDING,1);
AddCommand("promote", do_promote,491,POSITION_RESTING,1);

AddCommand("ooedit",do_ooedit,492,POSITION_DEAD,53);
AddCommand("whois",do_finger,493,POSITION_DEAD,1);
AddCommand("osave",do_osave,494,POSITION_DEAD,53);
AddCommand("msave",do_msave,494,POSITION_DEAD,53);

AddCommand("?",do_help,495,POSITION_DEAD,0);
AddCommand("home",do_home,496,POSITION_DEAD,51);
AddCommand("wizset",do_wizset,497,POSITION_DEAD,51);
AddCommand("ooc",do_ooc,497,POSITION_RESTING,51);
AddCommand("noooc",do_plr_noooc,498,POSITION_SLEEPING,2);
AddCommand("wiznoooc",do_wiznoooc,499,POSITION_SLEEPING,54);

/* New social commands that Ugha added... - Manwe Windmaster 280697 */
AddCommand("homer",do_action,500,POSITION_RESTING,0);
AddCommand("grumble",do_action,501,POSITION_RESTING,0);
AddCommand("typoking",do_action,502,POSITION_RESTING,0);
AddCommand("evilgrin",do_action,503,POSITION_RESTING,0);
AddCommand("faint",do_action,504,POSITION_RESTING,0);
AddCommand("ckiss",do_action,505,POSITION_STANDING,0);
AddCommand("brb",do_action,506,POSITION_RESTING,0);
AddCommand("ready",do_action,507,POSITION_STANDING,0);
AddCommand("wolfwhistle",do_action,508,POSITION_RESTING,0);
AddCommand("wizreport",do_wizreport,509,POSITION_RESTING,55);
AddCommand("lgos",do_lgos,510,POSITION_RESTING,51);
AddCommand("groove",do_action, 511, POSITION_RESTING,1);
AddCommand("wdisplay", do_action, 512, POSITION_STANDING, 1);
AddCommand("jam", do_action, 513, POSITION_STANDING, 1);
AddCommand("donate", do_donate, 514, POSITION_RESTING,1);
AddCommand("reply", do_reply, 515, POSITION_RESTING,1);
AddCommand("set_spy", do_set_spy, 516, POSITION_DEAD,60);
AddCommand("reward", do_reward, 517, POSITION_RESTING,51);
AddCommand("punish", do_punish, 518, POSITION_RESTING,51);
AddCommand("spend", do_spend, 519, POSITION_RESTING,51);
AddCommand("seepoints", do_see_points, 520, POSITION_RESTING,51);
AddCommand("bugmail", bugmail, 521, POSITION_STANDING,1);
AddCommand("setobjmax", do_setobjmax, 522, POSITION_RESTING,53);
AddCommand("seeobjmax", do_seeobjmax, 523, POSITION_RESTING,53);
AddCommand("wclean", do_wclean, 524, POSITION_RESTING,58);
AddCommand("glance", do_glance, 525, POSITION_SITTING,1);
AddCommand("arena", do_arena, 525, POSITION_SITTING,1);
AddCommand("startarena", do_startarena, 525, POSITION_SITTING,55);
AddCommand("whoarena", do_whoarena, 525, POSITION_RESTING,0);
AddCommand("frolic",do_action,526,POSITION_STANDING,0);
AddCommand("land",do_land,527,POSITION_STANDING,0);
AddCommand("launch",do_launch,528,POSITION_STANDING,0);
 AddCommand("disengage",do_disengage,529,POSITION_FIGHTING,1);
 AddCommand("prompt", do_set_prompt,530,POSITION_RESTING,0);

 /*New stuff 2001 (GH)*/
AddCommand("talk",do_action,531,POSITION_RESTING,1);
AddCommand("disagree",do_action,532,POSITION_RESTING,0);
AddCommand("beckon",do_action,533,POSITION_RESTING,0);
AddCommand("pounce",do_action,534,POSITION_STANDING,0);
AddCommand("amaze",do_action,535,POSITION_RESTING,0);
AddCommand("tank",do_action,536,POSITION_STANDING,0);
AddCommand("hshake",do_action,537,POSITION_STANDING,0);
AddCommand("backhand",do_action,538,POSITION_STANDING,0);
AddCommand("surrender",do_action,539,POSITION_RESTING,0);
AddCommand("collapse",do_action,540,POSITION_STANDING,0);
AddCommand("wince",do_action,541,POSITION_RESTING,0);
AddCommand("tag",do_action,542,POSITION_RESTING,0);
AddCommand("trip",do_action,543,POSITION_RESTING,0);
AddCommand("grunt",do_action,544,POSITION_RESTING,0);
AddCommand("imitate",do_action,545,POSITION_RESTING,0);
AddCommand("hickey",do_action,546,POSITION_RESTING,0);
AddCommand("torture",do_action,547,POSITION_RESTING,0);
AddCommand("addict",do_action,548,POSITION_RESTING,0);
AddCommand("adjust",do_action,549,POSITION_RESTING,0);
AddCommand("anti",do_action,550,POSITION_RESTING,0);
AddCommand("bbl",do_action,551,POSITION_RESTING,0);
AddCommand("beam",do_action,552,POSITION_RESTING,0);
AddCommand("challenge",do_action,553,POSITION_RESTING,0);
AddCommand("mutter",do_action,554,POSITION_RESTING,0);
AddCommand("beat",do_action,555,POSITION_RESTING,0);
 AddCommand("moon",do_action,556,POSITION_RESTING,0);
 AddCommand("dream",do_action,557,POSITION_RESTING,0);
 AddCommand("shove",do_action, 558, POSITION_RESTING,0);
 AddCommand("behead",do_behead,559,POSITION_STANDING,0);

/*
  talk disagree beckon pounce amaze tank hshake backhand surrender collapses wince
  tag trip grunt imitate hickey torture addict adjust anti  bbl beam
  challenge mutter beat moon dream
*/
}


/* *************************************************************************
 *  Stuff for controlling the non-playing sockets (get name, pwd etc)       *
 ************************************************************************* */






/* locate entry in p_table with entry->name == name. -1 mrks failed search */
int find_name(char *name)
{
  int i;

  for (i = 0; i <= top_of_p_table; i++) {
    if (!str_cmp((player_table + i)->name, name))
      return(i);
  }

  return(-1);
}


int _parse_name(char *arg, char *name)
{
  int i;

  /* skip whitespaces */
  for (; isspace(*arg); arg++);
  for (i = 0; *name = *arg; arg++, i++, name++) {
    if ((*arg <0) || !isalpha(*arg) || i > MAX_NAME_LENGTH)
      return(1);

   }

  if (!i)
    return(1);

  return(0);
}

#define ASSHOLE_FNAME "asshole.list"

int _check_ass_name(char *name)
{
/* 0 - full match
   1 - from start of string
   2 - from end of string
   3 - somewhere in string
*/
  static struct shitlist {
	int how;
	char name[80];
	} *shitlist = NULL;
  FILE *f;
  char buf[512];
  int i,j,k;

if(strlen(name) > MAX_NAME_LENGTH)
   return 1;

  if(!shitlist) {
    if((f=fopen(ASSHOLE_FNAME,"rt"))==NULL) {
      log("can't open asshole names list");
      shitlist=(struct shitlist *)calloc(1,sizeof(struct shitlist));
      *shitlist[0].name=0;
      return 0;
    }
    for(i=0;fgets(buf,180,f)!=NULL;i++) ;
    shitlist=(struct shitlist *)calloc((i+3), sizeof(struct shitlist));
    rewind(f);
    for(i=0;fgets(buf,180,f)!=NULL;i++) {
      if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r')
	buf[strlen(buf)-1]=0;
      if(buf[strlen(buf)-1]=='\n' || buf[strlen(buf)-1]=='\r')
	buf[strlen(buf)-1]=0;
      if(*buf=='*') {
	if(buf[strlen(buf)-1]=='*') {
	  shitlist[i].how=3;
	  buf[strlen(buf)-1]=0;
	  strcpy(shitlist[i].name,buf+1);
	} else {
	  shitlist[i].how=2;
	  strcpy(shitlist[i].name,buf+1);
	}
      } else {
	if(buf[strlen(buf)-1]=='*') {
	  shitlist[i].how=1;
	  buf[strlen(buf)-1]=0;
	  strcpy(shitlist[i].name,buf);
	} else {
	  shitlist[i].how=0;
	  strcpy(shitlist[i].name,buf);
	}
      }
    }
    *shitlist[i].name = 0;
    for(i=0;*shitlist[i].name;i++) {
      sprintf(buf, "mode: %d, name: %s", shitlist[i].how, shitlist[i].name);
/*      log(buf);*/
    }
  }
#define NAME shitlist[j].name
  for(j=0;*NAME;j++)
    switch(shitlist[j].how) {
      case 0:
	if(!str_cmp(name,NAME))
	  return 1;
	break;
     case 1:
	if(!strn_cmp(name,NAME,strlen(NAME)))
	  return 1;
	break;
     case 2:
	if(strlen(name)<strlen(NAME)) break;
	if(!str_cmp(name+(strlen(name)-strlen(NAME)), NAME))
	  return 1;
	break;
     case 3:
	if(strlen(name)<strlen(NAME)) break;
	for(k=0;k<=strlen(name)-strlen(NAME);k++)
	  if(!strn_cmp(name+k, NAME, strlen(NAME)))
	    return 1;
	break;
     default:
	log("Grr! invalid value in shitlist, interpereter.c _parse_name");
	return 1;
   }
#undef NAME
  return(0);
}



/* deal with newcomers and other non-playing sockets */
void nanny(struct descriptor_data *d, char *arg)
{
  char buf[100];
  int player_i, count=0, oops=FALSE, index=0;
  char tmp_name[20];
  struct char_file_u tmp_store;
  struct char_data *tmp_ch;
  struct descriptor_data *k;
  extern struct descriptor_data *descriptor_list;
  extern long SystemFlags;
  extern int plr_tick_count;

  void do_look(struct char_data *ch, char *argument, int cmd);
  void load_char_objs(struct char_data *ch);
  int load_char(char *name, struct char_file_u *char_element);
  void show_class_selection(struct descriptor_data *d, int r);

  write(d->descriptor, echo_on, 6);

  switch (STATE(d))     {
    case CON_ANSI:

    for (; isspace(*arg); arg++);
    switch (*arg)
      {
      case 'y':
      case 'Y':
	/* Set ansi */
	SET_BIT(d->character->player.user_flags,USE_ANSI);

	send_to_char("$c0012A$c0010n$c0011s$c0014i$c0007 colors enabled.\n\r\n\r"
		     ,d->character);
	show_race_choice(d);
	SEND_TO_Q("For help type '?'- will list level limits. \n\r RACE:  ", d);
	STATE(d) = CON_QRACE;
	return;
	break;

      case 'n':
      case 'N':
	show_race_choice(d);
	SEND_TO_Q("For help type '?'- will list level limits. \n\r RACE:  ", d);
	STATE(d) = CON_QRACE;
	return;
	break;

      default:
	SEND_TO_Q("Please type Yes or No.\n\r", d);
	SEND_TO_Q("Would you like ansi colors? :", d);
	//STATE(d) = CON_ANSI;

	return;
	break;
  }

  case CON_QRACE:
    d->character->reroll=9;
    for (; isspace(*arg); arg++)  ;
    if (!*arg)
    {
	show_race_choice(d);
      SEND_TO_Q("For help, and level limits type '?'. \n\r RACE?:  ", d);
      STATE(d) = CON_QRACE;
    } else
    if (*arg == '?') {
	SEND_TO_Q(RACEHELP, d);
	show_race_choice(d);
	SEND_TO_Q("For help type '?' - will also list level limits. \n\r RACE?:  ", d);
	STATE(d) = CON_QRACE;
    } else {
      int i=0,tmpi=0;

	while (race_choice[i]!=-1) i++;
	tmpi=atoi(arg);
	if (tmpi>=0 && tmpi <=i-1)
	  {
	   /* set the chars race to this */
		GET_RACE(d->character) = race_choice[tmpi];
		SEND_TO_Q("What is your sex (M/F) ? ", d);
		STATE(d) = CON_QSEX;
	  } else {
		SEND_TO_Q("\n\rThat's not a race.\n\rRACE?:", d);
		show_race_choice(d);
		STATE(d) = CON_QRACE;
	   /* bogus race selection! */
	  }



    }
    break;

  case CON_NME:         /* wait for input of name       */
    if (!d->character) {
      CREATE(d->character, struct char_data, 1);
      clear_char(d->character);
      d->character->desc = d;
    }

    for (; isspace(*arg); arg++)  ;
    if (!*arg)
      close_socket(d);
  else {
    if(_parse_name(arg, tmp_name))    {
      SEND_TO_Q("Illegal name, please try another.\r\n", d);
      SEND_TO_Q("Name: ", d);
      return;
    }
    if (SiteLock(d->host))  {
      SEND_TO_Q("Sorry, this site is temporarily banned.\n\r",d);
      STATE(d)=CON_WIZLOCK;
      return;
    }


    if ((player_i = load_char(tmp_name, &tmp_store)) > -1)  {
  /*  -- commented out long ago ... (bcw 13/july/1999)
   *  check for tmp_store.max_corpse;
 if (tmp_store.max_corpse > 3) {
 SEND_TO_Q("Too many corpses in game, can't connect\n\r", d);
 sprintf(buf, "%s: too many corpses.",tmp_name);
 log(buf);
 STATE(d) = CON_WIZLOCK;
 break;
 }
*/
/* connecting an existing character ... */
      store_to_char(&tmp_store, d->character);
      strcpy(d->pwd, tmp_store.pwd);
      d->pos = player_table[player_i].nr;
      SEND_TO_Q("Password: ", d);
      write(d->descriptor, echo_off, 4);
      STATE(d) = CON_PWDNRM;
    } else {
/* player unknown gotta make a new */
      if(_check_ass_name(tmp_name)) {
        SEND_TO_Q("\n\rIllegal name, please try another.", d);
        SEND_TO_Q("Name: ", d);
        return;
      }
/* move forward creating new character */
      if (!IS_SET(SystemFlags, SYS_WIZLOCKED)) {
        CREATE(GET_NAME(d->character), char,
          strlen(tmp_name) + 1);
        strcpy(GET_NAME(d->character), CAP(tmp_name));
        sprintf(buf, "Did I get that right, %s (Y/N)? ",
          tmp_name);
        SEND_TO_Q(buf, d);
        STATE(d) = CON_NMECNF;
/* We are wiz locked ... */
      } else {
        sprintf(buf, "Sorry, no new characters at this time\n\r");
        SEND_TO_Q(buf,d);
        STATE(d) = CON_WIZLOCK;
      }
    }
  }
    break;

  case CON_NMECNF:      /* wait for conf. of new name   */
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    if (*arg == 'y' || *arg == 'Y')     {
      write(d->descriptor, echo_on, 4);
      SEND_TO_Q("New character.\n\r", d);

      sprintf(buf,
	      "Give me a password for %s: ",
	      GET_NAME(d->character));

      SEND_TO_Q(buf, d);
      write(d->descriptor, echo_off, 4);
      STATE(d) = CON_PWDGET;
    } else {
      if (*arg == 'n' || *arg == 'N') {
	SEND_TO_Q("Ok, what IS it, then? ", d);
if (GET_NAME(d->character))
	free(GET_NAME(d->character));
	STATE(d) = CON_NME;
      } else { /* Please do Y or N */
	SEND_TO_Q("Please type Yes or No? ", d);
      }
    }
    break;

  case CON_PWDNRM:      /* get pwd for known player     */
/* skip whitespaces */
    for (; isspace(*arg); arg++);
    if (!*arg)
      close_socket(d);
    else  {
      int already_p = 0;
      if ( strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) {
//This send a log message
//when someone ennter a wrong PW
//- Manwe Windmaster 200697
        SEND_TO_Q("Wrong password.\n\r", d);
        sprintf(buf, "%s entered a wrong password", GET_NAME(d->character));
        log(buf);
        close_socket(d);
        return;
      }
#if IMPL_SECURITY
      if (top_of_p_table > 0) {
        if (GetMaxLevel(d->character) >= 58) {
          switch(SecCheck(GET_NAME(d->character), d->host)) {
            case -1:
            case 0:
              SEND_TO_Q("Security check reveals invalid site\n\r", d);
              SEND_TO_Q("Speak to an implementor to fix problem\n\r", d);
              SEND_TO_Q("If you are an implementor, add yourself to the\n\r",d);
              SEND_TO_Q("Security directory (lib/security)\n\r",d);
              close_socket(d);
              break;
          }
        }
/*  hmmm an empty else statement ... something un finished? --bcw
        else {
        }
 */
      }
#endif

/* Check if already playing */
    for(k=descriptor_list; k; k = k->next) {
      if ((k->character != d->character) && k->character) {
/* check to see if 'character' was switched to by the one trying to connect */
        if (k->original) {
          if (GET_NAME(k->original)
              && (str_cmp(GET_NAME(k->original), GET_NAME(d->character)) == 0))  {
/* Yep ... --start here--*/
            already_p = 1;
/*
 *          SEND_TO_Q("Already playing, cannot connect\n\r", d);
 *          SEND_TO_Q("Name: ", d);
 *          return;
 */
          }
        } else { /* No switch has been made */
          if (GET_NAME(k->character)
              && (str_cmp(GET_NAME(k->character), GET_NAME(d->character)) == 0))  {
            already_p = 1;
/*
 *          SEND_TO_Q("Already playing, connect\n\r", d);
 *          SEND_TO_Q("Name: ", d);
 *          return;
 */
          }
        }
      }
      if ( already_p ) {
            close_socket(k);
            break;
/*      We are trying a more direct route now ... --bcw
        write(d->descriptor, echo_on, 6);
        SEND_TO_Q("Already playing, reconnect anyway? (Y/N)", d);
        STATE(d) = CON_ALREADY_PLAYING;
        return;
*/
      }
    }

/* Check if disconnected ...  */
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
        if ((!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch))
            && !tmp_ch->desc && !IS_NPC(tmp_ch))
            || (IS_NPC(tmp_ch) && tmp_ch->orig
            && !str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch->orig)))) {

          write(d->descriptor, echo_on, 6);
          SEND_TO_Q("Reconnecting.\n\r", d);

          free_char(d->character);
          tmp_ch->desc = d;
          d->character = tmp_ch;
          tmp_ch->specials.timer = 0;
          if (!IS_IMMORTAL(tmp_ch)) {
            tmp_ch->invis_level = 0;
          }
          if (tmp_ch->orig) {
            tmp_ch->desc->original = tmp_ch->orig;
            tmp_ch->orig = 0;
          }
          d->character->persist = 0;
          STATE(d) = CON_PLYNG;

          act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
          sprintf(buf, "%s[%s] has reconnected.",
          GET_NAME(d->character), d->host);
          log(buf);
          return;
        }
      }


      sprintf(buf, "%s[%s] has connected.", GET_NAME(d->character), d->host);
      log(buf);
      SEND_TO_Q(motd, d);
      SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
      STATE(d) = CON_RMOTD;
    }
    break;

/* changed 'tack' for the implementation of this ... ;) */
#if 0
  case CON_ALREADY_PLAYING:      /* confirm this players desire to reconnect */
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    if (*arg == 'y' || *arg == 'Y') {
      for (tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next) {
        if ( (!str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch))
            && !IS_NPC(tmp_ch))
          || (IS_NPC(tmp_ch)
            && tmp_ch->orig
            && !str_cmp(GET_NAME(d->character), GET_NAME(tmp_ch->orig)))) {

          write(d->descriptor, echo_on, 6);
          SEND_TO_Q("Reconnecting.\n\r", d);

/* sync up the old and new connection */
          close_socket(tmp_ch->desc);
          tmp_ch->desc = d;
          free_char(d->character);
          d->character = tmp_ch;
          tmp_ch->specials.timer = 0;
          if (!IS_IMMORTAL(tmp_ch)) {
            tmp_ch->invis_level = 0;
          }
          if (tmp_ch->orig) {
            tmp_ch->desc->original = tmp_ch->orig;
            tmp_ch->orig = 0;
          }
          d->character->persist = 0;
          STATE(d) = CON_PLYNG;

          act("$n has reconnected.", TRUE, tmp_ch, 0, 0, TO_ROOM);
          sprintf(buf, "%s[%s] has reconnected.",
            GET_NAME(d->character), d->host);
          log(buf);
          return;
        }
      }
/* ooppss .... */
        SEND_TO_Q("Name: ", d);
        STATE(d) = CON_NME;

    } else {
      if (*arg == 'n' || *arg == 'N') {
        SEND_TO_Q("Ok, Try again later.", d);
        close_socket(d);
      } else { /* Please do Y or N */
        SEND_TO_Q("Please type Yes or No? ", d);
      }
    }

    break;

#endif

  case CON_PWDGET:      /* get pwd for new player       */
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    if (!*arg || strlen(arg) > 10)      {

      write(d->descriptor, echo_on, 6);
      SEND_TO_Q("Illegal password.\n\r", d);
      SEND_TO_Q("Password: ", d);

      write(d->descriptor, echo_off, 4);
      return;
    }

    strncpy(d->pwd,(char *)crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    write(d->descriptor, echo_on, 6);
    SEND_TO_Q("Please retype password: ", d);
    write(d->descriptor, echo_off, 4);
    STATE(d) = CON_PWDCNF;
    break;

   case CON_PWDCNF:      /* get confirmation of new pwd  */
      /* skip whitespaces */
      for (; isspace(*arg); arg++);

      if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10)) {
        write(d->descriptor, echo_on, 6);

        SEND_TO_Q("Passwords don't match.\n\r", d);
        SEND_TO_Q("Retype password: ", d);
        STATE(d) = CON_PWDGET;
        write(d->descriptor, echo_off, 4);
        return;
      } else {
        write(d->descriptor, echo_on, 6);

        SEND_TO_Q("Would you like to have ansi colors? ",d);
        STATE(d) = CON_ANSI;

        }
      break;

  case CON_QSEX:                /* query sex of new user        */
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    switch (*arg)
      {
      case 'm':
      case 'M':
	/* sex MALE */
	d->character->player.sex = SEX_MALE;
	break;

      case 'f':
      case 'F':
	/* sex FEMALE */
	d->character->player.sex = SEX_FEMALE;
	break;

      default:
	SEND_TO_Q("That's not a sex..\n\r", d);
	SEND_TO_Q("What IS your sex? :", d);
	return;
	break;
      }

    SEND_TO_Q("\n\rSelect your class now.\n\r",d);
    show_class_selection(d,GET_RACE(d->character));
    SEND_TO_Q("Enter ? for help.\n\r", d);
    SEND_TO_Q("\n\rClass :", d);
    STATE(d) = CON_QCLASS;
    break;

  case CON_STAT_LIST:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    index = 0;
    while (*arg && index < MAX_STAT) {
      if (*arg == 'S' || *arg == 's')
	d->stat[index++] = 's';
      if (*arg == 'I' || *arg == 'i')
	d->stat[index++] = 'i';
      if (*arg == 'W' || *arg == 'w')
	d->stat[index++] = 'w';
      if (*arg == 'D' || *arg == 'd')
	d->stat[index++] = 'd';
      if (*arg == 'C' || *arg == 'c') {
	arg++;
	if (*arg == 'O' || *arg == 'o') {
	  d->stat[index++] = 'o';
	} else if (*arg == 'H' || *arg == 'h') {
	  d->stat[index++] = 'h';
	} else {
	  SEND_TO_Q("That was an invalid choice.\n\r",d);
	  SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
	  SEND_TO_Q("Seperated by spaces.  don't duplicate letters \n\r", d);
	  SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
	  SEND_TO_Q("next in intelligence, Wisdom, Dex, Con and lastly Charisma\n\r",d);
	  SEND_TO_Q("Your choice? ",d);
	  STATE(d) = CON_STAT_LIST;
	  break;
	}
      }
      arg++;
    }

    if (index < MAX_STAT) {
      SEND_TO_Q("You did not enter enough legal stats\n\r", d);
      SEND_TO_Q("That was an invalid choice.\n\r",d);
      SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
      SEND_TO_Q("Seperated by spaces, don't duplicate letters \n\r", d);
      SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
      SEND_TO_Q("next in intelligence, Wisdom, Dex, Con and lastly Charisma\n\r",d);
      SEND_TO_Q("Your choice? ",d);
      STATE(d) = CON_STAT_LIST;
      break;
    } else {

      roll_abilities(d->character);
      SEND_TO_Q("Your current stats are:\n\r", d);
      sprintf(buf, "STR: %s\n\r", STAT_SWORD(GET_STR(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CON: %s\n\r", STAT_SWORD(GET_CON(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "DEX: %s\n\r", STAT_SWORD(GET_DEX(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "INT: %s\n\r", STAT_SWORD(GET_INT(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "WIS: %s\n\r", STAT_SWORD(GET_WIS(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CHR: %s\n\r", STAT_SWORD(GET_CHR(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "\n\rYou have 9 rerolls left, press R to reroll, any other key to keep.\n\r");
      SEND_TO_Q(buf, d);
	if (IS_SET(SystemFlags,SYS_REQAPPROVE)) {
	      /* set the AUTH flags */
	      /* (3 chances) */
	      d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;
	}
    }

if (IS_SET(SystemFlags,SYS_REQAPPROVE)) {
    /* set the AUTH flags */
    /* (3 chances) */
    d->character->generic = NEWBIE_REQUEST+NEWBIE_CHANCES;
    }

    STATE(d) = CON_REROLL;
    break;

  case CON_REROLL:

  for(; isspace(*arg);arg++);

  d->character->reroll--;

 if (*arg!='r' &&  *arg!='R'){
    SEND_TO_Q("Stats chosen!", d);
    STATE(d)= CON_RMOTD;
    if (IS_SET(SystemFlags,SYS_REQAPPROVE)) {
      STATE(d) = CON_AUTH;
      SEND_TO_Q("***PRESS ENTER**", d);
     } else {
	sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
	log(buf);
	/*
	 ** now that classes are set, initialize
	*/
	init_char(d->character);
	/* create an entry in the file */
	d->pos = create_entry(GET_NAME(d->character));
	save_char(d->character, AUTO_RENT);

        /* page_string(d,NEWBIE_NOTE,1); */
	 SEND_TO_Q(NEWBIE_NOTE, d);
	 STATE(d) = CON_RMOTD;
	 SEND_TO_Q(motd, d);
	 SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
	 STATE(d) = CON_RMOTD;
         break;
      }
    } else {
      if (d->character->reroll != 0){
      roll_abilities(d->character);
      SEND_TO_Q("Your current stats are:\n\r", d);
      sprintf(buf, "STR: %s\n\r", STAT_SWORD(GET_STR(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CON: %s\n\r", STAT_SWORD(GET_CON(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "DEX: %s\n\r", STAT_SWORD(GET_DEX(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "INT: %s\n\r", STAT_SWORD(GET_INT(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "WIS: %s\n\r", STAT_SWORD(GET_WIS(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CHR: %s\n\r", STAT_SWORD(GET_CHR(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "\n\rYou have %d rerolls left, press R to reroll, any other key to keep.\n\r", d->character->reroll);
      SEND_TO_Q(buf, d);
      STATE(d)= CON_REROLL;
      break;
      } else {
      roll_abilities(d->character);
      SEND_TO_Q("Your final stats are:\n\r", d);
      sprintf(buf, "STR: %s\n\r", STAT_SWORD(GET_STR(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CON: %s\n\r", STAT_SWORD(GET_CON(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "DEX: %s\n\r", STAT_SWORD(GET_DEX(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "INT: %s\n\r", STAT_SWORD(GET_INT(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "WIS: %s\n\r", STAT_SWORD(GET_WIS(d->character)));
      SEND_TO_Q(buf,d);
      sprintf(buf, "CHR: %s\n\r", STAT_SWORD(GET_CHR(d->character)));
      SEND_TO_Q(buf,d);
      SEND_TO_Q("Stats chosen!", d);
    STATE(d)= CON_RMOTD;
    if (IS_SET(SystemFlags,SYS_REQAPPROVE)) {
      STATE(d) = CON_AUTH;
      SEND_TO_Q("***PRESS ENTER**", d);
     } else {
	sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
	log(buf);
	/*
	 ** now that classes are set, initialize
	 */
	init_char(d->character);
	/* create an entry in the file */
	d->pos = create_entry(GET_NAME(d->character));
	save_char(d->character, AUTO_RENT);

	 SEND_TO_Q("\n\r\n*** PRESS ENTER: ", d);
	 STATE(d) = CON_PRESS_ENTER;
         break;
      }
  }
case CON_PRESS_ENTER:
       /*   page_string(d,NEWBIE_NOTE,1); */
	 SEND_TO_Q(NEWBIE_NOTE, d);
	 STATE(d) = CON_RMOTD;
	 SEND_TO_Q(motd, d);
	 SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
	 STATE(d) = CON_RMOTD;
         break;

  case CON_QCLASS :
{
  int total_class=0;
  int ii=0;
    /* skip whitespaces */

    for (; isspace(*arg); arg++);
    d->character->player.class = 0;
    count=0;
    oops=FALSE;


   switch (*arg) {
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	     {
	      switch(GET_RACE(d->character)) {
		case RACE_AVARIEL:{
		ii=0;
		while (d->character->player.class==0 &&
                       avariel_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=avariel_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }
                    }
		    else show_class_selection(d,GET_RACE(d->character));

				 } break;


		case RACE_GOLD_ELF:{
		ii=0;
		while (d->character->player.class==0 &&
                       gold_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=gold_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }
                    }
		    else show_class_selection(d,GET_RACE(d->character));

				 } break;


		case RACE_WILD_ELF:{

		ii=0;
		while (d->character->player.class==0 &&
                    wild_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=wild_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }
                    }
		    else show_class_selection(d,GET_RACE(d->character));

				  } break;

		case RACE_SEA_ELF:{
		ii=0;
		while (d->character->player.class==0 &&
                    sea_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=sea_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }
                    }
		    else show_class_selection(d,GET_RACE(d->character));

				  } break;


		case RACE_MOON_ELF: {
		ii=0;
		while (d->character->player.class==0 &&
                       moon_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=moon_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }
                    }
		    else show_class_selection(d,GET_RACE(d->character));

				  } break;

		case RACE_HUMAN:{
		ii=0;
		while (d->character->player.class==0 && human_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=human_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {
		     SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;
		case RACE_HALFLING:{
		ii=0;
		while (d->character->player.class==0 && halfling_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=halfling_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

	    case RACE_FOREST_GNOME:{
		ii=0;
		while (d->character->player.class==0 &&
                    forest_gnome_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)


              d->character->player.class=forest_gnome_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}

		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

		case RACE_DEEP_GNOME:
		case RACE_ROCK_GNOME:{
		ii=0;
		while (d->character->player.class==0 &&
                    rock_gnome_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)

                    d->character->player.class=rock_gnome_class_choice[ii];
		    ii++;
		} /* end while */
		if ((d->character->player.class != 0))
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}

		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

		case RACE_DWARF:{
		ii=0;
		while (d->character->player.class==0 && dwarf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=dwarf_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

		case RACE_HALF_ELF:{
		ii=0;
		while (d->character->player.class==0 && half_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=half_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

		case RACE_HALF_OGRE:{
		ii=0;
		while (d->character->player.class==0 && half_ogre_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=half_ogre_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;
		case RACE_HALF_GIANT:{
		ii=0;
		while (d->character->player.class==0 && half_giant_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=half_giant_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;
		case RACE_HALF_ORC:{
		ii=0;
		while (d->character->player.class==0 && half_orc_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=half_orc_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
		  } break;

     case RACE_ORC:{
		ii=0;
		while (d->character->player.class==0 && orc_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=orc_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

     case RACE_GOBLIN:{
		ii=0;
		while (d->character->player.class==0 && goblin_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=goblin_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		   {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

     case RACE_DROW:{
		ii=0;
		while (d->character->player.class==0 && dark_elf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=dark_elf_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		   {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

     case RACE_DARK_DWARF:{
		ii=0;
		while (d->character->player.class==0 && dark_dwarf_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=dark_dwarf_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;
     case RACE_TROLL:{
		ii=0;
		while (d->character->player.class==0 && troll_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=troll_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
				  } break;

     default:{
		ii=0;
		while (d->character->player.class==0 && default_class_choice[ii] !=0) {
		    if (atoi(arg) == ii)
		    d->character->player.class=default_class_choice[ii];
		    ii++;
		} /* end while */
		if (d->character->player.class != 0)
		    {if (!HasClass(d->character, CLASS_MAGIC_USER))
		    {
		     SEND_TO_Q("\n\rSelect your stat priority, by listing
them from highest to lowest\n\r",d);
    		     SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
                     SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
                     SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
                     SEND_TO_Q("Your choices? ",d);
                     STATE(d) = CON_STAT_LIST;
                    }}
		    else show_class_selection(d,GET_RACE(d->character));
	    } break;


	       } /* end race switch */
	     } break;


   case '?' : {
		SEND_TO_Q(CLASS_HELP,d);
		SEND_TO_Q("\n\rSelect your class now.\n\r",d);
		show_class_selection(d,GET_RACE(d->character));
		SEND_TO_Q("Enter ? for help.\n\r", d);
		SEND_TO_Q("\n\rClass :", d);
		break;
	       }
  default : {
		SEND_TO_Q("Invalid selection!\n\r",d);
		show_class_selection(d,GET_RACE(d->character));
		SEND_TO_Q("Enter ? for help.\n\r", d);
		SEND_TO_Q("\n\rClass :", d);
		break;
	    }


	   } /* end arg switch */



  if (HasClass(d->character,CLASS_MAGIC_USER)) {
     SEND_TO_Q(RU_SORCERER,d);
     STATE(d) = CON_CHECK_MAGE_TYPE;
     break;
    }

  } /* ADDED */

  }
  break; /* end CON_QCLASS */


  case CON_AUTH: {   /* notify gods */
     if (d->character->generic >= NEWBIE_START) {
	/*
	** now that classes are set, initialize
	*/
       init_char(d->character);
       /* create an entry in the file */
       d->pos = create_entry(GET_NAME(d->character));
       save_char(d->character, AUTO_RENT);
	SEND_TO_Q(motd, d);
	SEND_TO_Q("\n\r\n*** PRESS RETURN: ", d);
	STATE(d) = CON_RMOTD;
     } else if (d->character->generic >= NEWBIE_REQUEST) {
       sprintf(buf, "%s [%s] new player.", GET_NAME(d->character), d->host);
       log_sev(buf, 7);
       if (!strncmp(d->host,"128.197.152",11))
	   d->character->generic=1;
  /* I decided to give them another chance.  -Steppenwolf  */
  /* They blew it. -DM */
       if (!strncmp(d->host,"oak.grove", 9)
	    || !strncmp(d->host,"143.195.1.20",12)) {
	 d->character->generic=1;
       } else {
	 if (top_of_p_table > 0) {
	   sprintf(buf,"Type Authorize %s to allow into game.", GET_NAME(d->character));
	   log_sev(buf, 6);
	   log_sev("type 'Wizhelp Authorize' for other commands", 2);
	 } else {
	   log("Initial character.  Authorized Automatically");
	   d->character->generic = NEWBIE_START+5;
	 }
       }
       /*
       **  enough for gods.  now player is told to shut up.
       */
       d->character->generic--;   /* NEWBIE_START == 3 == 3 chances */
       sprintf(buf, "Please wait. You have %d requests remaining.\n\r",
	       d->character->generic);
       SEND_TO_Q(buf, d);
       if (d->character->generic == 0) {
	 SEND_TO_Q("Goodbye.", d);
	 STATE(d) = CON_WIZLOCK;   /* axe them */
	 break;
       } else {
	 SEND_TO_Q("Please Wait.\n\r", d);
	 STATE(d) = CON_AUTH;
       }
     } else {   /* Axe them */
       STATE(d) = CON_WIZLOCK;
     }
   }
   break;

case CON_CHECK_MAGE_TYPE:{
     for (;isspace(*arg);arg++);
     if (!strcmp(arg,"yes")) {
	  d->character->player.class -=CLASS_MAGIC_USER;
	  d->character->player.class +=CLASS_SORCERER;
	} /* end we wanted Sorcerer class! */
	SEND_TO_Q("\n\rSelect your stat priority, by listing them from highest to lowest\n\r",d);
   	SEND_TO_Q("Seperated by spaces.. don't duplicate\n\r", d);
   	SEND_TO_Q("for example: 'S I W D Co Ch' would put the highest roll in Strength, \n\r",d);
        SEND_TO_Q("next in intelligence, Wisdom, Dex, Con, and lastly charisma\n\r",d);
        SEND_TO_Q("Your choices? ",d);
        STATE(d) = CON_STAT_LIST;
   } break;

  case CON_RMOTD:               /* read CR after printing motd  */
    if(GetMaxLevel(d->character) > 50 ) {
	 SEND_TO_Q(wmotd, d);
	 SEND_TO_Q("\n\r\n[PRESS RETURN]", d);
	 STATE(d) = CON_WMOTD;
	 break;
	}
    if(d->character->term != 0)
       ScreenOff(d->character);
    send_to_char(MENU,d->character);
    
    STATE(d) = CON_SLCT;
    if (IS_SET(SystemFlags, SYS_WIZLOCKED) || SiteLock(d->host))  {
      if (GetMaxLevel(d->character) < LOW_IMMORTAL) {
	sprintf(buf, "Sorry, the game is locked up for repair or your site is bannedr\n\r");
	SEND_TO_Q(buf,d);
	STATE(d) = CON_WIZLOCK;
      }
    }
    break;


  case CON_WMOTD:               /* read CR after printing motd  */
    send_to_char(MENU,d->character);
    
    STATE(d) = CON_SLCT;
    if (IS_SET(SystemFlags, SYS_WIZLOCKED) || SiteLock(d->host)) {
      if (GetMaxLevel(d->character) < LOW_IMMORTAL) {
	sprintf(buf, "Sorry, the game is locked up for repair or your site is banned.\n\r");
	SEND_TO_Q(buf,d);
	STATE(d) = CON_WIZLOCK;
      }
    }
    break;

  case CON_WIZLOCK:
    close_socket(d);
    break;

 /* case CON_CITY_CHOICE: This thing was just a silly crap!
    for (; isspace(*arg); arg++);
    if (d->character->in_room != NOWHERE) {
      SEND_TO_Q("This choice is only valid when you have been auto-saved\n\r",d);
      STATE(d) = CON_SLCT;
    } else {
      switch (*arg)     {

      case '1':

	reset_char(d->character);
	sprintf(buf, "Loading %s's equipment", d->character->player.name);
	log(buf);
	load_char_objs(d->character);
	save_char(d->character, AUTO_RENT);
	send_to_char(WELC_MESSG, d->character);
	d->character->next = character_list;
	character_list = d->character;

	char_to_room(d->character, 3001);
	d->character->player.hometown = 3001;


	d->character->specials.tick = plr_tick_count++;
	if (plr_tick_count == PLR_TICK_WRAP)
	  plr_tick_count=0;


	act("$n has entered the game.",
	    TRUE, d->character, 0, 0, TO_ROOM);
	STATE(d) = CON_PLYNG;
	if (!GetMaxLevel(d->character))
	  do_start(d->character);
	do_look(d->character, "",15);
	d->prompt_mode = 1;

	break;
      case '2':

	reset_char(d->character);
	sprintf(buf, "Loading %s's equipment", d->character->player.name);
	log(buf);
	load_char_objs(d->character);
	save_char(d->character, AUTO_RENT);
	send_to_char(WELC_MESSG, d->character);
	d->character->next = character_list;
	character_list = d->character;

	char_to_room(d->character, 1103);
	d->character->player.hometown = 1103;

	  d->character->specials.tick = plr_tick_count++;
	  if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;

	  act("$n has entered the game.",
	      TRUE, d->character, 0, 0, TO_ROOM);
	  STATE(d) = CON_PLYNG;
	  if (!GetMaxLevel(d->character))
	     do_start(d->character);
	  do_look(d->character, "",15);
	  d->prompt_mode = 1;

	break;
      case '3':
	if (GetMaxLevel(d->character) > 5) {

	  reset_char(d->character);
	  sprintf(buf, "Loading %s's equipment", d->character->player.name);
	  log(buf);
	  load_char_objs(d->character);
	  save_char(d->character, AUTO_RENT);
	  send_to_char(WELC_MESSG, d->character);
	  d->character->next = character_list;
	  character_list = d->character;

	  char_to_room(d->character, 18221);
	  d->character->player.hometown = 18221;

	  d->character->specials.tick = plr_tick_count++;
	  if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;

	  act("$n has entered the game.",
	      TRUE, d->character, 0, 0, TO_ROOM);
	  STATE(d) = CON_PLYNG;
	  if (!GetMaxLevel(d->character))
	     do_start(d->character);
	  do_look(d->character, "",15);
	  d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}
      case '4':
	if (GetMaxLevel(d->character) > 5) {

	  reset_char(d->character);
	  sprintf(buf, "Loading %s's equipment", d->character->player.name);
	  log(buf);
	  load_char_objs(d->character);
	  save_char(d->character, AUTO_RENT);
	  send_to_char(WELC_MESSG, d->character);
	  d->character->next = character_list;
	  character_list = d->character;

	  char_to_room(d->character, 3606);
	  d->character->player.hometown = 3606;

	  d->character->specials.tick = plr_tick_count++;
	  if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;

	  act("$n has entered the game.",
	      TRUE, d->character, 0, 0, TO_ROOM);
	  STATE(d) = CON_PLYNG;
	  if (!GetMaxLevel(d->character))
	     do_start(d->character);
	  do_look(d->character, "",15);
	  d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}
      case '5':
	if (GetMaxLevel(d->character) > 5) {

	  reset_char(d->character);
	  sprintf(buf, "Loading %s's equipment", d->character->player.name);
	  log(buf);
	  load_char_objs(d->character);
	  save_char(d->character, AUTO_RENT);
	  send_to_char(WELC_MESSG, d->character);
	  d->character->next = character_list;
	  character_list = d->character;

	  char_to_room(d->character, 16107);
	  d->character->player.hometown = 16107;

	  d->character->specials.tick = plr_tick_count++;
	  if (plr_tick_count == PLR_TICK_WRAP)
	     plr_tick_count=0;

	  act("$n has entered the game.",
	      TRUE, d->character, 0, 0, TO_ROOM);
	  STATE(d) = CON_PLYNG;
	  if (!GetMaxLevel(d->character))
	     do_start(d->character);
	  do_look(d->character, "",15);
	  d->prompt_mode = 1;
	  break;

	} else {
	  SEND_TO_Q("That was an illegal choice.\n\r", d);
	  STATE(d) = CON_SLCT;
	  break;
	}

      default:
	SEND_TO_Q("That was an illegal choice.\n\r", d);
	STATE(d) = CON_SLCT;
	break;
      }
    }
    break; */


  case CON_DELETE_ME:{
      int i;
      struct char_file_u ch_st;
      FILE *char_file;


      for (; isspace(*arg); arg++);

   if (!strcmp(arg,"yes") && strcmp("Guest",GET_NAME(d->character)) ) {
      char buf[256];

       sprintf(buf,"%s just killed theirself!",GET_NAME(d->character));
       log(buf);
      for (i = 0; i <= top_of_p_table; i++)     {
	if (!str_cmp((player_table + i)->name, GET_NAME(d->character))) {
if ((player_table+i)->name)
	  free((player_table +i)->name);
	  (player_table +i)->name = (char *)malloc(strlen("111111"));
	  strcpy((player_table +i)->name, "111111");
	  break;
	}
      }
      /* get the structure from player_table[i].nr */
      if (!(char_file = fopen(PLAYER_FILE, "r+"))) {
	perror("Opening player file for updating. (interpreter.c, nanny)");
	assert(0);
      }
      rewind(char_file);
      fseek(char_file, (long) (player_table[i].nr *
				   sizeof(struct char_file_u)), 0);

      /* read in the char, change the name, write back */
      fread(&ch_st, sizeof(struct char_file_u), 1, char_file);
      sprintf(ch_st.name,"111111");
      rewind(char_file);
      fseek(char_file, (long) (player_table[i].nr *
			       sizeof(struct char_file_u)), 0);
      fwrite(&ch_st, sizeof(struct char_file_u), 1, char_file);
      fclose(char_file);
      sprintf(buf,"rent/%s",lower( GET_NAME(d->character)) );
      remove(buf);    //remove rent file
      sprintf(buf,"rent/%s.aux",GET_NAME(d->character));
      remove(buf);    //remove rent AUX file
      close_socket(d);
   }
   send_to_char(MENU,d->character);
   //SEND_TO_Q(MENU,d);
   
      STATE(d)= CON_SLCT;
     }
    break;


  case CON_SLCT:                /* get selection from main menu */
    /* skip whitespaces */
    for (; isspace(*arg); arg++);
    switch (*arg)       {
    case '0':
      close_socket(d);
      break;

 /*  case 'K':
   case 'k': {
	      SEND_TO_Q("Are you sure you want to delete yourself? (yes/no) ",d);
	      STATE(d)=CON_DELETE_ME;
	      break;
	} */


    case '1':
      reset_char(d->character);
      sprintf(buf, "Loading %s's equipment", d->character->player.name);
      log(buf);
      load_char_objs(d->character);

	  //	if ((r_num = real_object(12)) >= 0)   {
     //		obj = read_object(r_num, REAL);
     //		obj_to_char(obj,ch);                        /* bread   */
     //	obj = read_object(r_num, REAL);
     //	obj_to_char(obj,ch);                        /* bread   */
  	  //	}
     // if ((r_num = real_object(13)) >= 0)   {
     //		obj = read_object(r_num, REAL);
     //		obj_to_char(obj,ch);                        /* water   */
     //		obj = read_object(r_num, REAL);
     //		obj_to_char(obj,ch);                        /* water   */
  	  //	}
     // ch->points.gold = 150;   /* newbies starts with 150 coins */
     //
      save_char(d->character, AUTO_RENT);
      send_to_char(WELC_MESSG, d->character);
      d->character->next = character_list;
      character_list = d->character;
      if (d->character->in_room == NOWHERE ||
	  d->character->in_room == AUTO_RENT) {
	if (GetMaxLevel(d->character) < LOW_IMMORTAL) {

	  if (d->character->specials.start_room <= 0) {
	    if (GET_RACE(d->character) == RACE_HALFLING) {
	      char_to_room(d->character, 1103);
	      d->character->player.hometown = 1103;
	    } else {
	      char_to_room(d->character, 3001);
	      d->character->player.hometown = 3001;
	    }
	  } else {
	    char_to_room(d->character, d->character->specials.start_room);
	      d->character->player.hometown =
		d->character->specials.start_room;
	  }
	} else {
	  if (d->character->specials.start_room <= NOWHERE) {
	    char_to_room(d->character, 1000);
	    d->character->player.hometown = 1000;
	  } else {
	    if (real_roomp(d->character->specials.start_room)) {
	      char_to_room(d->character, d->character->specials.start_room);
	      d->character->player.hometown =
		d->character->specials.start_room;
	    } else {
	      char_to_room(d->character, 1000);
	      d->character->player.hometown = 1000;
	    }
	  }
	}
      } else {
	if (real_roomp(d->character->in_room)) {
	  char_to_room(d->character,
		       d->character->in_room);
	  d->character->player.hometown = d->character->in_room;
	} else {
	  char_to_room(d->character, 3001);
	  d->character->player.hometown = 3001;
	}
      }

      d->character->specials.tick = plr_tick_count++;
      if (plr_tick_count == PLR_TICK_WRAP)
	plr_tick_count=0;
      act("$n has entered the game.",
	  TRUE, d->character, 0, 0, TO_ROOM);
      STATE(d) = CON_PLYNG;
      if (!GetMaxLevel(d->character))
	 do_start(d->character);
      do_look(d->character, "",15);
      d->prompt_mode = 1;
      break;

    case '2':
      SEND_TO_Q("Enter a text you'd like others to see when they look at you.\n\r", d);
      SEND_TO_Q("Terminate with a '~'.\n\r", d);
      if (d->character->player.description)     {
	  SEND_TO_Q("Old description :\n\r", d);
	  SEND_TO_Q(d->character->player.description, d);
if (d->character->player.description)
	  free(d->character->player.description);
	  d->character->player.description = 0;
	}
      d->str =
	&d->character->player.description;
      d->max_str = 240;
      STATE(d) = CON_EXDSCR;
      break;

    case '3':
      SEND_TO_Q(STORY, d);
      STATE(d) = CON_RMOTD;
      break;
    case '4':
      SEND_TO_Q("Enter a new password: ", d);

      write(d->descriptor, echo_off, 4);

      STATE(d) = CON_PWDNEW;
      break;

  /*  case '5':
	SEND_TO_Q("Where would you like to enter?\n\r", d);
	SEND_TO_Q("1.    Shadowdale\n\r", d);
	SEND_TO_Q("2.    Shire\n\r",    d);
      if (GetMaxLevel(d->character) > 5)
	SEND_TO_Q("3.    Mordilnia\n\r", d);
      if (GetMaxLevel(d->character) > 10)
	SEND_TO_Q("4.    New  Thalos\n\r", d);
      if (GetMaxLevel(d->character) > 20)
	SEND_TO_Q("5.    The Gypsy Village\n\r", d);
      SEND_TO_Q("Your choice? ",d);
      STATE(d) = CON_CITY_CHOICE;
      break;  Same silly crap as up there */

   case 'K':
   case 'k':
	 SEND_TO_Q("Are you sure you want to delete yourself? (yes/no) ",d);
	 STATE(d)=CON_DELETE_ME;
	 break;


    default:
      SEND_TO_Q("Wrong option.\n\r", d);
      send_to_char(MENU,d->character);
      
      break;
    }
    break;

  case CON_PWDNEW:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    if (!*arg || strlen(arg) > 10)      {
	write(d->descriptor, echo_on, 6);

	SEND_TO_Q("Illegal password.\n\r", d);
	SEND_TO_Q("Password: ", d);

	write(d->descriptor, echo_off, 4);


	return;
      }

    strncpy(d->pwd,(char *) crypt(arg, d->character->player.name), 10);
    *(d->pwd + 10) = '\0';
    write(d->descriptor, echo_on, 6);

    SEND_TO_Q("Please retype password: ", d);

    STATE(d) = CON_PWDNCNF;
    write(d->descriptor, echo_off, 4);


    break;
  case CON_PWDNCNF:
    /* skip whitespaces */
    for (; isspace(*arg); arg++);

    if (strncmp((char *)crypt(arg, d->pwd), d->pwd, 10))      {
	  write(d->descriptor, echo_on, 6);
	  SEND_TO_Q("Passwords don't match.\n\r", d);
	  SEND_TO_Q("Retype password: ", d);
	  write(d->descriptor, echo_off, 4);

	  STATE(d) = CON_PWDNEW;
	  return;
	}
    write(d->descriptor, echo_on, 6);

    SEND_TO_Q(
	      "\n\rDone. You must enter the game to make the change final\n\r",
	      d);
    send_to_char(MENU,d->character);
    
    STATE(d) = CON_SLCT;
    break;

  default: {
   char buf[128];
    sprintf(buf,"Nanny: illegal state of con'ness (%d)",STATE(d));
    log(buf);
    abort();
    }
    break;
  }
}

void show_class_selection(struct descriptor_data *d, int r)
{
int i=0;
char buf[254],buf2[254];
extern char *pc_class_types[];

    switch(r) {
	case RACE_AVARIEL:{
		 for (i=0;avariel_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

       sprintbit((unsigned)avariel_class_choice[i],pc_class_types,buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;

	case RACE_GOLD_ELF:{
		 for (i=0;gold_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

       sprintbit((unsigned)gold_elf_class_choice[i],pc_class_types,buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;

	case RACE_WILD_ELF:{
		 for (i=0;wild_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

       sprintbit((unsigned)wild_elf_class_choice[i],pc_class_types,buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;

	case RACE_SEA_ELF:{
		 for (i=0;sea_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

       sprintbit((unsigned)sea_elf_class_choice[i],pc_class_types,buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
	 case RACE_MOON_ELF:{
		 for (i=0;moon_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

       sprintbit((unsigned)moon_elf_class_choice[i],pc_class_types,buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_FOREST_GNOME:{
		 for (i=0;forest_gnome_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);

sprintbit((unsigned)forest_gnome_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;


		 case RACE_DEEP_GNOME:
		 case RACE_ROCK_GNOME:{
		 for (i=0;rock_gnome_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
             sprintbit((unsigned)rock_gnome_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_DWARF:{
		 for (i=0;dwarf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)dwarf_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HALFLING:{
		 for (i=0;halfling_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)halfling_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HUMAN: {
		 for (i=0;human_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)human_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HALF_ELF:{
		 for (i=0;half_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)half_elf_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HALF_ORC:{
		 for (i=0;half_orc_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)half_orc_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HALF_OGRE:{
		 for (i=0;half_ogre_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)half_ogre_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */
		 } break;
		 case RACE_HALF_GIANT:{
		 for (i=0;half_giant_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)half_giant_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 case RACE_ORC:{
		 for (i=0;orc_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)orc_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 case RACE_GOBLIN:{
		 for (i=0;goblin_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)goblin_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 case RACE_DROW:{
		 for (i=0;dark_elf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)dark_elf_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 case RACE_DARK_DWARF:{
		 for (i=0;dark_dwarf_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)dark_dwarf_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 case RACE_TROLL:{
		 for (i=0;troll_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)troll_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

	default:{
		 for (i=0;default_class_choice[i]!=0;i++) {
		  sprintf(buf,"%d) ",i);
		  sprintbit((unsigned)default_class_choice[i],pc_class_types, buf2);
		  strcat(buf,buf2);  strcat(buf,"\n\r");
		  SEND_TO_Q(buf,d);
		  } /* end for */

		 } break;

		 } /* end switch */
}

int show_race_choice(struct descriptor_data *d)
{
	int ii,i=0;
	char buf[255],buf2[254];

SEND_TO_Q(  "                                  Level Limits\n\r",d);
sprintf(buf,"%-4s %-15s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s %-3s\n\r",
	"#","Race","ma","cl","wa","th","dr","mk","ba","so","pa","ra","ps");//,"bd");
	SEND_TO_Q(buf,d);

	while (race_choice[i]!=-1) {
	  sprintf(buf,"%-3d) %-15s",i,RaceName[race_choice[i]]);

	  /* show level limits */
	   for (ii=0;ii<MAX_CLASS-1;ii++) {
	      sprintf(buf2," %-3d",RacialMax[race_choice[i]][ii]);
	      strcat(buf,buf2);
	     } /* end for */

	  strcat(buf,"\n\r");
	  SEND_TO_Q(buf,d);
	  i++;
	 }

SEND_TO_Q("ma=magic user, cl=cleric, wa=warrior, th=thief,dr=druid,mk=monk\n\r",d);
SEND_TO_Q("ba=barbarian,  so=sorcerer,pa=paladin,ra=ranger,ps=psionist,\n\r\n\r",d);
}


