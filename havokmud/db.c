#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>

#include "protos.h"
#define RENT_INACTIVE 3         /* delete the users rent files after 1 month */

#define OLD_ZONE_STUFF TRUE     /* for temp testing of reset_zon */


#define NEW_ZONE_SYSTEM
#define killfile "killfile"

#define OBJ_DIR "objects"
#define MOB_DIR "mobiles"
#define GET_OBJ_NAME(obj)  ((obj)->name)
/**************************************************************************
*  declarations of most of the 'global' variables                         *
************************************************************************ */
int no_mail = 0;
int top_of_scripts = 0;
int top_of_world = 99999;                 /* ref to the top element of world */
#if HASH
struct hash_header      room_db;
#else
struct room_data        *room_db[WORLD_SIZE];
#endif

struct obj_data  *object_list = 0;    /* the global linked list of obj's */
struct char_data *character_list = 0; /* global l-list of chars          */

struct zone_data *zone_table;         /* table of reset data             */
int top_of_zone_table = 0;
struct message_list fight_messages[MAX_MESSAGES]; /* fighting messages   */
struct player_index_element *player_table = 0; /* index to player file   */
int top_of_p_table = 0;               /* ref to top of table             */
int top_of_p_file = 0;
int HpBonus;						/* used for the mob file conversion, mrebuild */
int cog_sequence = 0;

long total_bc = 0;
long room_count=0;
long mob_count=0;
long obj_count=0;
long total_mbc=0;
long total_obc=0;
long total_connections=0;
long total_max_players=0;

int map[7][7];
/*
**  distributed monster stuff
*/
int mob_tick_count=0;
char wmotd[MAX_STRING_LENGTH];
char credits[MAX_STRING_LENGTH];      /* the Credits List                */
char news[MAX_STRING_LENGTH];           /* the news                        */
char motd[MAX_STRING_LENGTH];         /* the messages of today           */
char help[MAX_STRING_LENGTH];         /* the main help page              */
char info[MAX_STRING_LENGTH];         /* the info text                   */
char wizlist[MAX_STRING_LENGTH*2];      /* the wizlist                     */
char iwizlist[MAX_STRING_LENGTH*2];      /* the wizlist                     */
char login[MAX_STRING_LENGTH];


FILE *mob_f,                          /* file containing mob prototypes  */
     *obj_f,                          /* obj prototypes                  */
     *help_fl,                        /* file for help texts (HELP <kwd>)*/
     *wizhelp_fl;                     /* file for wizhelp */

struct index_data *mob_index;         /* index table for mobile file     */
struct index_data *obj_index;         /* index table for object file     */
struct help_index_element *help_index = 0;
struct help_index_element *wizhelp_index = 0;
int top_of_mobt = 0;                  /* top of mobile index table       */
int top_of_objt = 0;                  /* top of object index table       */
int top_of_sort_mobt = 0;
int top_of_sort_objt = 0;

int top_of_alloc_mobt = 99999;
int top_of_alloc_objt = 99999;
int top_of_helpt;                     /* top of help index table         */
int top_of_wizhelpt;                  /* top of wiz help index table         */

struct time_info_data time_info;        /* the infomation about the time   */
struct weather_data weather_info;       /* the infomation about the weather */

/* long saved_rooms[WORLD_SIZE]; */
long number_of_saved_rooms = 0;
extern struct descriptor_data *descriptor_list;
extern struct spell_info_type spell_info[MAX_SPL_LIST];
struct index_data *insert_index(struct index_data *index, void *data, long vnum) ;
struct index_data *insert_objindex(struct index_data *index, void *data, long vnum);
void clean_playerfile(void);
int  read_mob_from_file(struct char_data *mob,FILE *mob_fi);
int  read_mob_from_new_file(struct char_data *mob,FILE *mob_fi);
int GetExpFlags( struct char_data *mob, int exp);
int wizcenter(char *buf);
char *fread_string(FILE *f1);
void setup_dir(FILE *fl, long room, int dir);
struct index_data *generate_indices(FILE *fl, int *top, int *sort_top, int *alloc_top, char *dirname);

int qp_patience;

/************************************************************************
*  routines for booting the system                                       *
*********************************************************************** */


/* body of the booting system */
void boot_db()
{
	int i;
	extern int no_specials;

	log("Boot db -- BEGIN.");

	log("Resetting the game time:");
	reset_time();

	log("Reading newsfile, credits, help-page, info and motd.");

	file_to_string(NEWS_FILE, news);
	file_to_string(CREDITS_FILE, credits);
	file_to_string(MOTD_FILE, motd);
	  file_to_string("wizmotd", wmotd);
	file_to_string(HELP_PAGE_FILE, help);
	file_to_string(INFO_FILE, info);
	file_to_string(WIZLIST_FILE, wizlist);
	file_to_string("login", login);

	log("Initializing Script Files.");

#if 0
    /* some machines are pre-allocation specific when dealing with realloc */
	script_data = (struct scripts *) malloc(sizeof(struct scripts));
	CommandSetup();
	InitScripts();
#endif

	log("Opening mobile, object and help files.");
	if (!(mob_f = fopen(MOB_FILE, "r")))    {
		perror("boot");
		assert(0);
	}

	if (!(obj_f = fopen(OBJ_FILE, "r")))    {
		perror("boot");
		assert(0);
	}
	if (!(help_fl = fopen(HELP_KWRD_FILE, "r")))
	  log("   Could not open help file.");
	else
	  help_index = build_help_index(help_fl,&top_of_helpt);

	if (!(wizhelp_fl = fopen(WIZ_HELP_FILE, "r")))
	  log("   Could not open wizhelp file.");
	else
	  wizhelp_index = build_help_index(wizhelp_fl, &top_of_wizhelpt);

#if CLEAN_AT_BOOT
	log("Clearing inactive players");
	clean_playerfile();
#endif

#if 1
	log("Booting mail system.");
	if (!scan_mail_file()) {
	   log("   Mail system error -- mail system disabled!");
	   no_mail = 1;
	}
#else
  no_mail=1;
#endif

	log("Loading zone table.");
	boot_zones();

	log("Loading saved zone table.");
	boot_saved_zones();

	log("Loading rooms.");
	boot_world();

	log("Loading saved rooms.");
	boot_saved_rooms();

	log("Generating index tables for mobile and object files.");
	mob_index = generate_indices(mob_f, &top_of_mobt, &top_of_sort_mobt, &top_of_alloc_mobt, MOB_DIR);
	obj_index = generate_indices(obj_f, &top_of_objt, &top_of_sort_objt, &top_of_alloc_objt, OBJ_DIR);

	log("Renumbering zone table.");
	renum_zone_table(0);

	log("Generating player index.");
	build_player_index();

	log("Loading fight messages.");
	load_messages();

	log("Loading social messages.");
	boot_social_messages();

  log("Loading pose messages.");
	boot_pose_messages();

	log("Assigning function pointers:");
	if (!no_specials)       {
		log("   Mobiles.");
		assign_mobiles();
		log("   Objects.");
		assign_objects();
		log("   Room.");
		assign_rooms();
	}

	log("   Commands.");
	assign_command_pointers();
	log("   Spells.");
	assign_spell_pointers();

	log("Updating characters with saved items:");
	update_obj_file();

#ifdef SAVEWORLD
	log("Loading saved rooms.");
	ReloadRooms();
#endif


#if LIMITED_ITEMS
	PrintLimitedItems();
#endif
	for (i = 0; i <= top_of_zone_table; i++)        {
	  char  *s;
	  long   d,e;
	  s = zone_table[i].name;
	  d = (i ? (zone_table[i - 1].top + 1) : 0);
	  e = zone_table[i].top;
	  fprintf(stderr, "Performing boot-time init of %d:%s (rooms %ld-%ld).\n",
		  zone_table[i].num, s, d, e);
	  zone_table[i].start = 0;


	  if (i == 0) {
	    fprintf(stderr, "Performing boot-time reload of static mobs\n",
		    s);
	    reset_zone(0,0);

	  }

	  if (i == 1) {
	    fprintf(stderr, "Automatic initialization of  %s\n", s);
	    reset_zone(1,0);
	  }
	}

	reset_q.head = reset_q.tail = 0;

//	/* cycle through pfiles to see if anyone get a statue */
//	generate_legend_statue();

	log("Boot db -- DONE.");
}


/* reset the time in the game from file */
/* reset the time in the game from file */
void reset_time()
{
	char buf[80];
	extern unsigned char moontype;
	long beginning_of_time = 650336715;



	struct time_info_data mud_time_passed(time_t t2, time_t t1);

	time_info = mud_time_passed(time(0), beginning_of_time);

	moontype = time_info.day;

	switch(time_info.hours){
		case 0 :
		case 1 :
		case 2 :
		case 3 :
		case 4 :
		{
			weather_info.sunlight = SUN_DARK;
			switch_light(MOON_SET);
			break;
		}
		case 5 :
		case 6 :
		{
			weather_info.sunlight = SUN_RISE;
			switch_light(SUN_RISE);
			break;
		}
		case 7 :
		case 8 :
		case 9 :
		case 10 :
		case 11 :
		case 12 :
		case 13 :
		case 14 :
		case 15 :
		case 16 :
		case 17 :
		case 18 :
		{
			weather_info.sunlight = SUN_LIGHT;
			break;
		}
		case 19 :
		case 20 :
		{
			weather_info.sunlight = SUN_SET;
			break;
		}
		case 21 :
		case 22 :
		case 23 :
		default :
		{
		  switch_light(SUN_DARK);
		  weather_info.sunlight = SUN_DARK;
		  break;
		}
	}

	sprintf(buf,"   Current Gametime: %dH %dD %dM %dY.",
		time_info.hours, time_info.day,
		time_info.month, time_info.year);
	log(buf);

	weather_info.pressure = 960;
	if ((time_info.month>=7)&&(time_info.month<=12))
		weather_info.pressure += dice(1,50);
	else
		weather_info.pressure += dice(1,80);

	weather_info.change = 0;

	if (weather_info.pressure<=980) {
	   if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_LIGHTNING;
	   else
		weather_info.sky = SKY_LIGHTNING;
	} else if (weather_info.pressure<=1000) {
	   if ((time_info.month>=3) && (time_info.month<=14))
		weather_info.sky = SKY_RAINING;
	   else
		weather_info.sky = SKY_RAINING;
	} else if (weather_info.pressure<=1020) {
		weather_info.sky = SKY_CLOUDY;
	} else {
	    weather_info.sky = SKY_CLOUDLESS;
	}
}



/* update the time file */
void update_time()
{
  return;
}



struct wizs {
   char name[20];
   int level;
};

int intcomp(struct wizs *j, struct wizs *k) {
   return (k->level - j->level);
}

	/*a function to center wizards with color in their titles*/
int wizcenter(char *buf)
{

int center = 38;
int buflen = strlen(buf);
int codes = 6;
int i = 0;
int j = 0;
for(i = 0; i < buflen; i++)
   {
     if( buf[i] == '$' && buf[i+1] == '$' && toupper(buf[i+2]) == 'C' ) {
         codes+=3;
         for ( j=3 ; isdigit(buf[i+j]) && j < 8; j++,codes++);
      }
   }
/*Returns the column to start the name at*/
center = 38 - ((buflen - codes) / 2);
return(center);
}

	 /* generate index table for the player file */
void build_player_index()
{
  int nr = -1, i;
  struct char_file_u dummy;
  FILE *fl;
  int active = 0;
  int temp= 0;
  char tempbuf[MAX_STRING_LENGTH];
  char title[MAX_STRING_LENGTH], tmp2[MAX_STRING_LENGTH], blank[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH*2];

  register int max=0, j;
  int center;

  struct wizlistgen list_wiz;

	/* might use ABS_MAX_CLASS here some time */
  for(j = 0; j < MAX_CLASS; j++)
     list_wiz.number[j] = 0;

	if (!(fl = fopen(PLAYER_FILE, "rb+")))        {
		perror("build player index");
		exit(0);
	}

	for (; !feof(fl);)    {
		fread(&dummy, sizeof(struct char_file_u), 1, fl);
		if (!feof(fl))   /* new record */           {
			/* Create new entry in the list */
			if (nr == -1) {
				CREATE(player_table, struct player_index_element, 1);
				nr = 0;
			} else {
				if (!(player_table = (struct player_index_element *)
					realloc(player_table, (++nr + 1) * sizeof(struct player_index_element))))
				{
					perror("generate index");
					exit(0);
				}
			}

			player_table[nr].nr = nr;

			CREATE(player_table[nr].name, char, strlen(dummy.name) + 1);
			for (i = 0; *(player_table[nr].name + i) = LOWER(*(dummy.name + i)); i++);
			for (j=0;j<ABS_MAX_CLASS;j++)
				if (dummy.level[j] > 60)
					dummy.level[j] = 0;

#if 1
			/* was 5 */
      for (i = 0; i < MAX_CLASS; i++)
       if (dummy.level[i] >= 51 && strcmp(dummy.name, "111111")) {
	sprintf(buf,"GOD: %s, Levels [%d][%d][%d][%d][%d][%d][%d][%d]",dummy.name,
		dummy.level[0],dummy.level[1],dummy.level[2],dummy.level[3],
		dummy.level[4],dummy.level[5],dummy.level[6],dummy.level[7]);
	log(buf);

	max = 0;

		/* MAX_CLASS does not work here... */
	 for (j=0 ; j < MAX_CLASS; j++)
	   if (dummy.level[j] > max) {
	     max = dummy.level[j];
	    }


	list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].name =
	  (char *)strdup(dummy.name);
	list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].title =
	  (char *)strdup(dummy.title);

	list_wiz.lookup[max - 51].stuff[list_wiz.number[max - 51]].active = ((time(0) - dummy.last_logon < 3000000) ? 1 : 0);

	list_wiz.number[max - 51]++;
	break;
      }
#endif


    }
  }


  fclose(fl);

  top_of_p_table = nr;

  top_of_p_file = top_of_p_table;

  log("Began Wizlist Generation.");

  sprintf(wizlist, "\033[2J\033[0;0H\n\r\n\r");
  sprintf(iwizlist,  "\033[2J\033[0;0H\n\r\n\r");
  sprintf(buf, "$$c0012-* $$c0009Creator and Supreme Being [$$c0015%d$$c0009] $$c0012*-\n\r",list_wiz.number[9]);


  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
     strcat(iwizlist, " ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist, buf);


  for(i = 0; i < list_wiz.number[9]; i++) {
	  if(list_wiz.lookup[9].stuff[i].active == 1) {
     	sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[9].stuff[i].title);//,  list_wiz.lookup[9].stuff[i].active ? "ACTIVE": "INACTIVE" );
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
		  strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
      } else {
        sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[9].stuff[i].title);
        center = wizcenter(buf);
 	     for(j = 0; j <= center; j++)
			strcat(iwizlist, " ");
 	     strcat(iwizlist, buf);
 	}
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");
 log("Creator Generated.");

  sprintf(buf, "$$c0012-*$$c0009 Supreme Lords and Ladies [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[8]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist, " ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[8]; i++) {
	if(list_wiz.lookup[8].stuff[i].active == 1) {
       sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[8].stuff[i].title);
       center = wizcenter(buf);
       for(j = 0; j <= center; j++)
	     strcat(wizlist, " ");
       strcat(wizlist, buf);
       active++;
	} else {
       sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[8].stuff[i].title);
       center = wizcenter(buf);
       for(j = 0; j <= center; j++)
          strcat(iwizlist, " ");
       strcat(iwizlist, buf);
	}
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist, "\n\r\n\r");
  log("Implementors Generated.");

  sprintf(buf, "$$c0012-*$$c0009 Lords and Ladies [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[7]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
     strcat(iwizlist, " ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist, buf);

  for(i = 0; i < list_wiz.number[7]; i++) {
	if(list_wiz.lookup[7].stuff[i].active == 1) {
       sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[7].stuff[i].title);
       center = wizcenter(buf);
       for(j = 0; j <= center; j++)
	     strcat(wizlist, " ");
       strcat(wizlist, buf);
       active++;
    } else {
       sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[7].stuff[i].title);
       center = wizcenter(buf);
       for(j = 0; j <= center; j++)
	     strcat(iwizlist, " ");
       strcat(iwizlist, buf);
	}

  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist, "\n\r\n\r");
  log("Gods of Final Judgement Generated.");

  sprintf(buf, "$$c0012-*$$c0009 Gods and Goddesses of Judgement [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[6]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[6]; i++) {
	if(list_wiz.lookup[6].stuff[i].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[6].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[6].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");

  sprintf(buf, "$$c0012-*$$c0009 Greater Gods and Goddesses [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[5]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[5]; i++) {
	if(list_wiz.lookup[5].stuff[i].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[5].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[5].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");



  sprintf(buf, "$$c0012-*$$c0009 Gods and Goddesses [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[4]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[4]; i++) {
	if(list_wiz.lookup[4].stuff[i].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[4].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[4].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");






  sprintf(buf, "$$c0012-*$$c0009 Lesser Gods and Goddesses [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[3]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[3]; i++) {
	if(list_wiz.lookup[3].stuff[i].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[3].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[3].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");




  sprintf(buf, "$$c0012-*$$c0009 Greater Deities [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[2]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[2]; i++) {
	if(list_wiz.lookup[2].stuff[i].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[2].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(wizlist, " ");
        strcat(wizlist, buf);
		active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[2].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");



  sprintf(buf, "$$c0012-*$$c0009 Deities [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[1]);
    center = wizcenter(buf);
    for(i = 0; i <= center; i++) {
       strcat(wizlist, " ");
    	 strcat(iwizlist," ");
    }
    strcat(wizlist, buf);
    strcat(iwizlist,buf);

    for(i = 0; i < list_wiz.number[1]; i++) {
  	if(list_wiz.lookup[1].stuff[i].active == 1) {
    		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[1].stuff[i].title);
          center = wizcenter(buf);
          for(j = 0; j <= center; j++)
  	      strcat(wizlist, " ");
          strcat(wizlist, buf);
          active++;
   	} else {
    		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[1].stuff[i].title);
          center = wizcenter(buf);
          for(j = 0; j <= center; j++)
  	      strcat(iwizlist, " ");
          strcat(iwizlist, buf);



      }
    }

    strcat(wizlist, "\n\r\n\r");
    strcat(iwizlist,"\n\r\n\r");


  sprintf(buf, "$$c0012-*$$c0009 Lesser Deities [$$c0015%d$$c0009] $$c0012*-\n\r", list_wiz.number[0]);
  center = wizcenter(buf);
  for(i = 0; i <= center; i++) {
     strcat(wizlist, " ");
  	 strcat(iwizlist," ");
  }
  strcat(wizlist, buf);
  strcat(iwizlist,buf);

  for(i = 0; i < list_wiz.number[0]; i++) {
	if(list_wiz.lookup[1].stuff[0].active == 1) {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[0].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)

	      strcat(wizlist, " ");
        strcat(wizlist, buf);
        active++;
 	} else {
  		sprintf(buf, "$$c0007%s\n\r", list_wiz.lookup[0].stuff[i].title);
        center = wizcenter(buf);
        for(j = 0; j <= center; j++)
	      strcat(iwizlist, " ");
        strcat(iwizlist, buf);



    }
  }

  strcat(wizlist, "\n\r\n\r");
  strcat(iwizlist,"\n\r\n\r");




  max = 0;
  for(i = 0; i <= 9; i++)
    max += list_wiz.number[i];



  sprintf(buf, "$c000BTotal Gods: $c000W%d$c000w\n\r$c000BType '$c000Wiwizlist$c000B' to see inactive gods.\n\r\n\r", active);
  strcat(wizlist, buf);


  sprintf(buf, "$c000BTotal Gods: $c000W%d$c000w\n\r$c000BType '$c000Wwizlist$c000B' to see active gods.\n\r\n\r", max - active);
  strcat(iwizlist, buf);

  return;
}


void insert_object(struct obj_data *obj, long vnum)
{
  obj_index=insert_objindex(obj_index, (void *)obj, vnum);
}

void insert_mobile(struct char_data *obj, long vnum)
{
  mob_index=insert_index(mob_index, (void *)obj, vnum);
}

struct index_data *insert_index(struct index_data *index, void *data, long vnum)
{
 if (top_of_mobt >= top_of_alloc_mobt) {
    if (!(index = (struct index_data*)
			      realloc(index, (top_of_mobt + 50) * sizeof(struct index_data)))){
      perror("load indices");
      assert(0);
    }
    top_of_alloc_mobt += 50;
  }
  index[top_of_mobt].virtual = vnum;
  index[top_of_mobt].pos = -1;
  index[top_of_mobt].name = strdup(GET_NAME((struct char_data *)data));
  index[top_of_mobt].number = 0;
  index[top_of_mobt].func = 0;
  index[top_of_mobt].data = data;
  top_of_mobt++;
return index;
}

struct index_data *insert_objindex(struct index_data *index, void *data, long vnum)
{
 if (top_of_objt >= top_of_alloc_objt) {
    if (!(index = (struct index_data*)
			      realloc(index, (top_of_objt + 50) * sizeof(struct index_data)))){
      perror("load indices");
      assert(0);
    }
    top_of_alloc_objt += 50;
  }
  index[top_of_objt].virtual = vnum;
  index[top_of_objt].pos = -1;
  index[top_of_objt].name = strdup(GET_OBJ_NAME((struct obj_data *)data));
  index[top_of_objt].number = 0;
  index[top_of_objt].func = 0;
  index[top_of_objt].data = data;
  top_of_objt++;
return index;
}

int read_object_to_memory(long vnum)
{
	int i;
  i=real_object(vnum);
  if(i==-1) return -1;
  obj_index[i].data = (void *)read_object(i, REAL);
}

/* generate index table for object or monster file */
#define MAX_INDICES 5000
struct index_data *generate_indices(FILE *fl, int *top, int *sort_top, int *alloc_top, char *dirname)
{
	FILE *f;
	DIR *dir;
	struct index_data *index;
	struct dirent *ent;
	long i = 0, di = 0, vnum, j;
	long bc=MAX_INDICES;
	long dvnums[MAX_INDICES];
	char buf[82], tbuf[128];

/* scan main obj file */
	rewind(fl);
	for (;;) {
		if (fgets(buf, sizeof(buf), fl)) {
			if (*buf == '#') {
				if (!i)                                          /* first cell */
					CREATE(index, struct index_data, bc);
				else
					if (i >= bc) {
						if (!(index = (struct index_data*)realloc(index, (i + 50) * sizeof(struct index_data)))) {
							perror("load indices");
							assert(0);
						}
						bc += 50;
					}
				sscanf(buf, "#%ld", &index[i].virtual);
				sprintf(tbuf,"%s/%ld",dirname,index[i].virtual);
				if((f=fopen(tbuf,"rt"))==NULL) {
					index[i].pos = ftell(fl);
					index[i].name = (index[i].virtual<99999)?fread_string(fl):strdup("omega");
				} else {
					index[i].pos = -1;
					fscanf(f, "#%*ld\n");
					index[i].name = (index[i].virtual<99999)?fread_string(f):strdup("omega");
					dvnums[di++] = index[i].virtual;
					fclose(f);
				}
				index[i].number = 0;
				index[i].func = 0;
				index[i].data = NULL;
				i++;
			} else {
				if (*buf == '%')        /* EOF */
					break;
			}
		} else {
			fprintf(stderr,"generate indices");
			assert(0);
		}
	}
	*sort_top = i - 1;
	*alloc_top = bc;
	*top = i;
/* scan for directory entrys */
	if((dir=opendir(dirname))==NULL) {
		sprintf(tbuf,"unable to open index directory %s",dirname);
		log(tbuf);
		return(index);
	}
	while((ent=readdir(dir)) != NULL) {
		if(*ent->d_name=='.')
			continue;
		vnum=atoi(ent->d_name);
		if(vnum == 0)
			continue;
		/* search if vnum was already sorted in main database */
		for(j=0;j<di;j++)
			if(dvnums[j] == vnum)
				break;
		if(dvnums[j] == vnum)
			continue;
		sprintf(buf,"%s/%s",dirname, ent->d_name);
		if((f=fopen(buf,"rt")) == NULL) {
			sprintf(tbuf, "Can't open file %s for reading\n",buf);
			log(tbuf);
			continue;
		}
		if (!i)
			CREATE(index, struct index_data, bc);
		else
			if (i >= bc) {
				if (!(index = (struct index_data*)realloc(index, (i + 50) * sizeof(struct index_data)))) {
					perror("load indices");
					assert(0);
				}
				bc += 50;
			}
		fscanf(f, "#%*ld\n");
		index[i].virtual = vnum;
		index[i].pos = -1;
		index[i].name = (index[i].virtual<99999)?fread_string(f):strdup("omega");
		index[i].number = 0;
		index[i].func = 0;
		index[i].data = NULL;
		fclose(f);
		i++;
	}
	*alloc_top = bc;
	*top = i ;
	return(index);
}


void cleanout_room(struct room_data *rp)
{
  int   i;
  struct extra_descr_data *exptr, *nptr;
if (rp->name)
  free(rp->name);
if (rp->description)
  free(rp->description);
  for (i=0; i<6; i++)
    if (rp->dir_option[i]) {
if (rp->dir_option[i]->general_description)
      free(rp->dir_option[i]->general_description);
if (rp->dir_option[i]->keyword)
      free(rp->dir_option[i]->keyword);
if (rp->dir_option[i])
      free (rp->dir_option[i]);
      rp->dir_option[i] = NULL;
    }

  for (exptr=rp->ex_description; exptr; exptr = nptr) {
    nptr = exptr->next;
if (exptr->keyword)
    free(exptr->keyword);
if (exptr->description)
    free(exptr->description);
if (exptr)
    free(exptr);
  }
}

void completely_cleanout_room(struct room_data *rp)
{
  struct char_data      *ch;
  struct obj_data       *obj;

  while (rp->people) {
    ch = rp->people;
    act("The hand of god sweeps across the land and you are swept into the Void.", FALSE, NULL, NULL, NULL, TO_VICT);
    char_from_room(ch);
    char_to_room(ch, 0);        /* send character to the void */
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);        /* send item to the void */
  }

  cleanout_room(rp);
}

void load_one_room(FILE *fl, struct room_data *rp)
{
  char chk[50];
  int   bc=0;
  unsigned long int     tmp;

  struct extra_descr_data *new_descr;

  bc = sizeof(struct room_data);

  rp->name = fread_string(fl);
  if (rp->name && *rp->name)
     bc += strlen(rp->name);
  rp->description = fread_string(fl);
  if (rp->description && *rp->description)
     bc += strlen(rp->description);

  if (top_of_zone_table >= 0) {
    int zone;
    fscanf(fl, " %*d ");

    /* OBS: Assumes ordering of input rooms */

    for (zone=0;
	 rp->number > zone_table[zone].top && zone<=top_of_zone_table;
	 zone++)
      ;
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      assert(0);
    }
    rp->zone = zone;
  }
  fscanf(fl, " %d ", &tmp);
  rp->room_flags = tmp;
  fscanf(fl, " %d ", &tmp);
  rp->sector_type = tmp;

  if (tmp == -1) {
    fscanf(fl, " %d", &tmp);
    rp->tele_time = tmp;
    fscanf(fl, " %d", &tmp);
    rp->tele_targ = tmp;
    fscanf(fl, " %d", &tmp);
    rp->tele_mask = tmp;
    if (IS_SET(TELE_COUNT, rp->tele_mask)) {
      fscanf(fl, "%d ", &tmp);
      rp->tele_cnt = tmp;
    } else {
      rp->tele_cnt = 0;
    }
    fscanf(fl, " %d", &tmp);
    rp->sector_type = tmp;
  } else {
    rp->tele_time = 0;
    rp->tele_targ = 0;
    rp->tele_mask = 0;
    rp->tele_cnt  = 0;
  }

  if (tmp == SECT_WATER_NOSWIM || tmp == SECT_UNDERWATER)  { /* river */
    /* read direction and rate of flow */
    fscanf(fl, " %d ", &tmp);
    rp->river_speed = tmp;
    fscanf(fl, " %d ", &tmp);
    rp->river_dir = tmp;
  }

  if (rp->room_flags & TUNNEL) {  /* read in mobile limit on tunnel */
    fscanf(fl, " %d ", &tmp);
    rp->moblim = tmp;
  }

  rp->funct = 0;
  rp->light = 0; /* Zero light sources */

  for (tmp = 0; tmp <= 5; tmp++)
    rp->dir_option[tmp] = 0;

  rp->ex_description = 0;

  while (1==fscanf(fl, " %s \n", chk)) {
    static char buf[MAX_INPUT_LENGTH];
    switch (*chk) {
    case 'D':
      setup_dir(fl, rp->number, atoi(chk + 1));
      bc += sizeof(struct room_direction_data);

/*    bc += strlen(rp->dir_option[atoi(chk + 1)]->general_description);
      bc += strlen(rp->dir_option[atoi(chk + 1)]->keyword);
*/
      break;
    case 'E': /* extra description field */

      CREATE(new_descr,struct extra_descr_data,1);
      bc += sizeof(struct extra_descr_data);

      new_descr->keyword = fread_string(fl);
      if (new_descr->keyword && *new_descr->keyword)
	 bc += strlen(new_descr->keyword);
      else
	fprintf(stderr, "No keyword in room %d\n", rp->number);

      new_descr->description = fread_string(fl);
      if (new_descr->description && *new_descr->description)
	 bc += strlen(new_descr->description);
      else
	fprintf(stderr, "No desc in room %d\n", rp->number);

      new_descr->next = rp->ex_description;
      rp->ex_description = new_descr;
      break;
    case 'S':   /* end of current room */

#if BYTE_COUNT
  if (bc >= 1000)
     fprintf(stderr, "Byte count for this room[%d]: %d\n",rp->number,  bc);
#endif
      total_bc += bc;
      room_count++;

/*
 comment start here
      if(IS_SET(rp->room_flags, SAVE_ROOM)) {
	 saved_rooms[number_of_saved_rooms] = rp->number;
	 number_of_saved_rooms++;
      }
*/

      {
       FILE *fp;
       char buf[255];

       sprintf(buf, "world/%d", rp->number);
       fp = fopen(buf, "r");
       if(fp) {
/*
	  saved_rooms[number_of_saved_rooms] = rp->number;
	  number_of_saved_rooms++;
*/
	  fclose(fp);
	}
      }
      return;
    default:
      sprintf(buf,"unknown auxiliary code `%s' in room load of #%d",
	      chk, rp->number);
      log(buf);
      break;
    }
  }
}



/* load the rooms */
void boot_world()
{
  FILE *fl;
  long virtual_nr, last;
  struct room_data      *rp;


#if HASH
  init_hash_table(&room_db, sizeof(struct room_data), 2048);
#else
  init_world(room_db);
#endif
  character_list = 0;
  object_list = 0;

  if (!(fl = fopen(WORLD_FILE, "r")))   {
    perror("fopen");
    log("boot_world: could not open world file.");
    assert(0);
  }




	  last = 0;
  while (1==fscanf(fl, " #%d\n", &virtual_nr)) {
    allocate_room(virtual_nr);
/* do we need to to_of_world++ in here somewhere? msw */
rp = real_roomp(virtual_nr);
	/* lets see what this does, old style allocations msw */
#if 0
    if (rp)
      bzero(rp, sizeof(struct room_data)
      //bzero(rp, sizeof(*rp));
    else {
      fprintf(stderr, "Error, room %d not in database!(%d)\n",
	      virtual_nr, last);
      assert(0);
    }
#else
    memset(rp, 0, sizeof(*rp));
#endif

    rp->number = virtual_nr;
    load_one_room(fl, rp);
    last = virtual_nr;
  }

  fclose(fl);
}





void allocate_room(long room_number)
{
  if (room_number>top_of_world)
    top_of_world = room_number;
#if HASH
  hash_find_or_create(&room_db, room_number);
#else
  room_find_or_create(room_db, room_number);
#endif
}






/* read direction data */
void setup_dir(FILE *fl, long room, int dir)
{
  long tmp;
  struct room_data      *rp, dummy;

  rp = real_roomp(room);

  if (!rp) {
    rp = &dummy;            /* this is a quick fix to make the game */
    dummy.number = room;   /* stop crashing   */
  }

  CREATE(rp->dir_option[dir], struct room_direction_data, 1);

  rp->dir_option[dir]->general_description = fread_string(fl);
  rp->dir_option[dir]->keyword = fread_string(fl);

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->exit_info = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->key = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->to_room = tmp;

  fscanf(fl, " %d ", &tmp);
  rp->dir_option[dir]->open_cmd = tmp;

}

void boot_saved_zones()
{
	DIR *dir;
	FILE *fp;
	struct dirent *ent;
	char    buf[80];
	long     zone;

  if ((dir = opendir("zones")) == NULL) {
    log("Unable to open zones directory.\n");
    return;
  }

  while((ent = readdir(dir)) != NULL) {
    if(*ent->d_name=='.') continue;
    zone=atoi(ent->d_name);
    if(!zone || zone>top_of_zone_table) {
      continue;
    }
    sprintf(buf,"zones/%s",ent->d_name);
    if((fp=fopen(buf,"rt")) == NULL) {
      sprintf(buf, "Can't open file %s for reading\n",buf);
      log(buf);
      continue;
    }
    sprintf(buf,"Loading saved zone %d:%s",zone,zone_table[zone].name);
    log(buf);
    LoadZoneFile(fp,zone);
    fclose(fp);
  }
}

void boot_saved_rooms()
{
	DIR *dir;
	FILE *fp;
	struct dirent *ent;
	char    buf[80];
	struct room_data *rp;
	long     rooms=0,vnum;

  if ((dir = opendir("rooms")) == NULL) {
    log("Unable to open rooms directory.\n");
    return;
  }

  while((ent = readdir(dir)) != NULL) {
    if(*ent->d_name=='.') continue;
    vnum=atoi(ent->d_name);
    if(!vnum || vnum>top_of_world) {
      continue;
    }
    sprintf(buf,"rooms/%s",ent->d_name);
    if((fp=fopen(buf,"rt")) == NULL) {
      sprintf(buf, "Can't open file %s for reading\n",buf);
      log(buf);
      continue;
    }
    while (!feof(fp)) {
      fscanf(fp, "#%*d\n");
      if ((rp=real_roomp(vnum)) == 0) {  /* empty room */
	rp = (void*)malloc(sizeof(struct room_data));
	if (rp)
#if 0
	  bzero(rp, sizeof(struct room_data));
#else
	  memset(rp, 0, sizeof(*rp));
#endif
#if HASH
	room_enter(&room_db, vnum, rp);
#else
	room_enter(room_db, vnum, rp);
#endif
      } else {
	cleanout_room(rp);
      }
      rp->number = vnum;
      load_one_room(fp, rp);
    }
    fclose(fp);
    rooms++;
  }
  if(rooms) {
    sprintf(buf, "Loaded %d rooms",rooms);
    log(buf);
  }

}


#define LOG_ZONE_ERROR(ch, type, zone, cmd) {\
	sprintf(buf, "error in zone %s cmd %d (%c) resolving %s number", \
		zone_table[zone].name, cmd, ch, type); \
		  log(buf); \
		  }

void renum_zone_table(int spec_zone)
{
  long zone, comm,start,end;
  struct reset_com *cmd;
  char  buf[256];

  if(spec_zone) {
    start=end=spec_zone;
  } else {
    start=0;
    end=top_of_zone_table;
  }

  for (zone = start; zone <= end; zone++)
    for (comm = 0; zone_table[zone].cmd[comm].command != 'S'; comm++)
      switch((cmd = zone_table[zone].cmd +comm)->command) {
      case 'M':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1<0)
	  LOG_ZONE_ERROR('M', "mobile", zone, comm);
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('M', "room", zone, comm);
	break;
      case 'C':
	cmd->arg1 = real_mobile(cmd->arg1);
	if (cmd->arg1<0)
	  LOG_ZONE_ERROR('C', "mobile", zone, comm);
	/*cmd->arg3 = real_room(cmd->arg3);*/
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('C', "room", zone, comm);
	break;
      case 'O':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('O', "object", zone, comm);
	if (cmd->arg3 != NOWHERE) {
	  /*cmd->arg3 = real_room(cmd->arg3);*/
	  if(cmd->arg3<0)
	    LOG_ZONE_ERROR('O', "room", zone, comm);
	}
	break;
      case 'G':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('G', "object", zone, comm);
	break;
      case 'E':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('E', "object", zone, comm);
	break;
      case 'P':
	cmd->arg1 = real_object(cmd->arg1);
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	cmd->arg3 = real_object(cmd->arg3);
	if(cmd->arg3<0)
	  LOG_ZONE_ERROR('P', "object", zone, comm);
	break;
      case 'D':
	/*cmd->arg1 = real_room(cmd->arg1);*/
	if(cmd->arg1<0)
	  LOG_ZONE_ERROR('D', "room", zone, comm);
	break;
      }
}


/* load the zone table and command tables */
void boot_zones()
{

  FILE *fl;
  int zon = 0, cmd_no = 0, expand, tmp, bc=100, cc = 22, znumber;
  char *check, buf[81];

  if (!(fl = fopen(ZONE_FILE, "r")))    {
    perror("boot_zones");
    assert(0);
  }

  for (;;)      {
    fscanf(fl, " #%d\n",&znumber);
    check = fread_string(fl);

    if (*check == '$')
      break;            /* end of file */

    /* alloc a new zone */

    if (!zon)
      CREATE(zone_table, struct zone_data, bc);
    else
      if (zon >= bc) {
	if (!(zone_table = (struct zone_data *) realloc(zone_table,
		     (zon + 10) * sizeof(struct zone_data)))){
	  perror("boot_zones realloc");
	  assert(0);
	}
	bc += 10;
      }
    zone_table[zon].num = znumber;
    zone_table[zon].name = check;
    fscanf(fl, " %d ", &zone_table[zon].top);
    fscanf(fl, " %d ", &zone_table[zon].lifespan);
    fscanf(fl, " %d ", &zone_table[zon].reset_mode);

    /* read the command table */

/*
  new code to allow the game to be 'static' i.e. all the mobs are saved
in one big zone file, and restored later.
*/

    cmd_no = 0;

    if (zon == 0)
      cc = 20;

    for (expand = 1;;)          {
      if (expand)
	if (!cmd_no)
	  CREATE(zone_table[zon].cmd, struct reset_com, cc);
	else
	  if (cmd_no >= cc) {
	    cc += 5;
	    if (!(zone_table[zon].cmd =
		  (struct reset_com *) realloc(zone_table[zon].cmd,
			       (cc * sizeof(struct reset_com)))))  {
		perror("reset command load");
		assert(0);
	      }
	  }

      expand = 1;

      fscanf(fl, " "); /* skip blanks */
      fscanf(fl, "%c",
	     &zone_table[zon].cmd[cmd_no].command);

      if (zone_table[zon].cmd[cmd_no].command == 'S')
	break;

      if (zone_table[zon].cmd[cmd_no].command == '*')   {
	expand = 0;
	fgets(buf, 80, fl); /* skip command */
	continue;
      }

      fscanf(fl, " %d %d %d",
	     &tmp,
	     &zone_table[zon].cmd[cmd_no].arg1,
	     &zone_table[zon].cmd[cmd_no].arg2);

      zone_table[zon].cmd[cmd_no].if_flag = tmp;

      if (zone_table[zon].cmd[cmd_no].command == 'M' ||
	  zone_table[zon].cmd[cmd_no].command == 'O' ||
	  zone_table[zon].cmd[cmd_no].command == 'C' ||
	  zone_table[zon].cmd[cmd_no].command == 'E' ||
	  zone_table[zon].cmd[cmd_no].command == 'P' ||
	  zone_table[zon].cmd[cmd_no].command == 'D')
	fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);

      fgets(buf, 80, fl);       /* read comment */
      cmd_no++;
    }
    zon++;
    if (zon == 1) {
      /* fix the cheat */
/*      if (fl != tmp_fl && fl != 0)
	fclose(fl);
      fl = tmp_fl;*/
    }

  }
  top_of_zone_table = --zon;
if (check)
  free(check);
  fclose(fl);
}


/*************************************************************************
*  procedures for resetting, both play-time and boot-time                *
*********************************************************************** */


/* read a mobile from MOB_FILE */
struct char_data *read_mobile(int nr, int type)
{
	FILE *f;
	FILE *newmobfile;
	struct char_data *mob;
	int tmp, i;
	long bc;
	char chk[50], buf[100];

	i = nr;
	if (type == VIRTUAL)
		if ((nr = real_mobile(nr)) < 0) {
			sprintf(buf, "Mobile (V) %d does not exist in database.", i);
			return(0);
		}

	CREATE(mob, struct char_data, 1);

	if (!mob){
		log("Cannot create mob?! db.c read_mobile");
		return(FALSE);
	}

	bc = sizeof(struct char_data);
	clear_char(mob);
	mob->nr = nr;

	if(mob_index[nr].pos == -1) { /* mobile in external file */
		sprintf(buf,"%s/%ld",MOB_DIR,mob_index[nr].virtual);
		if((f = fopen(buf,"rt"))==NULL) {
			sprintf(buf,"can't open mobile file for mob %ld" ,mob_index[nr].virtual);
			log(buf);
			free_char(mob);
			return(0);
		}
		fscanf(f, "#%*ld\n");

#if NEWMOBSTRUCTURE
		bc += read_mob_from_new_file(mob,f);
		fclose(f);
	} else {
		rewind(mob_f);
		fseek(mob_f, mob_index[nr].pos, 0);
		bc += read_mob_from_new_file(mob, mob_f);
#else
		bc += read_mob_from_file(mob,f);
		fclose(f);
	} else {
		rewind(mob_f);
		fseek(mob_f, mob_index[nr].pos, 0);
		bc += read_mob_from_file(mob, mob_f);
#endif

    }

	total_mbc += bc;
	mob_count++;
	/* assign common proc flags */
	if(mob->specials.proc != 0) {
		if(mob->specials.proc == PROC_QUEST) {
			if(mob_index[mob->nr].func != QuestMobProc){
				mob_index[mob->nr].func = *QuestMobProc;
				log("assigning quest proc");
			}
		} else if (mob->specials.proc == PROC_SHOPKEEPER) {
			if(mob_index[mob->nr].func != shopkeeper){
				mob_index[mob->nr].func = *shopkeeper;
				log("assigning shopkeeper proc");
			}
		} else if (mob->specials.proc == PROC_GUILDMASTER) {
			if(mob_index[mob->nr].func != generic_guildmaster){
				mob_index[mob->nr].func = *generic_guildmaster;
				log("assigning guildmaster proc");
			}
		} else if (mob->specials.proc == PROC_SWALLOWER) {
			if(mob_index[mob->nr].func != Tyrannosaurus_swallower){
				mob_index[mob->nr].func = *Tyrannosaurus_swallower;
				log("assigning swallower proc");
			}
		} else if (mob->specials.proc == PROC_OLD_BREATH) {
			if(mob_index[mob->nr].func != BreathWeapon){
				mob_index[mob->nr].func = *BreathWeapon;
				log("assigning old breath proc");
			}
		} else if (mob->specials.proc == PROC_FIRE_BREATH) {
			if(mob_index[mob->nr].func != FireBreather){
				mob_index[mob->nr].func = *FireBreather;
				log("assigning fire breath proc");
			}
		} else if (mob->specials.proc == PROC_GAS_BREATH) {
			if(mob_index[mob->nr].func != GasBreather){
				mob_index[mob->nr].func = *GasBreather;
				log("assigning gas breath proc");
			}
		} else if (mob->specials.proc == PROC_FROST_BREATH) {
			if(mob_index[mob->nr].func != FrostBreather){
				mob_index[mob->nr].func = *FrostBreather;
				log("assigning frost breath proc");
			}
		} else if (mob->specials.proc == PROC_ACID_BREATH) {
			if(mob_index[mob->nr].func != AcidBreather){
				mob_index[mob->nr].func = *AcidBreather;
				log("assigning acid breath proc");
			}
		} else if (mob->specials.proc == PROC_LIGHTNING_BREATH) {
			if(mob_index[mob->nr].func != LightningBreather){
				mob_index[mob->nr].func = *LightningBreather;
				log("assigning electric breath proc");
			}
		} else if (mob->specials.proc == PROC_DEHYDRATION_BREATH) {
			if(mob_index[mob->nr].func != DehydBreather){
				mob_index[mob->nr].func = *DehydBreather;
				log("assigning dehydration breath proc");
			}
		} else if (mob->specials.proc == PROC_VAPOR_BREATH) {
			if(mob_index[mob->nr].func != VaporBreather){
				mob_index[mob->nr].func = *VaporBreather;
				log("assigning vapor breath proc");
			}
		} else if (mob->specials.proc == PROC_SOUND_BREATH) {
			if(mob_index[mob->nr].func != SoundBreather){
				mob_index[mob->nr].func = *SoundBreather;
				log("assigning sound breath proc");
			}
		} else if (mob->specials.proc == PROC_SHARD_BREATH) {
			if(mob_index[mob->nr].func != ShardBreather){
				mob_index[mob->nr].func = *ShardBreather;
				log("assigning shard breath proc");
			}
		} else if (mob->specials.proc == PROC_SLEEP_BREATH) {
			if(mob_index[mob->nr].func != SleepBreather){
				mob_index[mob->nr].func = *SleepBreather;
				log("assigning sleep breath proc");
			}
		} else if (mob->specials.proc == PROC_LIGHT_BREATH) {
			if(mob_index[mob->nr].func != LightBreather){
				mob_index[mob->nr].func = *LightBreather;
				log("assigning light breath proc");
			}
		} else if (mob->specials.proc == PROC_DARK_BREATH) {
			if(mob_index[mob->nr].func != DarkBreather){
				mob_index[mob->nr].func = *DarkBreather;
				log("assigning dark breath proc");
			}
		} else if (mob->specials.proc == PROC_RECEPTIONIST) {
			if(mob_index[mob->nr].func != receptionist) {
				mob_index[mob->nr].func = *receptionist;
				log("assigning receptionist proc");
			}
			if (!IS_SET(mob->specials.act, ACT_SENTINEL))
				SET_BIT(mob->specials.act, ACT_SENTINEL);
		} else if (mob->specials.proc == PROC_REPAIRGUY) {
			if(mob_index[mob->nr].func != RepairGuy) {
				mob_index[mob->nr].func = *RepairGuy;
				log("assigning repair guy proc");
			}
		}
	}
	return(mob);
}

int read_mob_from_file(struct char_data *mob, FILE *mob_fi)
{
  int i,nr;
  long tmp, tmp2, tmp3, bc = 0;
  char buf[100], buffer[255];
  char letter;

  extern int mob_tick_count;
  extern long mob_count;

  HpBonus=0;

  nr = mob->nr;
  mob->player.name = fread_string(mob_fi);
  if (*mob->player.name)
    bc += strlen(mob->player.name);
  mob->player.short_descr = fread_string(mob_fi);
  if (*mob->player.short_descr)
    bc += strlen(mob->player.short_descr);
  mob->player.long_descr = fread_string(mob_fi);
  if (*mob->player.long_descr)
    bc += strlen(mob->player.long_descr);
  mob->player.description = fread_string(mob_fi);
  if (mob->player.description && *mob->player.description)
    bc += strlen(mob->player.description);
  mob->player.title = 0;

  /* *** Numeric data *** */

  mob->mult_att = 1.0;
  mob->specials.spellfail = 101;

  fscanf(mob_fi, "%d ", &tmp);
  mob->specials.act = tmp;
  SET_BIT(mob->specials.act, ACT_ISNPC);

  fscanf(mob_fi, " %d ", &tmp);
  mob->specials.affected_by = tmp;

  fscanf(mob_fi, " %d ", &tmp);
  mob->specials.alignment = tmp;

  tmp=0;
  tmp2=0;
  tmp3=0;

  mob->player.class = CLASS_WARRIOR;

  fscanf(mob_fi, " %c ", &letter);

	if (letter == 'S') {

		fscanf(mob_fi, "\n");

		fscanf(mob_fi, " %d ", &tmp);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.intel =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.wis   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.dex   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.con   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.chr   =  9+number(1,(MAX(1,tmp/5 - 1)));

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.hitroll = 20-tmp;

		fscanf(mob_fi, " %d ", &tmp);

		if (tmp > 10 || tmp < -10)
			tmp /= 10;
		mob->points.armor = 10*tmp;

		fscanf(mob_fi, " %dd%d+%d ", &tmp, &tmp2, &tmp3);
		mob->points.max_hit = dice(tmp, tmp2)+tmp3;
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_fi, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.mana = 10;
		mob->points.max_mana = 10;


		mob->points.move = 50;
		mob->points.max_move = 50;

		fscanf(mob_fi, " %d ", &tmp);
		if (tmp == -1) {
			fscanf(mob_fi, " %d ", &tmp);
			mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);

			fscanf(mob_fi, " %d ", &tmp);
			tmp = MAX(tmp,GET_LEVEL(mob, WARRIOR_LEVEL_IND)*mob->points.max_hit);
			GET_EXP(mob) = tmp;

			fscanf(mob_fi, " %d \n", &tmp);
			GET_RACE(mob) = tmp;
			if(IsGiant(mob))
				mob->abilities.str += number(1,4);
			if(IsSmall(mob))
				mob->abilities.str -= 1;
		} else {
			mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);//tmp;

			fscanf(mob_fi, " %d \n", &tmp);
			if (tmp >= 0)
				GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
			else
				GET_EXP(mob) = -tmp;
			GET_EXP(mob) = tmp;
		}

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		if (tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		} else if (tmp < 6) {
			mob->player.sex = (tmp-3);

			fscanf(mob_fi, " %d ", &tmp);
			mob->immune = tmp;

			fscanf(mob_fi, " %d ", &tmp);
			mob->M_immune = tmp;

			fscanf(mob_fi, " %d ", &tmp);
			mob->susc = tmp;
		} else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

		fscanf(mob_fi,"\n");

		mob->player.class = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played     = 0;
		mob->player.time.logon  = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for (i = 0; i < 3; i++)
			GET_COND(mob, i) = -1;

		for (i = 0; i < MAX_SAVES; i++)
			mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);

	} else if ((letter == 'A') || (letter == 'N') || (letter == 'B') || (letter == 'L')) {

		if ((letter == 'A') || (letter == 'B') || (letter == 'L')) {
			fscanf(mob_fi, " %d ", &tmp);
			mob->mult_att = (float)tmp;
		}

		fscanf(mob_fi, "\n");

		fscanf(mob_fi, " %d ", &tmp);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		mob->abilities.str   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.intel =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.wis   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.dex   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.con   =  9+number(1,(MAX(1,tmp/5 - 1)));
		mob->abilities.chr   =  9+number(1,(MAX(1,tmp/5 - 1)));

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.hitroll = 20-tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.armor = 10*tmp;

		fscanf(mob_fi, " %d ", &tmp);
//		HpBonus = tmp;
		mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
		mob->points.hit = mob->points.max_hit;
HpBonus = mob->points.max_hit;

		fscanf(mob_fi, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
		mob->points.damroll = tmp3;
		mob->specials.damnodice = tmp;
		mob->specials.damsizedice = tmp2;

		mob->points.mana = 10;
		mob->points.max_mana = 10;

		mob->points.move = 50;
		mob->points.max_move = 50;

		fscanf(mob_fi, " %d ", &tmp);

		if (tmp == -1) {
			fscanf(mob_fi, " %d ", &tmp);
			mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);//tmp;

			fscanf(mob_fi, " %d ", &tmp);
			if (tmp >= 0)
				GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
			else
				GET_EXP(mob) = -tmp;

			fscanf(mob_fi, " %d ", &tmp);
			GET_RACE(mob) = tmp;

			if(IsGiant(mob))
				mob->abilities.str += number(1,4);
			if(IsSmall(mob))
				mob->abilities.str -= 1;
		} else {
			mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);//tmp;

      /*
	this is where the new exp will come into play
	*/
			fscanf(mob_fi, " %d \n", &tmp);
			if (tmp >=0)
				GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
			else
				GET_EXP(mob) = -tmp;
		}

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_fi, " %d \n", &tmp);
		if (tmp < 3) {
			mob->player.sex = tmp;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		} else if (tmp < 6) {
			mob->player.sex = (tmp-3);

			fscanf(mob_fi, " %d ", &tmp);
			mob->immune = tmp;

			fscanf(mob_fi, " %d ", &tmp);
			mob->M_immune = tmp;

			fscanf(mob_fi, " %d ", &tmp);
			mob->susc = tmp;
		} else {
			mob->player.sex = 0;
			mob->immune = 0;
			mob->M_immune = 0;
			mob->susc = 0;
		}

    /*
     *   read in the sound string for a mobile
     */

		if (letter == 'L') {
			mob->player.sounds = fread_string(mob_fi);
			if (mob->player.sounds && *mob->player.sounds)
				bc += strlen(mob->player.sounds);

			mob->player.distant_snds = fread_string(mob_fi);
			if (mob->player.distant_snds && *mob->player.distant_snds)
				bc += strlen(mob->player.distant_snds);
		} else {
			mob->player.sounds = 0;
			mob->player.distant_snds = 0;
		}

		if (letter == 'B') {
			SET_BIT(mob->specials.act, ACT_HUGE);
		}

		mob->player.class = 0;

		mob->player.time.birth = time(0);
		mob->player.time.played     = 0;
		mob->player.time.logon  = time(0);
		mob->player.weight = 200;
		mob->player.height = 198;

		for (i = 0; i < 3; i++)
			GET_COND(mob, i) = -1;

		for (i = 0; i < MAX_SAVES; i++)
			mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);

	} else {  /* The old monsters are down below here */

		fscanf(mob_fi, "\n");

		fscanf(mob_fi, " %d ", &tmp);
		mob->abilities.str = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->abilities.intel = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->abilities.wis = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->abilities.dex = tmp;

		fscanf(mob_fi, " %d \n", &tmp);
		mob->abilities.con = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		fscanf(mob_fi, " %d ", &tmp2);

		mob->points.max_hit = number(tmp, tmp2);
		mob->points.hit = mob->points.max_hit;

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.armor = 10*tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.mana = tmp;
		mob->points.max_mana = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.move = tmp;
		mob->points.max_move = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);//tmp;

		fscanf(mob_fi, " %d \n", &tmp);
		if (tmp >= 0)
			GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);
		else
			GET_EXP(mob) = -tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.position = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->specials.default_pos = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->player.sex = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->player.class = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

		fscanf(mob_fi, " %d ", &tmp);
		mob->player.time.birth = time(0);
		mob->player.time.played     = 0;
		mob->player.time.logon  = time(0);

		fscanf(mob_fi, " %d ", &tmp);
		mob->player.weight = tmp;

		fscanf(mob_fi, " %d \n", &tmp);
		mob->player.height = tmp;

		for (i = 0; i < 3; i++) {
			fscanf(mob_fi, " %d ", &tmp);
			GET_COND(mob, i) = tmp;
		}
		fscanf(mob_fi, " \n ");

		for (i = 0; i < MAX_SAVES; i++) {
			fscanf(mob_fi, " %d ", &tmp);
			mob->specials.apply_saving_throw[i] = tmp;
		}

		fscanf(mob_fi, " \n ");

		/* Set the damage as some standard 1d4 */
		mob->points.damroll = 0;
		mob->specials.damnodice = 1;
		mob->specials.damsizedice = 6;

		/* Calculate THAC0 as a formular of Level */
		mob->points.hitroll = MAX(1, GET_LEVEL(mob,WARRIOR_LEVEL_IND)-3);
	}

	mob->tmpabilities = mob->abilities;

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = 0;

	mob->desc = 0;

	if (!IS_SET(mob->specials.act, ACT_ISNPC))
		SET_BIT(mob->specials.act, ACT_ISNPC);

	mob->generic = 0;
	mob->commandp = 0;
	mob->commandp2 = 0;
	mob->waitp = 0;

	/* Check to see if associated with a script, if so, set it up */
	if(IS_SET(mob->specials.act, ACT_SCRIPT))
		REMOVE_BIT(mob->specials.act, ACT_SCRIPT);

	for(i = 0; i < top_of_scripts; i++) {
		if(script_data[i].virtual == mob_index[nr].virtual) {
			SET_BIT(mob->specials.act, ACT_SCRIPT);
			mob->script = i;
			break;
		}
	}
	/* tell the spec_proc (if there is one) that we've been born */
	/* insert in list */

	mob->next = character_list;
	character_list = mob;

#if LOW_GOLD
	if (mob->points.gold >= 50)
		mob->points.gold /= 5;
	else if (mob->points.gold < 10)
		mob->points.gold = 0;
#endif

/* Meh this creates such a lot of spam
	if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*1500) {
		char buf[200];
		sprintf(buf, "%s has gold > level * 1500 (%d)", mob->player.short_descr, mob->points.gold);
		log(buf);
	}
*/
	/* set up things that all members of the race have */
	SetRacialStuff(mob);

	/* change exp for wimpy mobs (lower) */
	if (IS_SET(mob->specials.act, ACT_WIMPY))
		GET_EXP(mob) -= GET_EXP(mob)/10;

	/* change exp for agressive mobs (higher) */
	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		GET_EXP(mob) += GET_EXP(mob)/10;
		/* big bonus for fully aggressive mobs for now */
		if (!IS_SET(mob->specials.act, ACT_WIMPY)|| IS_SET(mob->specials.act, ACT_META_AGG))
			GET_EXP(mob) += (GET_EXP(mob)/2);
	}

	/* set up distributed movement system */
	mob->specials.tick = mob_tick_count++;

	if (mob_tick_count == TICK_WRAP_COUNT)
		mob_tick_count=0;

	mob_index[nr].number++;

#if BYTE_COUNT
	fprintf(stderr,"Mobile [%d]: byte count: %d\n", mob_index[nr].virtual, bc);
#endif
	return(bc);
}

int read_mob_from_new_file(struct char_data *mob, FILE *mob_fi)
{
  int i,nr;
  long tmp, tmp2, tmp3, bc = 0;
  char buf[100], buffer[255];
  char letter;
  int HpBonus;
  float att;

  extern int mob_tick_count;
  extern long mob_count;

  HpBonus=0;

  nr = mob->nr;

	mob->player.name = fread_string(mob_fi);
	if (*mob->player.name)
		bc += strlen(mob->player.name);
	mob->player.short_descr = fread_string(mob_fi);
	if (*mob->player.short_descr)
		bc += strlen(mob->player.short_descr);
	mob->player.long_descr = fread_string(mob_fi);
	if (*mob->player.long_descr)
		bc += strlen(mob->player.long_descr);
	mob->player.description = fread_string(mob_fi);
	if (mob->player.description && *mob->player.description)
		bc += strlen(mob->player.description);

	mob->player.sounds = fread_string(mob_fi);
	if (mob->player.sounds && *mob->player.sounds)
		bc += strlen(mob->player.sounds);
	mob->player.distant_snds = fread_string(mob_fi);
	if (mob->player.distant_snds && *mob->player.distant_snds)
		bc += strlen(mob->player.distant_snds);


	mob->player.title = 0;

	/* *** Numeric data *** */
	mob->specials.spellfail = 101;

	fscanf(mob_fi, "%d ", &tmp);
	mob->specials.act = tmp;
	SET_BIT(mob->specials.act, ACT_ISNPC);

	fscanf(mob_fi, " %d ", &tmp);
	mob->specials.affected_by = tmp;

	fscanf(mob_fi, " %d ", &tmp);
	mob->specials.alignment = tmp;

	fscanf(mob_fi, " %f \n", &att);
	mob->mult_att = att;

	fscanf(mob_fi, " %d ", &tmp);
	GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

	mob->abilities.str   =  9+number(1,(MAX(1,tmp/5 - 1)));
	mob->abilities.intel =  9+number(1,(MAX(1,tmp/5 - 1)));
	mob->abilities.wis   =  9+number(1,(MAX(1,tmp/5 - 1)));
	mob->abilities.dex   =  9+number(1,(MAX(1,tmp/5 - 1)));
	mob->abilities.con   =  9+number(1,(MAX(1,tmp/5 - 1)));
	mob->abilities.chr   =  9+number(1,(MAX(1,tmp/5 - 1)));

	fscanf(mob_fi, " %d ", &tmp);
	mob->points.hitroll = 20-tmp;

	fscanf(mob_fi, " %d ", &tmp);
	if (tmp > 10 || tmp < -10)
		tmp /= 10;
	mob->points.armor = 10*tmp;

	fscanf(mob_fi, " %d ", &tmp);
	mob->points.max_hit = tmp;
	mob->points.hit = tmp;
//	mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
//	mob->points.hit = mob->points.max_hit;

	fscanf(mob_fi, " %dd%d+%d \n", &tmp, &tmp2, &tmp3);
	mob->points.damroll = tmp3;
	mob->specials.damnodice = tmp;
	mob->specials.damsizedice = tmp2;

	mob->points.mana = 10;
	mob->points.max_mana = 10;

	mob->points.move = 50;
	mob->points.max_move = 50;


	fscanf(mob_fi, " -1 ", &tmp);

	fscanf(mob_fi, " %d ", &tmp);
	mob->points.gold = (int) 1 * (float)((number(900,1100)*tmp)/1000);

	fscanf(mob_fi, " %d ", &tmp);
	if (tmp < 1)
		tmp = 1;
	GET_EXP(mob) = (DetermineExp(mob, tmp)+mob->points.gold);

	fscanf(mob_fi, " %d \n", &tmp);
	GET_RACE(mob) = tmp;

	if(IsGiant(mob))
		mob->abilities.str += number(1,4);
	if(IsSmall(mob))
		mob->abilities.str -= 1;

	fscanf(mob_fi, " %d ", &tmp);
	mob->specials.position = tmp;
	fscanf(mob_fi, " %d ", &tmp);
	mob->specials.default_pos = tmp;

	fscanf(mob_fi, " %d ", &tmp);
	mob->player.sex = tmp;

	fscanf(mob_fi, " %d ", &tmp);
	mob->immune = tmp;

	fscanf(mob_fi, " %d ", &tmp);
	mob->M_immune = tmp;

	fscanf(mob_fi, " %d \n", &tmp);
	mob->susc = tmp;

	fscanf(mob_fi, " %d \n", &tmp);
	mob->specials.proc = tmp;

	mob->specials.talks = fread_string(mob_fi);
	if (mob->specials.talks && *mob->specials.talks)
		bc += strlen(mob->specials.talks);
	mob->specials.quest_yes = fread_string(mob_fi);
	if (mob->specials.quest_yes && *mob->specials.quest_yes)
		bc += strlen(mob->specials.quest_yes);
	mob->specials.quest_no = fread_string(mob_fi);
	if (mob->specials.quest_no && *mob->specials.quest_no)
		bc += strlen(mob->specials.quest_no);



	mob->player.class = 0;

	mob->player.time.birth = time(0);
	mob->player.time.played     = 0;
	mob->player.time.logon  = time(0);
	mob->player.weight = 200;
	mob->player.height = 198;

	for (i = 0; i < 3; i++)
		GET_COND(mob, i) = -1;

	for (i = 0; i < MAX_SAVES; i++)
		mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob, WARRIOR_LEVEL_IND), 2);

	mob->tmpabilities = mob->abilities;

	for (i = 0; i < MAX_WEAR; i++) /* Initialisering Ok */
		mob->equipment[i] = 0;

	mob->desc = 0;

	if (!IS_SET(mob->specials.act, ACT_ISNPC))
		SET_BIT(mob->specials.act, ACT_ISNPC);

	mob->generic = 0;
	mob->commandp = 0;
	mob->commandp2 = 0;
	mob->waitp = 0;

	/* Check to see if associated with a script, if so, set it up */
	if(IS_SET(mob->specials.act, ACT_SCRIPT))
		REMOVE_BIT(mob->specials.act, ACT_SCRIPT);

	for(i = 0; i < top_of_scripts; i++) {
		if(script_data[i].virtual == mob_index[nr].virtual) {
			SET_BIT(mob->specials.act, ACT_SCRIPT);
			mob->script = i;
			break;
		}
	}
	/* tell the spec_proc (if there is one) that we've been born */
	/* insert in list */

	mob->next = character_list;
	character_list = mob;

/* Meh this creates such a lot of spam
	if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*1500) {
		char buf[200];
		sprintf(buf, "%s has gold > level * 1500 (%d)", mob->player.short_descr, mob->points.gold);
		log(buf);
	}
*/
	/* set up things that all members of the race have */
	SetRacialStuff(mob);

	/* change exp for wimpy mobs (lower) */
	if (IS_SET(mob->specials.act, ACT_WIMPY))
		GET_EXP(mob) -= GET_EXP(mob)/10;

	/* change exp for agressive mobs (higher) */
	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		GET_EXP(mob) += GET_EXP(mob)/10;
		/* big bonus for fully aggressive mobs for now */
		if (!IS_SET(mob->specials.act, ACT_WIMPY)|| IS_SET(mob->specials.act, ACT_META_AGG))
			GET_EXP(mob) += (GET_EXP(mob)/2);
	}

	/* set up distributed movement system */
	mob->specials.tick = mob_tick_count++;

	if (mob_tick_count == TICK_WRAP_COUNT)
		mob_tick_count=0;

	mob_index[nr].number++;

#if BYTE_COUNT
	fprintf(stderr,"Mobile [%d]: byte count: %d\n", mob_index[nr].virtual, bc);
#endif
	return(bc);
}


int clone_obj_to_obj(struct obj_data *obj, struct obj_data *osrc)
{
	struct extra_descr_data *new_descr, *tmp_descr;
	int     i;

  if(osrc->name)
    obj->name = strdup(osrc->name);
  if(osrc->short_description)
    obj->short_description = strdup(osrc->short_description);
  if(osrc->description)
    obj->description = strdup(osrc->description);
  if(osrc->action_description)
    obj->action_description = strdup(osrc->action_description);

    /* *** numeric data *** */

  obj->obj_flags.type_flag      = osrc->obj_flags.type_flag;
  obj->obj_flags.extra_flags    = osrc->obj_flags.extra_flags;
  obj->obj_flags.wear_flags     = osrc->obj_flags.wear_flags;
  obj->obj_flags.value[0]       = osrc->obj_flags.value[0];
  obj->obj_flags.value[1]       = osrc->obj_flags.value[1];
  obj->obj_flags.value[2]       = osrc->obj_flags.value[2];
  obj->obj_flags.value[3]       = osrc->obj_flags.value[3];
  obj->obj_flags.weight         = osrc->obj_flags.weight;
  obj->obj_flags.cost           = osrc->obj_flags.cost;
  obj->obj_flags.cost_per_day   = osrc->obj_flags.cost_per_day;

  /* *** extra descriptions *** */

  obj->ex_description = 0;

  if(osrc->ex_description)
    for(tmp_descr=osrc->ex_description;tmp_descr;tmp_descr=tmp_descr->next) {
      CREATE(new_descr, struct extra_descr_data, 1);
      if(tmp_descr->keyword)
      new_descr->keyword = strdup(tmp_descr->keyword);
      if(tmp_descr->description)
      new_descr->description = strdup(tmp_descr->description);
      new_descr->next = obj->ex_description;
      obj->ex_description = new_descr;
    }

    for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)     {
      obj->affected[i].location = osrc->affected[i].location;
      obj->affected[i].modifier = osrc->affected[i].modifier;
    }
}

#define NEWSETUP 1  //used for old style dale objects
int read_obj_from_file(struct obj_data *obj, FILE *f)
{

	int     i,tmp;
	long    bc,ltmp;
	char    chk[50];
	struct extra_descr_data *new_descr;

    obj->name = fread_string(f);
    if (obj->name && *obj->name) {
      bc += strlen(obj->name);
    }
    obj->short_description = fread_string(f);
    if (obj->short_description && *obj->short_description) {
      bc += strlen(obj->short_description);
    }
    obj->description = fread_string(f);
    if (obj->description && *obj->description) {
      bc += strlen(obj->description);
    }
    obj->action_description = fread_string(f);
    if (obj->action_description && *obj->action_description) {
      bc += strlen(obj->action_description);
    }

	/*New object crap(GH)*/
    obj->modBy = fread_string(f);
    if (obj->modBy && *obj->modBy) {
      bc += strlen(obj->modBy);
    }

    /* *** numeric data *** */

    fscanf(f, " %d ", &tmp);
    obj->obj_flags.type_flag = tmp;
    fscanf(f, " %ld ", &ltmp);
    obj->obj_flags.extra_flags = ltmp;
    fscanf(f, " %ld ", &ltmp);
    obj->obj_flags.wear_flags = ltmp;
    fscanf(f, " %d ", &tmp);
    obj->obj_flags.value[0] = tmp;
    fscanf(f, " %d ", &tmp);
    obj->obj_flags.value[1] = tmp;
    fscanf(f, " %d ", &tmp);
    obj->obj_flags.value[2] = tmp;
    fscanf(f, " %d ", &tmp);
    obj->obj_flags.value[3] = tmp;
    fscanf(f, " %d ", &tmp);
    obj->obj_flags.weight = tmp;
    fscanf(f, " %d \n", &tmp);
    obj->obj_flags.cost = tmp;
    fscanf(f, " %d \n", &tmp);
    obj->obj_flags.cost_per_day = tmp;
	/*New fields (GH)*/


    fscanf(f, " %d \n", &tmp);
    obj->level = tmp;
    fscanf(f, " %d \n", &tmp);
    obj->max = tmp;
	fscanf(f, " %ld \n", &tmp);
	obj->modified = tmp;

#if NEWSETUP
	fscanf(f, " %d \n", &tmp);
    obj->speed = tmp;

	fscanf(f, " %d \n", &tmp);
	obj->weapontype = tmp;

#endif

	/* Let's see if this works, Lennya */
	fscanf(f, " %d \n", &tmp);
	obj->tweak = tmp;


/* *** extra descriptions *** */

    obj->ex_description = 0;

    while (fscanf(f, " %s \n", chk), *chk == 'E')     {
      CREATE(new_descr, struct extra_descr_data, 1);
      bc += sizeof(struct extra_descr_data);
      new_descr->keyword = fread_string(f);
      if (new_descr->keyword && *new_descr->keyword)
	bc += strlen(new_descr->keyword);
      new_descr->description = fread_string(f);
      if (new_descr->description && *new_descr->description)
	bc += strlen(new_descr->description);

      new_descr->next = obj->ex_description;
      obj->ex_description = new_descr;
      chk[0] ='\0';
    }

    for( i = 0 ; (i < MAX_OBJ_AFFECT) && (*chk == 'A') ; i++)     {
      *chk= 'x';       // inserted to fix bug with load of object in individual file
      fscanf(f, " %d ", &tmp);
      obj->affected[i].location = tmp;
      fscanf(f, " %d \n", &tmp);
      obj->affected[i].modifier = tmp;
      fscanf(f, " %s \n", chk);
    }

    for (;(i < MAX_OBJ_AFFECT);i++)       {
      obj->affected[i].location = APPLY_NONE;
      obj->affected[i].modifier = 0;
    }
  return bc;
}

/* ---------- Start of change_cr_to_nl) ---------- */
/* change each occurence of a Carriage Return by a New Line in a buffer */
void remove_cr(char *output, char *input)
{
   int i,j,k;

   output[0] = '\0';
   j=strlen(input);
   for (i=0,k=0;i<j;i++)
      {
	if (input[i] == '\r') k++;   // indicate one char skip
	else output[i-k] = input [i];
      }
      output[i-k] = '\0';
 }

/* ---------- Start of write_mob_to_file ---------- */
/* write a mobile to a file */


 /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                          *
 *  Redone By Lennya, 20030802                                              *
 *  Mob files will now have this format:                                    *
 *                                                                          *
 *  #vnum                                                                   *
 *  name~                                                                   *
 *  short description~                                                      *
 *  long description~                                                       *
 *  description                                                             *
 *  ~                                                                       *
 *  local sound                                                             *
 *  ~                                                                       *
 *  distant sound                                                           *
 *  ~                                                                       *
 *  ActionFlags  AffectFlags  Alignment  NumAttacks                         *
 *  Level  HitRoll  ArmorClass/10  TotalHp  Damage(Example: 1d8+2)          *
 *  -1  Gold  ExpFlag  Race                                                 *
 *  Position  DefaultPosition  Sex  Immunities  Resistance  Susceptibilies  *
 *  CommonProcedure                                                         *
 *  talk string                                                             *
 *  ~                                                                       *
 *  quest solved string                                                     *
 *  ~                                                                       *
 *  wrong quest item string                                                 *
 *  ~                                                                       *
 *                                                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void write_mob_to_file(struct char_data *mob, FILE *mob_fi)//,int hpB)
{
#if NEWMOBSTRUCTURE
	long    bc,ltmp;
	int i, xpflag;
	long tmp, tmp2, tmp3;
	float tmpexp;
	char letter;
	char buff[MAX_STRING_LENGTH];

	/***** String data *** */
	fwrite_string(mob_fi,mob->player.name);
	fwrite_string(mob_fi,mob->player.short_descr);
	remove_cr(buff,mob->player.long_descr);  // remove carriage return
	fwrite_string(mob_fi,buff);
	remove_cr(buff,mob->player.description);
	fwrite_string(mob_fi,buff);
	if(mob->player.sounds) {
		remove_cr(buff,mob->player.sounds);  // remove carriage return
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->player.distant_snds) {
		remove_cr(buff,mob->player.distant_snds);  // remove carriage return
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}

	/* *** Numeric data *** */
	tmp = mob->specials.act;    	// get actions flags
	REMOVE_BIT(tmp, ACT_ISNPC);   	// remove flag IS NPC
	REMOVE_BIT(tmp, ACT_HATEFUL);   // remove flag HATEFUL, these are of no import
	REMOVE_BIT(tmp, ACT_GUARDIAN);  // remove flag GUARDIAN
	REMOVE_BIT(tmp, ACT_HUNTING);	// remove flag HUNTING
	REMOVE_BIT(tmp, ACT_AFRAID);	// remove flag AFRAID

	fprintf(mob_fi, "%d ", tmp);    // write action flags
	fprintf(mob_fi, " %d ", mob->specials.affected_by);
	fprintf(mob_fi, " %d ",mob->specials.alignment );
	fprintf(mob_fi, " %.1f \n",mob->mult_att );

	fprintf(mob_fi, " %d ",GET_LEVEL(mob, WARRIOR_LEVEL_IND) );
	fprintf(mob_fi, " %d ",(mob->points.hitroll-20)*-1 );
	fprintf(mob_fi, " %d ",mob->points.armor/10 );
	fprintf(mob_fi, " %d ", mob->points.max_hit);
	fprintf(mob_fi, " %dd%d+%d \n", mob->specials.damnodice, mob->specials.damsizedice, mob->points.damroll);

	tmpexp = GET_EXP(mob);    // convert Exp to float for calculation
	/* revert exp for agressive	 mobs (lower) */
	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		/* big bonus for fully aggressive mobs for now */
		if (!IS_SET(mob->specials.act, ACT_WIMPY) || IS_SET(mob->specials.act, ACT_META_AGG))
			tmpexp = tmpexp / 1.5;
		tmpexp = tmpexp / 1.1;
	}
	/* revert exp for wimpy mobs (higher) */
	if (IS_SET(mob->specials.act, ACT_WIMPY))
		tmpexp = tmpexp/ 0.9;
	i = (int) tmpexp;   // reconvert to long after calculation

	fprintf(mob_fi, " %d ", -1);
	fprintf(mob_fi, " %d ", mob->points.gold);

	xpflag = GetExpFlags(mob, i); /* make sure this mob gets an xp flag from 1 - 31 */
	if(xpflag < 1)
		xpflag = 1;
	if(xpflag > 31)
		xpflag = 31;

	fprintf(mob_fi, " %d ", xpflag);
	fprintf(mob_fi, " %d \n", GET_RACE(mob));

	fprintf(mob_fi, " %d ",mob->specials.position );
	fprintf(mob_fi, " %d ", mob->specials.default_pos);

	fprintf(mob_fi, " %d ",mob->player.sex);
	fprintf(mob_fi, " %d ",mob->immune);
	fprintf(mob_fi, " %d ",mob->M_immune);
	fprintf(mob_fi, " %d \n", mob->susc);

	fprintf(mob_fi, " %d \n", mob->specials.proc);

	if(mob->specials.talks) {
		remove_cr(buff,mob->specials.talks);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->specials.quest_yes) {
		remove_cr(buff,mob->specials.quest_yes);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->specials.quest_no) {
		remove_cr(buff,mob->specials.quest_no);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}


#else

  int i;
  long tmp, tmp2, tmp3;
  float tmpexp;
  char letter;
  char buff[MAX_STRING_LENGTH];
  /***** String data *** */

  fwrite_string(mob_fi,mob->player.name);
  fwrite_string(mob_fi,mob->player.short_descr);
  remove_cr(buff,mob->player.long_descr);  // remove carriage return
  fwrite_string(mob_fi,buff);
  remove_cr(buff,mob->player.description);
  fwrite_string(mob_fi,buff);

  /* *** Numeric data *** */

  tmp = mob->specials.act;    // get actions flags
  REMOVE_BIT(tmp, ACT_ISNPC);   // remove flag IS NPC
  REMOVE_BIT(tmp, ACT_HATEFUL);   // remove flag  HATEFUL
  REMOVE_BIT(tmp, ACT_GUARDIAN);   // remove flag  GUARDIAN
  REMOVE_BIT(tmp, ACT_HUNTING);   // remove flag HUMTING
  REMOVE_BIT(tmp, ACT_AFRAID);   // remove flag AFRAID
  if (IS_SET(mob->specials.act, ACT_HUGE)) REMOVE_BIT(tmp, ACT_HUGE);

  fprintf(mob_fi, "%d ", tmp);    // write actions flaf

  fprintf(mob_fi, " %d ", mob->specials.affected_by);

  fprintf(mob_fi, " %d ",mob->specials.alignment );

  letter= 'N';
  if ((mob->player.sounds != 0) && (mob->player.distant_snds != 0)) letter= 'L';
  if (mob->mult_att > 1) letter = 'A';
  if (IS_SET(mob->specials.act, ACT_HUGE)) letter = 'B';
  fprintf(mob_fi,"%c",letter);

  if ((letter == 'A') || (letter == 'B') || (letter == 'L')) {
      tmp = (int) mob->mult_att;
      fprintf(mob_fi, " %d ",tmp );

	}
    fprintf(mob_fi, "\n");

    fprintf(mob_fi, " %d ",GET_LEVEL(mob, WARRIOR_LEVEL_IND) );


    fprintf(mob_fi, " %d ",(mob->points.hitroll-20)*-1 );

    fprintf(mob_fi, " %d ",mob->points.armor/10 );
    fprintf(mob_fi, " %d ", 0); /* set hp bonus to 0.. should never pass this code anymore anyway */

    fprintf(mob_fi, " %dd%d+%d \n",mob->specials.damnodice
				  ,mob->specials.damsizedice
				  , mob->points.damroll);


    tmpexp = GET_EXP(mob);    // convert Exp to float for calculation
    /* revert exp for agressive mobs (lower) */
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
      /* big bonus for fully aggressive mobs for now */
      if (!IS_SET(mob->specials.act, ACT_WIMPY)||
	  IS_SET(mob->specials.act, ACT_META_AGG))
	tmpexp = tmpexp / 1.5;
       tmpexp = tmpexp / 1.1;

      }

    /* revert exp for wimpy mobs (higher) */
    if (IS_SET(mob->specials.act, ACT_WIMPY))
       tmpexp = tmpexp/ 0.9;
    i = (int) tmpexp;   // reconvert to long after calculation

    if GET_RACE(mob) {
      fprintf(mob_fi, " %d ", -1);
      fprintf(mob_fi, " %d ", 5 * mob->points.gold); /* fix that odd money thing */
      fprintf(mob_fi, " %d ",GetExpFlags(mob, i) );
      fprintf(mob_fi, " %d \n", GET_RACE(mob));
    } else {
      fprintf(mob_fi, " %d ", mob->points.gold);

      /*
	this is where the new exp  fLAGS will come into play
	*/
      fprintf(mob_fi, " %d \n",GetExpFlags(mob, i) );
    }

    fprintf(mob_fi, " %d ",mob->specials.position );

    fprintf(mob_fi, " %d ", mob->specials.default_pos);

    if ((mob->immune) || (mob->M_immune) || (mob->susc)){
      fprintf(mob_fi, " %d ",mob->player.sex+3 );
      fprintf(mob_fi, " %d ",mob->immune);
      fprintf(mob_fi, " %d ",mob->M_immune);
      fprintf(mob_fi, " %d \n", mob->susc);
    } else {
      fprintf(mob_fi, " %d \n",mob->player.sex );
    }

    /*
     *   write in the sound string for a mobile
     */
    if (letter == 'L') {
		remove_cr(buff,mob->player.sounds);  // remove carriage return
		fwrite_string(mob_fi,buff);
		remove_cr(buff,mob->player.distant_snds);  // remove carriage return
		fwrite_string(mob_fi,buff);
	}
#endif
}

int save_new_mobile_structure(struct char_data *mob, FILE *mob_fi)//, int hpB)
{
	long    bc,ltmp;
	int i, xpflag;
	long tmp, tmp2, tmp3;
	float tmpexp;
	char letter;
	char buff[MAX_STRING_LENGTH];

	/***** String data *** */
	fwrite_string(mob_fi,mob->player.name);
	fwrite_string(mob_fi,mob->player.short_descr);
	remove_cr(buff,mob->player.long_descr);  // remove carriage return
	fwrite_string(mob_fi,buff);
	remove_cr(buff,mob->player.description);
	fwrite_string(mob_fi,buff);
	if(mob->player.sounds) {
		remove_cr(buff,mob->player.sounds);  // remove carriage return
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->player.distant_snds) {
		remove_cr(buff,mob->player.distant_snds);  // remove carriage return
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}

	/* *** Numeric data *** */

#if NEWMOBSTRUCTURE

	/* do nothing */

#else
	if(IS_SET(mob->specials.act,ACT_NECROMANCER)) {
		REMOVE_BIT(mob->specials.act, ACT_NECROMANCER); // replace old quest flag to new quest flag location
		mob->specials.proc = PROC_QUEST;
	}  /* this was a one time only action */
#endif

	tmp = mob->specials.act;    	// get actions flags
	REMOVE_BIT(tmp, ACT_ISNPC);   	// remove flag IS NPC
	REMOVE_BIT(tmp, ACT_HATEFUL);   // remove flag HATEFUL
	REMOVE_BIT(tmp, ACT_GUARDIAN);  // remove flag GUARDIAN
	REMOVE_BIT(tmp, ACT_HUNTING);	// remove flag HUNTING
	REMOVE_BIT(tmp, ACT_AFRAID);	// remove flag AFRAID

	fprintf(mob_fi, "%d ", tmp);    // write action flags
	fprintf(mob_fi, " %d ", mob->specials.affected_by);
	fprintf(mob_fi, " %d ",mob->specials.alignment );
	fprintf(mob_fi, " %.1f \n",mob->mult_att );

	fprintf(mob_fi, " %d ",GET_LEVEL(mob, WARRIOR_LEVEL_IND) );
	fprintf(mob_fi, " %d ",(mob->points.hitroll-20)*-1 );
	fprintf(mob_fi, " %d ",mob->points.armor/10 );
	fprintf(mob_fi, " %d ", mob->points.max_hit);
	fprintf(mob_fi, " %dd%d+%d \n", mob->specials.damnodice, mob->specials.damsizedice, mob->points.damroll);

	tmpexp = GET_EXP(mob);    // convert Exp to float for calculation
	/* revert exp for agressive	 mobs (lower) */
	if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
		/* big bonus for fully aggressive mobs for now */
		if (!IS_SET(mob->specials.act, ACT_WIMPY) || IS_SET(mob->specials.act, ACT_META_AGG))
			tmpexp = tmpexp / 1.5;
		tmpexp = tmpexp / 1.1;
	}
	/* revert exp for wimpy mobs (higher) */
	if (IS_SET(mob->specials.act, ACT_WIMPY))
		tmpexp = tmpexp/ 0.9;
	i = (int) tmpexp;   // reconvert to long after calculation
	if(mob->points.gold < 0)
		mob->points.gold = 0;
	i -= mob->points.gold;

	fprintf(mob_fi, " %d ", -1);
	fprintf(mob_fi, " %d ", mob->points.gold);

	xpflag = GetExpFlags(mob, i); /* make sure this mob gets an xp flag from 1 - 31 */
	if(xpflag < 1)
		xpflag = 1;
	if(xpflag > 31)
		xpflag = 31;

	fprintf(mob_fi, " %d ", xpflag);
	fprintf(mob_fi, " %d \n", GET_RACE(mob));

	fprintf(mob_fi, " %d ",mob->specials.position );
	fprintf(mob_fi, " %d ", mob->specials.default_pos);

	fprintf(mob_fi, " %d ",mob->player.sex);
	fprintf(mob_fi, " %d ",mob->immune);
	fprintf(mob_fi, " %d ",mob->M_immune);
	fprintf(mob_fi, " %d \n", mob->susc);

	fprintf(mob_fi, " %d \n", mob->specials.proc);

	if(mob->specials.talks) {
		remove_cr(buff,mob->specials.talks);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->specials.quest_yes) {
		remove_cr(buff,mob->specials.quest_yes);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
	if(mob->specials.quest_no) {
		remove_cr(buff,mob->specials.quest_no);
		fwrite_string(mob_fi,buff);
	} else {
		fwrite_string(mob_fi,"");
	}
}


int weaponconvert(struct obj_data *obj) {

	char buf[256];
	extern const struct skillset weaponskills[];
	int i=0;
	return 0;
	if(!obj)
		return WEAPON_GENERIC;
	while(weaponskills[i].level != -1) {
		if(strstr(obj->name,weaponskills[i].name))	{
			//sprintf(buf,"%s is a %s.\n\r",obj->name, weaponskills[i].name);
			//printf(buf);
			return 350+i;
		}


	}
	return WEAPON_GENERIC;
}


int write_obj_to_file(struct obj_data *obj, FILE *f)
{
	int     i,tmp;
	long    bc,ltmp;
	struct extra_descr_data *descr;

  /* fprintf(f,"#%ld\n",obj_index[obj->item_number].virtual); */
  fwrite_string(f, obj->name);
  fwrite_string(f, obj->short_description);
  fwrite_string(f, obj->description);
  fwrite_string(f, obj->action_description);
  fwrite_string(f, obj->modBy);

  fprintf(f,"%d %ld %ld\n", obj->obj_flags.type_flag,\
		obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);
  fprintf(f,"%d %d %d %d\n", obj->obj_flags.value[0], obj->obj_flags.value[1],\
		obj->obj_flags.value[2], obj->obj_flags.value[3]);

	fprintf(f,"%d %d %d %d %d %ld %d %d %d\n", obj->obj_flags.weight, \
		obj->obj_flags.cost, obj->obj_flags.cost_per_day,obj->level,obj->max
		,obj->modified, obj->speed, obj->weapontype, obj->tweak);


  /* *** extra descriptions *** */
  if(obj->ex_description)
    for(descr=obj->ex_description;descr;descr=descr->next) {
      fprintf(f,"E\n");
      fwrite_string(f,descr->keyword);
      fwrite_string(f,descr->description);
    }

  for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)
    if(obj->affected[i].location!=APPLY_NONE)
      fprintf(f,"A\n%d %d\n",obj->affected[i].location,obj->affected[i].modifier);

}
int save_new_object_structure(struct obj_data *obj, FILE *f)
{
	int     i,tmp;
	long    bc,ltmp;
	struct extra_descr_data *descr;

  /* fprintf(f,"#%ld\n",obj_index[obj->item_number].virtual); */
  fwrite_string(f, obj->name);
  fwrite_string(f, obj->short_description);
  fwrite_string(f, obj->description);
  fwrite_string(f, obj->action_description);
  fwrite_string(f, obj->modBy);
/* obj->obj_flags.cost_per_day>LIM_ITEM_COST_MIN         obj->obj_flags.extra_flags */


	if(obj->obj_flags.cost_per_day > LIM_ITEM_COST_MIN)
	  fprintf(f,"%d %ld %ld\n", obj->obj_flags.type_flag,\
		obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);
	else
	  fprintf(f,"%d %ld %ld\n", obj->obj_flags.type_flag,\
				obj->obj_flags.extra_flags, obj->obj_flags.wear_flags);



  fprintf(f,"%d %d %d %d\n", obj->obj_flags.value[0], obj->obj_flags.value[1],\
		obj->obj_flags.value[2], obj->obj_flags.value[3]);

 	if(IS_WEAPON(obj)) {
		fprintf(f,"%d %d %d %d %d %ld %d %d %d\n", obj->obj_flags.weight, \
			obj->obj_flags.cost, obj->obj_flags.cost_per_day==-1?-1:obj->obj_flags.cost_per_day, obj->level, obj->max, obj->modified, obj->speed, weaponconvert(obj), obj->tweak);
	} else {
		fprintf(f,"%d %d %d %d %d %ld 0 0 %d\n", obj->obj_flags.weight, \
		obj->obj_flags.cost, obj->obj_flags.cost_per_day==-1?-1:obj->obj_flags.cost_per_day, obj->level, obj->max, obj->modified, /* two zeros for speed and wtype*/ obj->tweak);
	}



  /* *** extra descriptions *** */
  if(obj->ex_description)
    for(descr=obj->ex_description;descr;descr=descr->next) {
      fprintf(f,"E\n");
      fwrite_string(f,descr->keyword);
      fwrite_string(f,descr->description);
    }

  for( i = 0 ; i < MAX_OBJ_AFFECT ; i++)
    if(obj->affected[i].location!=APPLY_NONE)
      fprintf(f,"A\n%d %d\n",obj->affected[i].location,obj->affected[i].modifier);

}

/* read an object from OBJ_FILE */
struct obj_data *read_object(int nr, int type)
{
  FILE *f;
  struct obj_data *obj;
  int tmp, i;
  long bc;
  char chk[50], buf[100];

  extern long obj_count;
  extern long total_obc;

  i = nr;
  if (type == VIRTUAL) {
    nr = real_object(nr);
  }
  if (nr<0 || nr>top_of_objt) {
    sprintf(buf, "Object (V) %d does not exist in database.", i);
    return(0);
  }

  CREATE(obj, struct obj_data, 1);

  if (!obj){
     log("Cannot create obj?! db.c read_obj");
     return(FALSE);
     }

  bc = sizeof(struct obj_data);

  clear_object(obj);

  if(obj_index[nr].data == NULL) {      /* object haven't data structure */
    if(obj_index[nr].pos == -1) {               /* object in external file */
      sprintf(buf,"%s/%ld",OBJ_DIR,obj_index[nr].virtual);
      if((f = fopen(buf,"rt"))==NULL) {
	sprintf(buf,"can't open object file for object %ld", obj_index[nr].virtual);
	log(buf);
	free_obj(obj);
	return(0);
      }
      fscanf(f, "#%*ld\n");
      bc += read_obj_from_file(obj, f);
      fclose(f);
    } else {
      rewind(obj_f);
      fseek(obj_f, obj_index[nr].pos, 0);
      bc += read_obj_from_file(obj, obj_f);
    }
  } else {                      /* data for object present */
    clone_obj_to_obj(obj, (struct obj_data *)obj_index[nr].data);
  }

  obj->in_room = NOWHERE;
  obj->next_content = 0;
  obj->carried_by = 0;
  obj->equipped_by = 0;
  obj->eq_pos = -1;
  obj->in_obj = 0;
  obj->contains = 0;
  obj->item_number = nr;
  obj->in_obj = 0;

  obj->next = object_list;
  object_list = obj;

  obj_index[nr].number++; /*Object maxxing*/
  //obj->level = 0;
  obj_count++;
#if BYTE_COUNT
  fprintf(stderr, "Object [%d] uses %d bytes\n", obj_index[nr].virtual, bc);
#endif
  total_obc += bc;
  return (obj);
}

#define ZO_DEAD  999

/* counts the number of inited zones */
int init_counter()
{
	int i, j = 0;

	for (i = 0; i <= top_of_zone_table; i++) {
		if (zone_table[i].start && zone_table[i].age < ZO_DEAD) {
			j++;
		}
	}
	return(j);
}

/* update zone ages, queue for reset if necessary, and dequeue when possible */
void zone_update()
{
  int i,j;
  struct reset_q_element *update_u, *temp, *tmp2;
  extern struct reset_q_type reset_q;
  char buf[128];
  struct char_data *newch;
  int to_room = 0;
  struct room_data *room;
  struct obj_data *travelqp, *tmp;

  /* enqueue zones */

	for (i = 0; i <= top_of_zone_table; i++) {
		if (zone_table[i].start) { /* has been reset already */
			if (zone_table[i].age < zone_table[i].lifespan && zone_table[i].reset_mode)
				(zone_table[i].age)++;
			else if (zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {
				/* enqueue zone */

				CREATE(update_u, struct reset_q_element, 1);

				update_u->zone_to_reset = i;
				update_u->next = 0;

				if (!reset_q.head)
					reset_q.head = reset_q.tail = update_u;
				else  {
					reset_q.tail->next = update_u;
					reset_q.tail = update_u;
				}
				zone_table[i].age = ZO_DEAD;
			}
		}
	}

	/* dequeue zones (if possible) and reset */
	for (update_u = reset_q.head; update_u; update_u = tmp2) {
		if (update_u->zone_to_reset > top_of_zone_table) {

		/*  this may or may not work */
		/*  may result in some lost memory, but the loss is not signifigant
		 *	over the short run
		 */

			update_u->zone_to_reset = 0;
			update_u->next = 0;
		}
		tmp2 = update_u->next;

		if (IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_ALWAYS) ||
			IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_NODEINIT) ||
			(IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_EMPTY) &&
			is_empty(update_u->zone_to_reset))) {
/* Lennya's deinit of zones stuff */
			if (IS_SET(zone_table[update_u->zone_to_reset].reset_mode, ZONE_NODEINIT)) {
				/* this should never deinit, or set back to 0 */
				reset_zone(update_u->zone_to_reset,0);
			} else if (!is_empty(update_u->zone_to_reset)) {
				/* just reset, not cleaned when there's people in it */
				reset_zone(update_u->zone_to_reset,0);
			} else {
				CleanZone(update_u->zone_to_reset);
				zone_table[update_u->zone_to_reset].start=0;
				sprintf(buf, "zone %d just deinited",update_u->zone_to_reset);
				log(buf);
			}
			/* dequeue */
			if (update_u == reset_q.head) {
				reset_q.head = reset_q.head->next;
			} else {
				for (temp = reset_q.head; temp->next != update_u; temp = temp->next)
					;
				if (!update_u->next)
					reset_q.tail = temp;
				temp->next = update_u->next;
			}
			if (update_u)
				free(update_u);
		}
	}

	i = init_counter();
	if(i > (MIN_INIT_TQP-1)) {
		for(i = 1; i <= TQP_AMOUNT; i++) {
			if(!(travelqp = find_tqp(i))) {
				// init the travelqp thing
				newch = 0;
				qp_patience = 0;
				while (!newch) { // this MAY cause endless loop, may have to go    for(1..100)
					to_room = number(0, top_of_world);
					room = real_roomp(to_room);
					if (room) {
						if(newch = room->people) {
							if (IS_PC(newch) || IS_SET(newch->specials.act, ACT_POLYSELF)) {
								newch = 0;
							} else {
								// does he already have one? (is this really needed? more calcs)
								for(j = 1; j <= TQP_AMOUNT; j++) {
									if(tmp = find_tqp(j)) {
										if(newch == tmp->carried_by) {
											newch = 0;
										}
									}
								}
							}
						}
					}
				}
				if(!(travelqp = read_object(TRAVELQP, VIRTUAL))) {
					log("no tqp item could be loaded");
					return;
				}
				obj_to_char(travelqp, newch);
			}
		}
		log("travelqp inited");
	}
}


int does_Load(int num, int max ) {
  float temp = 0.0;
  float temp2 = 0.0;
  char buff[200];

	if (max == 0)
  		return (TRUE);

	if(number(0,100) < max)
		return(TRUE);
	else
		return(FALSE);

  return TRUE;
#if 1
  sprintf(buff,"num=%d  max=%d", num, max);
  log(buff);
  if (max == 0)
    return (TRUE);

  if (num > max) { /* Maxxed.. but there is a slight chance of loading (GH)*/
    temp = (max / 2) + 1;  /* 20/2 would be a 10% chance of loading..*/
  }else
    temp = 100 - ((num / (2*max)) * 100.0);
  temp2 = number(1,101);

  sprintf(buff,"Chance to load: %d    <=? Dice roll: %d",temp, temp2);
  log(buff);
  if (temp2 <= temp)
    return TRUE;
  else {
    log("ITEM NEVER LOADED");
    return FALSE;
  }
#endif
}

#define ZCMD zone_table[zone].cmd[cmd_no]

/* execute the reset command table of a given zone */
void reset_zone(int zone, int cmd)
{
  int cmd_no, last_cmd = 1, j, i, tweakroll = 100,tweakrate = 0;
  int tweakmin = 0; /* this is the minimum tweak rate on ALL items, currently 0 */
  char buf[256];
  struct char_data *mob;
  struct char_data *master;
  struct obj_data *obj, *obj_to;
  struct room_data      *rp;
  FILE *fl;
  static int done = 0;
  extern long SystemFlags;
  mob = 0;

	if(zone == 189) {
		cog_sequence = 0;
	}

#if OLD_ZONE_STUFF
#if 0
  if (zone == 0 && !done) {
    done = 1;
    for (i=0;i<WORLD_SIZE;i+=1000)
    {
      sprintf(buf, "world/mobs.%d", i);
      fl = fopen(buf, "r");
      if(!fl)
      {
	log("Unable to load scratch zone file for update.");
	return;
      }
      ReadTextZone(fl);
      fclose(fl);
    }
    return;
  }
#endif


  if (zone_table[zone].start == 0) {
    char        *s;
    int d,e;
    s = zone_table[zone].name;
    d = (zone ? (zone_table[zone - 1].top + 1) : 0);
    e = zone_table[zone].top;
    sprintf(buf, "Run time initialization of zone %s, rooms (%d-%d)",
	    s, d, e);
    log(buf);
  }

  if (!zone_table[zone].cmd) {
    return;
  }

  for (cmd_no = 0;;cmd_no++) {
    if (ZCMD.command == 'S')
      break;

    if (last_cmd || ZCMD.if_flag <= 0)
      switch(ZCMD.command) {
      case 'M': /* read a mobile */
	if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) &&
	    !fighting_in_room(ZCMD.arg3) &&
	    !CheckKillFile(mob_index[ZCMD.arg1].virtual)) {
	  mob = read_mobile(ZCMD.arg1, REAL) ;
	  if (!mob) {
	  		log_sev("Error while loading mob in reset_zone(read)", 1);
	  		last_cmd = 0;
	  		break;
	            } else ;

	  mob->specials.zone = zone;
	  char_to_room(mob, ZCMD.arg3);

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[zone].races, GET_RACE(mob))) {
	      zone_table[zone].races[strlen(zone_table[zone].races)] = GET_RACE(mob);
	    }

	  last_cmd = 1;
	  master = mob;
	} else
	  last_cmd = 0;
	break;

      case 'C': /* read a mobile.  Charm them to follow prev. */
	if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) && !CheckKillFile(mob_index[ZCMD.arg1].virtual)) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  if (!mob) {
	             log_sev("error loading mob in Reset_zone(Charm)",1);
	             last_cmd= 0;
	             break;
	            } else ;
	  mob->specials.zone = zone;

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[zone].races, GET_RACE(mob))) {
	      zone_table[zone].races[strlen(zone_table[zone].races)] = GET_RACE(mob);
	    }

	  if (master) {
	    char_to_room(mob, master->in_room);
	    /*
	      add the charm bit to the dude.
	      */
	    add_follower(mob, master);
	    SET_BIT(mob->specials.affected_by, AFF_CHARM);
	    SET_BIT(mob->specials.act, ZCMD.arg3);
	  } else {
	    extract_char(mob);
	    last_cmd = 0;
	  }
	  last_cmd = 1;
	}
	else
	  last_cmd = 0;
	break;

      case 'Z':  /* set the last mobile to this zone */
	if (mob) {
	  mob->specials.zone = ZCMD.arg1;

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[ZCMD.arg1].races, GET_RACE(mob))) {
	   zone_table[ZCMD.arg1].races[strlen(zone_table[ZCMD.arg1].races)] =
	     GET_RACE(mob);
	    }
	}
	break;

      case 'O': /* read an object */ /*On ground load (GH) */

	if (TRUE) {
	  if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL)) {
	    if((ZCMD.if_flag>0&&ObjRoomCount(ZCMD.arg1,rp)<ZCMD.if_flag) ||
	      (ZCMD.if_flag<=0&&ObjRoomCount(ZCMD.arg1,rp)<(-ZCMD.if_flag)+1)){
	      if ((obj = read_object(ZCMD.arg1, REAL)) != NULL) {
		obj_index[ZCMD.arg1].MaxObjCount = ZCMD.arg2;

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

		if(cmd != 375) {
			if(does_Load((int)obj_index[ZCMD.arg1].number, (int)obj->max)==TRUE)
				obj_to_room(obj, ZCMD.arg3);
			else
				extract_obj(obj);
		} else {
			obj_to_room(obj, ZCMD.arg3);
		}

		last_cmd = 1;
	      } else {
		last_cmd = 0;
	      }
	    } else {
		last_cmd = 0;
	    }
	  } else if (obj = read_object(ZCMD.arg1, REAL)) {
	    sprintf(buf, "Error finding room #%d", ZCMD.arg3);
	    log(buf);
	    last_cmd = 1;
	  }  else {
	    last_cmd = 0;
	  }
	}
	break;

      case 'P': /* object to object */
	if (TRUE)  {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj_to = get_obj_num(ZCMD.arg3);
	  if (obj_to && obj) {
            obj_index[ZCMD.arg1].MaxObjCount = ZCMD.arg2;

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

		if(cmd != 375) {
			if(does_Load((int)obj_index[ZCMD.arg1].number, (int)obj->max)==TRUE)
				obj_to_obj(obj, obj_to);
			else
				extract_obj(obj);
		} else {
			obj_to_obj(obj, obj_to);
		}

	    last_cmd = 1;
	  } else {
	    last_cmd = 0;
	  }
	}
	else
	  last_cmd = 0;
	break;

      case 'G': /* obj_to_char */
	if (TRUE  &&
	    (obj = read_object(ZCMD.arg1, REAL)) && mob) {
	  obj_index[ZCMD.arg1].MaxObjCount = ZCMD.arg2;

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}
		if(cmd != 375) {
			if(does_Load((int)obj_index[ZCMD.arg1].number, (int)obj->max)==TRUE)
				obj_to_char(obj, mob);
			else
				extract_obj(obj);
		} else {
			obj_to_char(obj, mob);
		}

	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'H': /* hatred to char */

	if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'F': /* fear to char */

	if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'E': /* object to equipment list */
	if (TRUE &&
	    (obj = read_object(ZCMD.arg1, REAL))) {
	  if (!mob->equipment[ZCMD.arg3]) {
	     obj_index[ZCMD.arg1].MaxObjCount = ZCMD.arg2;

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}
		if(cmd != 375) {
			if(does_Load((int)obj_index[ZCMD.arg1].number, (int)obj->max)==TRUE)
				equip_char(mob, obj, ZCMD.arg3);
			else
				extract_obj(obj);
		} else {
			equip_char(mob, obj, ZCMD.arg3);
		}

	  } else {
	    sprintf(buf, "eq error - zone %d, cmd %d, item %d, mob %d, loc %d\n", zone, cmd_no,
		    obj_index[ZCMD.arg1].virtual, mob_index[mob->nr].virtual, ZCMD.arg3);
	    log_sev(buf, 6);
	  }
	    last_cmd = 1;
	  }    else
	    last_cmd = 0;
	break;

      case 'D': /* set state of door */
	rp = real_roomp(ZCMD.arg1);
	if (rp && rp->dir_option[ZCMD.arg2]) {
	  switch (ZCMD.arg3) {
	  case 0:
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  case 1:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    break;
	  case 2:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  }
	  last_cmd = 1;
	} else {
	  /* that exit doesn't exist anymore */
	}
	break;

      default:
	sprintf(buf, "Undefd cmd in reset table; zone %d cmd# %d\n\r",
		zone, cmd_no);
	log(buf);
	break;
      }
    else
      last_cmd = 0;

  }
#if 0
  for (j=0;zone_table[zone].races[j];j++) {
    fprintf(stderr, "%d ", (int)zone_table[zone].races[j]);
  }
  fprintf(stderr, "\n");
#endif

  zone_table[zone].age = 0;
  zone_table[zone].start = 1;
#else
		/* Old Silly reset zone... */

  if (zone == 0 && !done) {
    done = 1;
#if N_SAVE_WORLD
    for (i=0;i<30000;i+=1000) {
      sprintf(buf, "world/mobs.%d", i);
      fl = fopen(buf, "r");
      if(!fl) {
	log("Unable to load scratch zone file for update.");
	return;
      }
      ReadTextZone(fl);
      fclose(fl);
    }
#endif
    return;
  }



  if (zone_table[zone].start == 0) {
    char        *s;
    int d,e;
    s = zone_table[zone].name;
    d = (zone ? (zone_table[zone - 1].top + 1) : 0);
    e = zone_table[zone].top;
    sprintf(buf, "Run time initialization of zone %s, rooms (%d-%d)",
	    s, d, e);
    log(buf);

  }

  if (!zone_table[zone].cmd) {
    return;
  }

  for (cmd_no = 0;;cmd_no++) {
    if (ZCMD.command == 'S')
      break;

    if (last_cmd || ZCMD.if_flag <= 0)
      switch(ZCMD.command) {
      case 'M': /* read a mobile */
	if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) && !CheckKillFile(mob_index[ZCMD.arg1].virtual)) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  if (!mob) {
	             log_sev("error loading mob in Reset_zone(Read)",1);
	             last_cmd= 0;
	             break;
	            } else ;
	  mob->specials.zone = zone;
	  char_to_room(mob, ZCMD.arg3);

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[zone].races, GET_RACE(mob))) {
	      zone_table[zone].races[strlen(zone_table[zone].races)] = GET_RACE(mob);
	    }

	  last_cmd = 1;
	  master = mob;
	} else
	  last_cmd = 0;
	break;

      case 'C': /* read a mobile.  Charm them to follow prev. */
	if ((mob_index[ZCMD.arg1].number < ZCMD.arg2) && !CheckKillFile(mob_index[ZCMD.arg1].virtual)) {
	  mob = read_mobile(ZCMD.arg1, REAL);
	  if (!mob) {
	             log_sev("error loading mob in Reset_zone(Charm)",1);
	             last_cmd= 0;
	             break;
	             } else ;
	  mob->specials.zone = zone;

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[zone].races, GET_RACE(mob))) {
	      zone_table[zone].races[strlen(zone_table[zone].races)] = GET_RACE(mob);
	    }

	  if (master) {
	    char_to_room(mob, master->in_room);
	    /*
	      add the charm bit to the dude.
	      */
	    add_follower(mob, master);
	    SET_BIT(mob->specials.affected_by, AFF_CHARM);
	    SET_BIT(mob->specials.act, ZCMD.arg3);
	  } else {
	    extract_char(mob);
	    last_cmd = 0;
	  }
	  last_cmd = 1;
	}
	else
	  last_cmd = 0;
	break;

      case 'Z':  /* set the last mobile to this zone */
	if (mob) {
	  mob->specials.zone = ZCMD.arg1;

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[ZCMD.arg1].races, GET_RACE(mob))) {
	   zone_table[ZCMD.arg1].races[strlen(zone_table[ZCMD.arg1].races)] =
	     GET_RACE(mob);
	    }
	}
	break;

      case 'O': /* read an object */
	if (obj_index[ZCMD.arg1].number <= ZCMD.arg2 || obj_index[ZCMD.arg1].number >= ZCMD.arg2) {
	  if (ZCMD.arg3 >= 0 && ((rp = real_roomp(ZCMD.arg3)) != NULL)) {
	    if((ZCMD.if_flag>0&&ObjRoomCount(ZCMD.arg1,rp)<ZCMD.if_flag) ||
	      (ZCMD.if_flag<=0&&ObjRoomCount(ZCMD.arg1,rp)<(-ZCMD.if_flag)+1)){
	      if ((obj = read_object(ZCMD.arg1, REAL)) != NULL) {

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

		obj_to_room(obj, ZCMD.arg3);
		last_cmd = 1;
	      } else {
		last_cmd = 0;
	      }
	    } else {
		last_cmd = 0;
	    }
	  } else if (obj = read_object(ZCMD.arg1, REAL)) {
	    sprintf(buf, "Error finding room #%d", ZCMD.arg3);
	    log(buf);
	    last_cmd = 1;
	  }  else {
	    last_cmd = 0;
	  }
	}
	break;

      case 'P': /* object to object */
	if (obj_index[ZCMD.arg1].number <= ZCMD.arg2 || obj_index[ZCMD.arg1].number >= ZCMD.arg2)  {
	  obj = read_object(ZCMD.arg1, REAL);
	  obj_to = get_obj_num(ZCMD.arg3);
	  if (obj_to && obj) {

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

	    obj_to_obj(obj, obj_to);
	    last_cmd = 1;
	  } else {
	    last_cmd = 0;
	  }
	}
	else
	  last_cmd = 0;
	break;

      case 'G': /* obj_to_char */
	if ((obj_index[ZCMD.arg1].number <= ZCMD.arg2 || obj_index[ZCMD.arg1].number >= ZCMD.arg2) &&
	    (obj = read_object(ZCMD.arg1, REAL))) {

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

	  obj_to_char(obj, mob);
	  last_cmd = 1;
	} else
	  last_cmd = 0;
	break;

      case 'H': /* hatred to char */

	if (AddHatred(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'F': /* fear to char */

	if (AddFears(mob, ZCMD.arg1, ZCMD.arg2))
	  last_cmd = 1;
	else
	  last_cmd = 0;
	break;

      case 'E': /* object to equipment list */
	if ((obj_index[ZCMD.arg1].number <= ZCMD.arg2  || obj_index[ZCMD.arg1].number >= ZCMD.arg2) &&
	    (obj = read_object(ZCMD.arg1, REAL))) {

	if(!SET_BIT(SystemFlags,SYS_NO_TWEAK)) {
	  tweakroll = number(1,100);
	  if(obj->tweak < tweakmin) {
		  tweakrate = tweakmin;
	  } else {
		  tweakrate = obj->tweak;
	  }
	  if(tweakroll <= tweakrate)
		tweak(obj);
	}

	  if (!mob->equipment[ZCMD.arg3]) {
	    equip_char(mob, obj, ZCMD.arg3);
	  } else {
	    sprintf(buf, "eq error - zone %d, cmd %d, item %d, mob %d, loc %d\n", zone, cmd_no,
		    obj_index[ZCMD.arg1].virtual, mob_index[mob->nr].virtual, ZCMD.arg3);
	    log_sev(buf, 6);
	  }
	    last_cmd = 1;
	  }    else
	    last_cmd = 0;
	break;

      case 'D': /* set state of door */
	rp = real_roomp(ZCMD.arg1);
	if (rp && rp->dir_option[ZCMD.arg2]) {
	  switch (ZCMD.arg3) {
	  case 0:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  case 1:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR);
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    REMOVE_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    break;
	  case 2:
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_ISDOOR);
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_LOCKED);
	    SET_BIT(rp->dir_option[ZCMD.arg2]->exit_info, EX_CLOSED);
	    break;
	  }
	  last_cmd = 1;
	} else {
	  /* that exit doesn't exist anymore */
	}
	break;

      default:
	sprintf(buf, "Undefd cmd [%c] in reset table; zone %d cmd %d.",
		ZCMD.command,zone, cmd_no);
	log(buf);
	break;
      }
    else
      last_cmd = 0;

  }
#if 0
  for (j=0;zone_table[zone].races[j];j++) {
    fprintf(stderr, "%d ", (int)zone_table[zone].races[j]);
  }
  fprintf(stderr, "\n");
#endif

  zone_table[zone].age = 0;
  zone_table[zone].start = 1;

#endif

}

#undef ZCMD

/* for use in reset_zone; return TRUE if zone 'nr' is free of PC's  */
int is_empty(int zone_nr)
{
  struct descriptor_data *i;

  for (i = descriptor_list; i; i = i->next)
    if (!i->connected)
      if (real_roomp(i->character->in_room)->zone == zone_nr)
	return(0);

  return(1);
}





/*************************************************************************
*  stuff related to the save/load player system                                                           *
*********************************************************************** */

/* Load a char, TRUE if loaded, FALSE if not */
int load_char(char *name, struct char_file_u *char_element)
{
  FILE *fl;
  int player_i;

  int find_name(char *name);

  if ((player_i = find_name(name)) >= 0) {

    if (!(fl = fopen(PLAYER_FILE, "r"))) {
      perror("Opening player file for reading. (db.c, load_char)");
      assert(0);
    }
    rewind(fl);
    fseek(fl, (long) (player_table[player_i].nr *
		      sizeof(struct char_file_u)), 0);

    fread(char_element, sizeof(struct char_file_u), 1, fl);
    fclose(fl);
    /*
     **  Kludge for ressurection
     */
    char_element->talks[2] = FALSE; /* they are not dead */
    return(player_i);

  } else

    return(-1);
}




/* copy data from the file structure to a char struct */
void store_to_char(struct char_file_u *st, struct char_data *ch)
{
  int i;
  int max;
	char buf[100];

  GET_SEX(ch) = st->sex;
  ch->player.class = st->class;


  for (i=MAGE_LEVEL_IND; i< MAX_CLASS; i++)
    ch->player.level[i] = st->level[i];

	/* to make sure all levels above the normal are 0 */
 for (i=MAX_CLASS;i<ABS_MAX_CLASS;i++)
    ch->player.level[i] = 0;



  GET_RACE(ch)  = st->race;

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
//printf(GET_NAME(ch));
//printf(st->title);
  if (*st->title)       {
    CREATE(ch->player.title, char, strlen(st->title) + 1);
    strcpy(ch->player.title, st->title);
  }  else {
	CREATE(ch->player.title, char, strlen(st->name) + 1);
    strcpy(ch->player.title, st->name);
  }
	/*some little fixxes for new title (GH) may28/2002 */
    if (!strstr(ch->player.title, st->name)) {
		sprintf(ch->player.title,"%s",st->name);
	}

  if (*st->description) {
    CREATE(ch->player.description, char,
	   strlen(st->description) + 1);
    strcpy(ch->player.description, st->description);
  } else
    ch->player.description = 0;


  ch->player.hometown = st->hometown;

  ch->player.q_points = st->q_points;

  ch->player.time.birth = st->birth;

  ch->player.time.played = st->played;

  ch->player.time.logon  = time(0);

  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    ch->player.talks[i] = st->talks[i];

  ch->player.weight = st->weight;
  ch->player.height = st->height;

  ch->abilities = st->abilities;
  ch->tmpabilities = st->abilities;
  ch->points = st->points;


  SpaceForSkills(ch);

  if (HowManyClasses(ch) >= 3) {
    max = 80;
  } else if (HowManyClasses(ch) == 2) {
    max = 87;
  } else {
    max = 95;
  }

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    ch->skills[i].flags   = st->skills[i].flags;
    ch->skills[i].special = st->skills[i].special;
    ch->skills[i].nummem  = st->skills[i].nummem;
    ch->skills[i].learned = MIN(st->skills[i].learned, max);
  }

  for (i = WEAPON_FIRST; i <= WEAPON_LAST; i++) {
    ch->skills[i].flags   = st->skills[i].flags;
    ch->skills[i].special = st->skills[i].special;
    ch->skills[i].nummem  = st->skills[i].nummem;
    ch->skills[i].learned = MIN(st->skills[i].learned, max);
  }

	ch->specials.a_deaths = st->a_deaths;
	ch->specials.a_kills = st->a_kills;
	ch->specials.m_deaths = st->m_deaths;
	ch->specials.m_kills = st->m_kills;

	ch->specials.remortclass = 1; //st->remortclass;

  ch->specials.spells_to_learn = st->spells_to_learn;
  ch->specials.alignment    = st->alignment;

  ch->specials.act          = st->act;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items  = 0;
  ch->specials.pmask        = 0;
  ch->specials.poofin       = 0;
  ch->specials.poofout      = 0;
  ch->specials.group_name   = 0;
  ch->points.armor          = 100;
  ch->points.hitroll        = 0;
  ch->points.damroll        = 0;
  ch->specials.affected_by  = st->affected_by;
  ch->specials.affected_by2  = st->affected_by2;
  ch->specials.start_room     = st->startroom;

     ch->player.speaks = st->speaks;
	ch->player.user_flags = st->user_flags;

  ch->player.extra_flags    = st->extra_flags;

  CREATE(GET_NAME(ch), char, strlen(st->name) +1);
	strcpy(GET_NAME(ch), st->name);


  for(i = 0; i < MAX_SAVES ; i++) {
    ch->specials.apply_saving_throw[i] = 0;
  }

  for(i = 0; i <= 2; i++)
    GET_COND(ch, i) = st->conditions[i];

  /* Add all spell effects */
  ch->affected=0;
  for(i=0; i < MAX_AFFECT; i++) {
    if (st->affected[i].type)
      affect_to_char(ch, &st->affected[i]);
  }


  ch->in_room = st->load_room;


  ch->term = 0;

  /* set default screen size */
  ch->size = 25;

	ch->specials.mobedit=0;
	ch->specials.objedit=0;
	ch->specials.help=0;
	ch->specials.misc=0;
	ch->specials.fighting=0;
	ch->specials.hunting=0;
	ch->specials.ridden_by=0;
	ch->specials.mounted_on=0;
	ch->specials.charging=0;
	ch->specials.auction = 0;//st->auction;
	ch->specials.minbid = 0;//st->minbid;
	ch->specials.is_playing = 0;
	ch->specials.is_hearing = 0;
	ch->style=0;
	       /* new stuff added 1-26-95 msw */
	       /* need to malloc the space for the pc struct */
    ch->pc = (void*)malloc(sizeof(struct pc_data));
    if (ch->pc) {
	    bzero(ch->pc,sizeof(struct pc_data));
       } else {
	log("malloc for ch->pc failed, db.c store_to_char");
     }
	     ch->pc->plr=0;  /* should = st->act eventually */
	     ch->pc->dimd_credits=0;
	     ch->pc->comm=0;
	/* end */


  affect_total(ch);
} /* store_to_char */




/* copy vital data from a players char-structure to the file structure */
void char_to_store(struct char_data *ch, struct char_file_u *st)
{
  int i;
  struct affected_type *af;
  struct obj_data *char_eq[MAX_WEAR];

  /* Unaffect everything a character can be affected by */


  for(i=0; i<MAX_WEAR; i++) {
    if (ch->equipment[i])
      char_eq[i] = unequip_char(ch, i);
    else
      char_eq[i] = 0;
  }

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
    if (af) {
      st->affected[i] = *af;
      st->affected[i].next = 0;
      /* subtract effect of the spell or the effect will be doubled */
      affect_modify( ch, st->affected[i].location,
		    st->affected[i].modifier,
		    st->affected[i].bitvector, FALSE);
      af = af->next;
    } else {
      st->affected[i].type = 0;  /* Zero signifies not used */
      st->affected[i].duration = 0;
      st->affected[i].modifier = 0;
      st->affected[i].location = 0;
      st->affected[i].bitvector = 0;
      st->affected[i].next = 0;
    }
  }

  if ((i >= MAX_AFFECT) && af && af->next)
    log("WARNING: OUT OF STORE ROOM FOR AFFECTED TYPES!!!");



  ch->tmpabilities = ch->abilities;

  st->birth      = ch->player.time.birth;
  st->played     = ch->player.time.played;
  st->played    += (long) (time(0) - ch->player.time.logon);
  st->last_logon = time(0);


  ch->player.time.played = st->played;
  ch->player.time.logon = time(0);

  st->hometown = ch->player.hometown;
  st->q_points = ch->player.q_points;
  st->weight   = GET_WEIGHT(ch);
  st->height   = GET_HEIGHT(ch);
  st->sex      = GET_SEX(ch);
  st->class    = ch->player.class;

  for (i=MAGE_LEVEL_IND; i< MAX_CLASS; i++)
    st->level[i]    = ch->player.level[i];

  st->race     = GET_RACE(ch);

  st->abilities = ch->abilities;
  st->points    = ch->points;
  st->alignment       = ch->specials.alignment;
  st->spells_to_learn = ch->specials.spells_to_learn;
  st->act             = ch->specials.act;
  st->affected_by      = ch->specials.affected_by;
  st->affected_by2     = ch->specials.affected_by2;
  st->startroom =     ch->specials.start_room;
  st->extra_flags =   ch->player.extra_flags;


	st->a_deaths= ch->specials.a_deaths;
	st->a_kills = ch->specials.a_kills;
	st->m_deaths = ch->specials.m_deaths;
	st->m_kills  = ch->specials.m_kills;

//	 st->auction = ch->specials.auction;
//	 st->minbid = ch->specials.minbid;

     st->speaks = ch->player.speaks;
	st->user_flags = ch->player.user_flags;
  st->points.armor   = 100;
  st->points.hitroll =  0;
  st->points.damroll =  0;

  if (GET_TITLE(ch))
    strcpy(st->title, GET_TITLE(ch));
  else
    *st->title = '\0';

  if (ch->player.description)
    strcpy(st->description, ch->player.description);
  else
    *st->description = '\0';


  for (i = 0; i <= MAX_TOUNGE - 1; i++)
    st->talks[i] = ch->player.talks[i];

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
     st->skills[i] = ch->skills[i];
     st->skills[i].flags   = ch->skills[i].flags;
     st->skills[i].special = ch->skills[i].special;
     st->skills[i].nummem  = ch->skills[i].nummem;
    }
  for (i = WEAPON_FIRST; i <= WEAPON_LAST; i++) {
    st->skills[i] = ch->skills[i];
        st->skills[i].flags   = ch->skills[i].flags;
        st->skills[i].special = ch->skills[i].special;
     st->skills[i].nummem  = ch->skills[i].nummem;
  }

  strcpy(st->name, GET_NAME(ch) );

  for(i = 0; i < MAX_SAVES; i++)
    st->apply_saving_throw[i] = ch->specials.apply_saving_throw[i];

  for(i = 0; i <= 2; i++)
    st->conditions[i] = GET_COND(ch, i);

  for(af = ch->affected, i = 0; i<MAX_AFFECT; i++) {
    if (af) {
      /* Add effect of the spell or it will be lost */
      /* When saving without quitting               */
      affect_modify( ch, st->affected[i].location,
			 st->affected[i].modifier,
		    st->affected[i].bitvector, TRUE);
      af = af->next;
    }
  }
  for(i=0; i<MAX_WEAR; i++) {
    if (char_eq[i])
      equip_char(ch, char_eq[i], i);
  }
  affect_total(ch);
} /* Char to store */




/* create a new entry in the in-memory index table for the player file */
int create_entry(char *name)
{
  int i;

  if (top_of_p_table == -1)     {
    CREATE(player_table, struct player_index_element, 1);
    top_of_p_table = 0;
  }  else
    if (!(player_table = (struct player_index_element *)
	  realloc(player_table, sizeof(struct player_index_element) *
		  (++top_of_p_table + 1))))
      {
	perror("create entry");
	assert(0);
      }

  CREATE(player_table[top_of_p_table].name, char , strlen(name) + 1);

  /* copy lowercase equivalent of name to table field */
  for (i = 0; *(player_table[top_of_p_table].name + i) =
       LOWER(*(name + i)); i++);

  player_table[top_of_p_table].nr = top_of_p_table;

  return (top_of_p_table);
}



/* write the vital data of a player to the player file */
void save_char(struct char_data *ch, sh_int load_room)
{
  struct char_file_u st;
  FILE *fl;
  char mode[4];
  int expand;
  struct char_data *tmp;


  if (IS_NPC(ch) && !(IS_SET(ch->specials.act, ACT_POLYSELF))) {
    return;
  }

/*      if (IS_CARRYING_W(ch) < 0) {
	char buf[256];
	sprintf(buf,"%s has a negative weight carried of (%d) please fix me",
		GET_NAME(ch),IS_CARRYING_W(ch));
	log(buf);
	} This was ridiculous! -MW */

  if (IS_NPC(ch)) {
    if (!ch->desc) {
      return;
      }
    tmp = ch->desc->original;
    if (!tmp) {
      return;
      }

  } else {
    if (!ch->desc) {
      return;
      }
    tmp = 0;
  }
  if (expand = (ch->desc->pos > top_of_p_file)) {
    strcpy(mode, "a");
    top_of_p_file++;
  }  else
    strcpy(mode, "r+");

  if (!tmp)
    char_to_store(ch, &st);
  else
    char_to_store(tmp, &st);

  st.load_room = load_room;

  strcpy(st.pwd, ch->desc->pwd);

  if (!(fl = fopen(PLAYER_FILE, mode))) {
    perror("save char");
    assert(0);
  }

  if (!expand)
    rewind(fl);
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);

  fwrite(&st, sizeof(struct char_file_u), 1, fl);

  fclose(fl);
}




/* for possible later use with qsort */
int compare(struct player_index_element *arg1, struct player_index_element
	*arg2)
{
	return (str_cmp(arg1->name, arg2->name));
}




/************************************************************************
*  procs of a (more or less) general utility nature                     *
********************************************************************** */

int fwrite_string (FILE *fl, char *buf)
{
  if(buf)
    return (fprintf(fl, "%s~\n", buf));
  else
    return (fprintf(fl, "~\n", buf));
}

/* Original fread_string was a memory leaking piece of garbage... -DM */
char *fread_string(FILE *f1) {
	char buf[MAX_STRING_LENGTH];
	int i = 0, tmp;

	buf[0] = '\0';

	while(i < MAX_STRING_LENGTH - 2) {
		tmp = fgetc(f1);
		if(!tmp) {
			perror("Fread_string");
			assert(0);
		}

		if(tmp == '~') {
			break;
		}

		buf[i++] = (char)tmp;
		if (buf[i-1] == '\n')
			buf[i++] = '\r';
	}

	if(i == MAX_STRING_LENGTH - 3) { /* We filled the buffer */
		buf[i] = '\0';
		log("File too long (fread_string).");
		while(tmp = fgetc(f1))
			if(tmp == '~')
				break;
	} else
		buf[i] = '\0';

	fgetc(f1);

	return((char *)strdup(buf));
}



/* release memory allocated for a char struct */
void free_char(struct char_data *ch)
{
  struct affected_type *af, *af1;
  int i;

if (GET_NAME(ch))
  free(GET_NAME(ch));

  if (ch->player.title)
    free(ch->player.title);
  if (ch->player.short_descr)
    free(ch->player.short_descr);
  if (ch->player.long_descr)
    free(ch->player.long_descr);
  if(ch->player.description)
    free(ch->player.description);
  if (ch->player.sounds)
    free(ch->player.sounds);
  if (ch->player.distant_snds)
    free(ch->player.distant_snds);
  if (ch->specials.A_list) {
    for (i=0;i<10;i++) {
      if (GET_ALIAS(ch, i))
	free(GET_ALIAS(ch, i));
    }
    free(ch->specials.A_list);
  }

  for (af = ch->affected; af; af = af1)
  {
    if (af)
      {
       af1 = af->next;
       affect_remove(ch, af);
      }
  }

  if (ch->skills)
    free(ch->skills);


if (ch->pc)
   free(ch->pc);
 if (ch)
  free(ch);
}

/* release memory allocated for an obj struct */
void free_obj(struct obj_data *obj)
{
	struct extra_descr_data *this, *next_one;

if (!obj) {                             /* bug fix, msw */
  log("!obj in free_obj, db.c");
  return;
 }
	if (obj->name && *obj->name)    /* msw, bug fix */
		free(obj->name);
	if(obj->description && *obj->description)
		free(obj->description);
	if(obj->short_description && *obj->short_description)
		free(obj->short_description);
	if(obj->action_description && *obj->action_description)
		free(obj->action_description);
	if(obj->modBy && *obj->modBy)
		free(obj->modBy);

	for( this = obj->ex_description ;
		(this != 0);this = next_one )
	{
		next_one = this->next;
		if(this->keyword)
			free(this->keyword);
		if(this->description)
			free(this->description);
		free(this);
	}

if (obj)
	free(obj);
}

/* read contents of a text file, and place in buf */
int file_to_string(char *name, char *buf)
{
	FILE *fl;
	char tmp[128];

	*buf = '\0';

	if (!(fl = fopen(name, "r")))
	{
		sprintf(tmp,"[%s] file-to-string",name);
		perror(tmp);
		*buf = '\0';
		return(-1);
	}

	do
	{
		fgets(tmp, 99, fl);

		if (!feof(fl))
		{
			if (strlen(buf) + strlen(tmp) + 2 > MAX_STRING_LENGTH)
			{
				log("fl->strng: string too big (db.c, file_to_string)");
				*buf = '\0';
				fclose(fl);
				return(-1);
			}

			strcat(buf, tmp);
			*(buf + strlen(buf) + 1) = '\0';
			*(buf + strlen(buf)) = '\r';
		}
	}
	while (!feof(fl));

	fclose(fl);

	return(0);
}


void ClearDeadBit(struct char_data *ch)
{

  FILE *fl;
  struct char_file_u st;

  fl = fopen(PLAYER_FILE, "r+");
  if (!fl) {
    perror("player file");
    exit(0);
  }

  rewind(fl);
  fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
  fread(&st, sizeof(struct char_file_u), 1, fl);
  /*
   **   this is a serious kludge, and must be changed before multiple
   **   languages can be implemented
   */
  if (st.talks[2]) {
    st.talks[2] = 0;  /* fix the 'resurrectable' bit */
    rewind(fl);
    fseek(fl, ch->desc->pos * sizeof(struct char_file_u), 0);
    fwrite(&st, sizeof(struct char_file_u), 1, fl);
    ch->player.talks[2] = 0;  /* fix them both */
  }
  fclose(fl);


}

/* clear some of the the working variables of a char */
void reset_char(struct char_data *ch)
{
  char buf[100];
  struct affected_type *af;
  extern struct dex_app_type dex_app[];

  int i;

  for (i = 0; i < MAX_WEAR; i++) /* Initializing */
    ch->equipment[i] = 0;

  ch->followers = 0;
  ch->master = 0;
  ch->carrying = 0;
  ch->next = 0;

  ch->immune = 0;
  ch->M_immune = 0;
  ch->susc = 0;
  ch->mult_att = 1.0;

  if (!GET_RACE(ch))
    GET_RACE(ch) = RACE_HUMAN;

  if ((ch->player.class == 3) && (GET_LEVEL(ch, THIEF_LEVEL_IND))) {
    ch->player.class = 8;
    send_to_char("Setting your class to THIEF only.\n\r", ch);
  }

  for (i=0;i<MAX_CLASS;i++) {
    if (GET_LEVEL(ch, i) > BIG_GUY) {
      GET_LEVEL(ch,i) = 51;
    }
  }

  /* Setting Bard level (GH) */

  SET_BIT(ch->specials.act, PLR_ECHO);

  ch->hunt_dist = 0;
  ch->hatefield = 0;
  ch->fearfield = 0;
  ch->hates.clist = 0;
  ch->fears.clist = 0;

  /* AC adjustment */
  GET_AC(ch) = 100;

  GET_HITROLL(ch)=0;
  GET_DAMROLL(ch)=0;

  ch->next_fighting = 0;
  ch->next_memorize = 0;
  ch->next_in_room = 0;
  ch->specials.fighting = 0;
  ch->specials.position = POSITION_STANDING;
  ch->specials.default_pos = POSITION_STANDING;
  ch->specials.carry_weight = 0;
  ch->specials.carry_items = 0;
  ch->specials.spellfail = 101;
  ch->specials.questwon = 0;

  if (GET_HIT(ch) <= 0)
    GET_HIT(ch) = 1;
  if (GET_MOVE(ch) <= 0)
    GET_MOVE(ch) = 1;
  if (GET_MANA(ch) <= 0)
    GET_MANA(ch) = 1;

  ch->points.max_mana = 0;
  ch->points.max_move = 0;

  if (IS_IMMORTAL(ch)) {
    GET_BANK(ch) = 0;
    GET_GOLD(ch) = 100000;
  }

  if (GET_BANK(ch) > GetMaxLevel(ch)*100000) {
    sprintf(buf, "%s has %s coins in bank.", GET_NAME(ch), formatNum(GET_BANK(ch)));
    log(buf);
  }
  if (GET_GOLD(ch) > GetMaxLevel(ch)*100000) {
    sprintf(buf, "%s has %s coins.", GET_NAME(ch), formatNum(GET_GOLD(ch)));
    log(buf);
  }

  /*
    Class specific Stuff
    */

  ClassSpecificStuff(ch);


  if (HasClass(ch, CLASS_MONK)) {
    GET_AC(ch) -= MIN(150, (GET_LEVEL(ch, MONK_LEVEL_IND)*5));
    ch->points.max_move += GET_LEVEL(ch, MONK_LEVEL_IND);
  }

  /*
    racial stuff
    */
  SetRacialStuff(ch);

/*
  update the affects on the character.
*/
	/* we try and use default set in .aux file */
	if (ch->specials.sev >9 || ch->specials.sev <0)
	     ch->specials.sev = 0;

  for(af = ch->affected; af; af=af->next)
    affect_modify(ch, af->location, af->modifier, af->bitvector,
		  TRUE);
  affect_total(ch);
  if (!HasClass(ch, CLASS_MONK))
      GET_AC(ch) += dex_app[GET_DEX(ch)].defensive;


/* could add barbarian double dex bonus here.... ... Nah! */

  if (GET_AC(ch) > 100)
      GET_AC(ch) = 100;


/*
  clear out the 'dead' bit on characters
*/
  if (ch->desc)
    ClearDeadBit(ch);
/*
   Clear out berserk flags case there was a crash in a fight
*/
  if (IS_SET(ch->specials.affected_by2,AFF2_BERSERK)){
      REMOVE_BIT(ch->specials.affected_by2,AFF2_BERSERK);
      }

	if (IS_SET(ch->specials.affected_by2,AFF2_STYLE_BERSERK)){
	      REMOVE_BIT(ch->specials.affected_by2,AFF2_STYLE_BERSERK);
	}

/*
   Clear out MAILING flags case there was a crash
*/
  if (IS_SET(ch->specials.act,PLR_MAILING))
	REMOVE_BIT(ch->specials.act,PLR_MAILING);

/*
	Clear out edit flags
*/
	if (IS_SET(ch->player.user_flags,CAN_OBJ_EDIT))
		REMOVE_BIT(ch->player.user_flags,CAN_OBJ_EDIT);
	if (IS_SET(ch->player.user_flags,FAST_AREA_EDIT))
		REMOVE_BIT(ch->player.user_flags,FAST_AREA_EDIT);
	if (IS_SET(ch->player.user_flags,FAST_MAP_EDIT))
		REMOVE_BIT(ch->player.user_flags,FAST_MAP_EDIT);

/*
  Clear out order/AFK flags
*/

	if (IS_SET(ch->specials.affected_by2,AFF2_CON_ORDER))
	  REMOVE_BIT(ch->specials.affected_by2,AFF2_CON_ORDER);

	if (IS_AFFECTED2(ch,AFF2_AFK))
	  REMOVE_BIT(ch->specials.affected_by2,AFF2_AFK);

	if (IS_AFFECTED2(ch,AFF2_QUEST))
	  REMOVE_BIT(ch->specials.affected_by2,AFF2_QUEST);

/*
	Remove bogus flags on mortals
*/

if (IS_SET(ch->specials.act,PLR_NOHASSLE) && GetMaxLevel(ch) < LOW_IMMORTAL) {
	REMOVE_BIT(ch->specials.act,PLR_NOHASSLE);
}

		/* check spells and if lower than 95 remove special flag */
 if (!IS_IMMORTAL(ch))
  for (i=0;i<MAX_SKILLS-1;i++) {
    if (ch->skills[i].learned < 95 || !IS_SET(ch->skills[i].flags,SKILL_KNOWN))
       ch->skills[i].special = 0;
    }

  for (i=WEAPON_FIRST;i<=WEAPON_LAST;i++) {
    if (ch->skills[i].learned < 95 || !IS_SET(ch->skills[i].flags,SKILL_KNOWN))
       ch->skills[i].special = 0;
    }

  SetDefaultLang(ch);


	/* to correct bogus races... */
if (GET_RACE(ch) > MAX_RACE || GET_RACE(ch) < 0)
	GET_RACE(ch)=1;

// Don't want the imps to be deleted, do you?
if (!strcmp(GET_NAME(ch),"Banon")) {
	GET_LEVEL(ch,0) = 60;
	}

if (!(strcmp(GET_NAME(ch),"Lennya")) || !(strcmp(GET_NAME(ch), "Tsaron"))) {
	GET_LEVEL(ch,0)=59;
	}
//End of this modif... -Manwe Windmaster 260697 */

/* this is to clear up bogus levels on people that where here before */
/* these classes where made... */


if (!HasClass(ch,CLASS_MAGIC_USER))
	ch->player.level[0] = 0;
if (!HasClass(ch,CLASS_CLERIC))
	ch->player.level[1] = 0;
if (!HasClass(ch,CLASS_THIEF))
	ch->player.level[3] =0;
if (!HasClass(ch,CLASS_WARRIOR))
	ch->player.level[2] = 0;
if (!HasClass(ch,CLASS_DRUID))
	ch->player.level[4] = 0;
if (!HasClass(ch,CLASS_MONK))
	ch->player.level[5] = 0;
if (!HasClass(ch,CLASS_BARBARIAN))
	ch->player.level[6] = 0;
if (!HasClass(ch,CLASS_SORCERER))
   ch->player.level[7] = 0;
if (!HasClass(ch,CLASS_PALADIN))
   ch->player.level[8]=0;
if (!HasClass(ch,CLASS_RANGER))
   ch->player.level[9]=0;
if (!HasClass(ch,CLASS_PSI))
   ch->player.level[10]=0;
if (!HasClass(ch,CLASS_BARD))
   ch->player.level[11]=0;
if (!HasClass(ch,CLASS_NECROMANCER))
   ch->player.level[12]=0;


ch->player.has_mail=FALSE;

}               /* end */



/* clear ALL the working variables of a char and do NOT free any space alloc'ed*/
void clear_char(struct char_data *ch)
{
	memset(ch, 0, sizeof(struct char_data));

	ch->in_room = NOWHERE;
	ch->specials.was_in_room = NOWHERE;
	ch->specials.position = POSITION_STANDING;
	ch->specials.default_pos = POSITION_STANDING;
	GET_AC(ch) = 100; /* Basic Armor */
}


void clear_object(struct obj_data *obj)
{
	memset(obj, 0, sizeof(struct obj_data));

	obj->item_number = -1;
	obj->in_room      = NOWHERE;
	obj->eq_pos       = -1;
}




/* initialize a new character only if class is set */
void init_char(struct char_data *ch)
{
  int i;

  /* *** if this is our first player --- he be God *** */

  if (top_of_p_table < 0)       {

    slog("Building FIRST CHAR, setting up IMPLEMENTOR STATUS!") ;

    GET_EXP(ch) = 24000000;
    GET_LEVEL(ch,0) = IMPLEMENTOR;
    ch->points.max_hit = 1000;

/* set all levels */

	 for (i=0;i<MAX_CLASS;i++) {
	  if (GET_LEVEL(ch,i) < GetMaxLevel(ch))
	      GET_LEVEL(ch,i) = GetMaxLevel(ch);
	  }/* for */

/* set all classes */
	  for (i=1;i<=CLASS_NECROMANCER;i*=2) {
	    if (!HasClass(ch,i))
	       ch->player.class +=i;
	  } /* for */

  } /* end implmentor setup */

  set_title(ch);

  ch->player.short_descr = 0;
  ch->player.long_descr = 0;
  ch->player.description = 0;

  ch->player.hometown = number(1,4);

  ch->player.time.birth = time(0);
  ch->player.time.played = 0;
  ch->player.time.logon = time(0);

  for (i = 0; i < MAX_TOUNGE; i++)
    ch->player.talks[i] = 0;

  GET_STR(ch) = 11;
  GET_INT(ch) = 11;
  GET_WIS(ch) = 11;
  GET_DEX(ch) = 11;
  GET_CON(ch) = 11;
  GET_CHR(ch) = 11;

  /* make favors for sex */
  if (GET_RACE(ch) == RACE_HUMAN) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    } else {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
  } else if (GET_RACE(ch) == RACE_DWARF ||
        GET_RACE(ch)==RACE_ROCK_GNOME ||
        GET_RACE(ch) == RACE_FOREST_GNOME){
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120,180);
      ch->player.height = number(100,150);
    } else {
      ch->player.weight = number(100,160);
      ch->player.height = number(100,150);
    }
  } else if (GET_RACE(ch) == RACE_HALFLING) {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(70,120);
      ch->player.height = number(80,120);
    } else {
      ch->player.weight = number(60,110);
      ch->player.height = number(70,115);
    }

  } else if (GET_RACE(ch) == RACE_MOON_ELF  ||
             GET_RACE(ch) == RACE_DROW      ||
             GET_RACE(ch) == RACE_GOLD_ELF  ||
             GET_RACE(ch) == RACE_WILD_ELF  ||
             GET_RACE(ch) == RACE_SEA_ELF   ||
             GET_RACE(ch) == RACE_AVARIEL   )
  {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(100,150);
      ch->player.height = number(160,200);
    } else {
      ch->player.weight = number(80,130);
      ch->player.height = number(150,180);
    }
  }  else   if (GET_RACE(ch) == RACE_HALF_ELF)
  {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(110,160);
      ch->player.height = number(140,180);
    } else {
      ch->player.weight = number(90,150);
      ch->player.height = number(140,170);
    }
}
  else   if (GET_RACE(ch) == RACE_HALF_OGRE)
  {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(200,400);
      ch->player.height = number(200,230);
    } else {
      ch->player.weight = number(180,350);
      ch->player.height = number(190,220);
    }
}
 else   if (GET_RACE(ch) == RACE_HALF_ORC)
  {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    } else {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
}  else
  if (GET_RACE(ch) == RACE_HALF_GIANT)
  {
    if (ch->player.sex == SEX_MALE) {
      ch->player.weight = number(300,900);
      ch->player.height = number(300,400);
    } else {
      ch->player.weight = number(250,800);
      ch->player.height = number(290,350);
    }
}
 else {
    if (ch->player.sex == SEX_MALE)
    {
      ch->player.weight = number(120,180);
      ch->player.height = number(160,200);
    } else
    {
      ch->player.weight = number(100,160);
      ch->player.height = number(150,180);
    }
 }

  ch->points.mana = GET_MAX_MANA(ch);
  ch->points.hit = GET_MAX_HIT(ch);
  ch->points.move = GET_MAX_MOVE(ch);

  ch->points.armor = 100;

  if (!ch->skills)
    SpaceForSkills(ch);

  for (i = 0; i <= MAX_SKILLS - 1; i++) {
    if (GetMaxLevel(ch) <IMPLEMENTOR) {
      ch->skills[i].learned = 0;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 0;
      ch->skills[i].nummem  = 0;
    }   else {
      ch->skills[i].learned = 100;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 1;
      ch->skills[i].nummem  = 0;
    }
  }

  for (i = WEAPON_FIRST; i <= WEAPON_LAST; i++) {
    if (GetMaxLevel(ch) <IMPLEMENTOR) {
      ch->skills[i].learned = 0;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 0;
      ch->skills[i].nummem  = 0;
    }   else {
      ch->skills[i].learned = 100;
      ch->skills[i].flags   = 0;
      ch->skills[i].special = 1;
      ch->skills[i].nummem  = 0;
    }
  }


  ch->specials.affected_by = 0;
  ch->specials.spells_to_learn = 0;

  for (i = 0; i < MAX_SAVES; i++)
    ch->specials.apply_saving_throw[i] = 0;

  for (i = 0; i < 3; i++)
    GET_COND(ch, i) = (GetMaxLevel(ch) > GOD ? -1 : 24);
}

/*
**  this duplicates the code in room_find, because it is much quicker this way.
*/
struct room_data *real_roomp(long virtual)
{
#if HASH
  return hash_find(&room_db, virtual);
#else
   return((virtual<WORLD_SIZE&&virtual>-1)?room_db[virtual]:0);
#endif
}

/* returns the real number of the monster with given virtual number */
int real_mobile(int virtual)
{
  int bot, top, mid;

  bot = 0;
  top = top_of_sort_mobt;

  /* perform binary search on mob-table */
  for (;;)
    {
      mid = (bot + top) / 2;

      if ((mob_index + mid)->virtual == virtual)
	return(mid);
      if (bot >= top) {
	/* start unsorted search now */
	for(mid=top_of_sort_mobt;mid<top_of_mobt;mid++)
	  if((mob_index + mid)->virtual == virtual)
	    return(mid);
	return(-1);
      }
      if ((mob_index + mid)->virtual > virtual)
	top = mid - 1;
      else
	bot = mid + 1;
    }
}






/* returns the real number of the object with given virtual number */
int real_object(int virtual)
{
  long bot, top, mid;
  char buf[150];
  bot = 0;
  top = top_of_sort_objt;
  /* perform binary search on obj-table */
  for (;;)    {
      mid = (bot + top) / 2;

      if ((obj_index + mid)->virtual == virtual)
	return(mid);
      if (bot >= top) {
	/* start unsorted search now */
	for(mid=top_of_sort_objt;mid<top_of_objt;mid++) {
	  if((obj_index + mid)->virtual == virtual)
	    return(mid);
	  }
	return(-1);
      }
      if ((obj_index + mid)->virtual > virtual)
	top = mid - 1;
      else
	bot = mid + 1;
    }
}

int ObjRoomCount(int nr, struct room_data *rp)
{
  struct obj_data *o;
  int count = 0;

  for (o=rp->contents;o;o=o->next_content) {
    if (o->item_number == nr) {
      count++;
    }
  }
  return(count);
}

int str_len(char *buf)
{
  int i = 0;
  for(i; buf[i] != '\0'; i++);
  return(i);
}


int load()
{
  return(0);
}

void gr()
{
  return;
}

int workhours()
{
  return(0);
}

void reboot_text(struct char_data *ch, char *arg, int cmd)
{
  char buffer[100];
  struct char_data *p;
  int i;

 if(IS_NPC(ch))
    return;

 log("Rebooting Essential Text Files.");

 file_to_string(NEWS_FILE, news);
 file_to_string(CREDITS_FILE, credits);
 file_to_string(MOTD_FILE, motd);
 file_to_string("wizmotd", wmotd);
 log("Initializing Scripts.");
 InitScripts();

/* jdb -- you don't appear to re-install the scripts after you
   reset the script db
   */

  for (p = character_list;p;p=p->next) {
    for(i = 0; i < top_of_scripts; i++) {
      if(script_data[i].virtual == mob_index[p->nr].virtual) {
	SET_BIT(p->specials.act, ACT_SCRIPT);
	sprintf(buffer, "Setting SCRIPT bit for mobile %s, file %s.", GET_NAME(p), script_data[i].filename);
	log(buffer);
	p->script = i;
	break;
      }
    }
  }
 return;
}


void InitScripts()
{
 char buf[255], buf2[255];
 FILE *f1, *f2;
 int i, count;
 struct char_data *mob;

 if(!script_data)
    top_of_scripts = 0;

return;
/* DISABLED */

 /* what is ths for?  turn off all the scripts ??? */
 /* -yes, just in case the script file was removed, saves pointer probs */

 for(mob = character_list; mob; mob = mob->next)
    if(IS_MOB(mob) && IS_SET(mob->specials.act, ACT_SCRIPT)) {
       mob->commandp = 0;
       REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
     }

 if(!(f1 = fopen("scripts.dat", "r"))) {
    log("Unable to open file \"scripts.dat\".");
    return;
  }

 if(script_data) {
    int i = 0;
    for(;i < top_of_scripts; i++) {
	if (script_data[i].script)
       free(script_data[i].script);
	if (script_data[i].filename)
       free(script_data[i].filename);
     }
if (script_data)
    free(script_data);
    top_of_scripts = 0;
  }


 script_data = NULL;
 script_data = (struct scripts *)malloc(sizeof(struct scripts));

 while(1) {
    if(fgets(buf, 254, f1) == NULL)
       break;

    if(buf[strlen(buf) - 1] == '\n')
       buf[strlen(buf) - 1] = '\0';

    sscanf(buf, "%s %d", buf2, &i);

    sprintf(buf, "scripts/%s", buf2);
    if(!(f2 = fopen(buf, "r"))) {
       sprintf(buf, "Unable to open script \"%s\" for reading.", buf2);
       log(buf);
     }

     else {

       script_data = (struct scripts *) realloc(script_data, (top_of_scripts + 1) * sizeof(struct scripts));

       count = 0;
       while(!feof(f2)) {
	   fgets(buf, 254, f2);
	   if(buf[strlen(buf) - 1] == '\n')
	      buf[strlen(buf) - 1] = '\0';
/* you really don't want to do a lot of reallocs all at once
*/
	   if (count==0) {
	     script_data[top_of_scripts].script = (struct foo_data *) malloc(script_data[top_of_scripts].script, sizeof(struct foo_data));
	   } else  {
	     script_data[top_of_scripts].script = (struct foo_data *) realloc(script_data[top_of_scripts].script, sizeof(struct foo_data) * (count + 1));
	   }
	   script_data[top_of_scripts].script[count].line = (char *) malloc(sizeof(char) * (strlen(buf) + 1));

	   strcpy(script_data[top_of_scripts].script[count].line, buf);

	   count++;
	 }

       script_data[top_of_scripts].virtual = i;
       script_data[top_of_scripts].filename = (char *) malloc((strlen(buf2) + 1) * sizeof(char));
       strcpy(script_data[top_of_scripts].filename, buf2);
       sprintf(buf, "Script %s assigned to mobile %d.", buf2, i);
       log(buf);
       top_of_scripts++;
       fclose(f2);
     }
  }

  if(top_of_scripts)
     sprintf(buf, "%d scripts assigned.", top_of_scripts);
  else
     sprintf(buf, "No scripts found to assign.");
  log(buf);

  fclose(f1);
}

int CheckKillFile(long virtual)
{
 FILE *f1;
 char buf[255];
 int i;

 if(!(f1 = fopen(killfile, "r"))) {
    log("Unable to find killfile.");
    exit(0);
  }

 while(fgets(buf, 254, f1) != NULL) {
    sscanf(buf, "%d", &i);
    if(i == virtual) {
       fclose(f1);
       return(1);
     }
  }

 fclose(f1);
 return(0);
}

void ReloadRooms()
{
 int i;

 for(i = 0; i < number_of_saved_rooms; i++)
/*    load_room_objs(saved_rooms[i]) */ ;
}


void SaveTheWorld()
{
#if SAVEWORLD

  static int ctl=0;
  char cmd, buf[80];
  int i, j, arg1, arg2, arg3;
  struct char_data *p;
  struct obj_data *o;
  struct room_data *room;
  FILE *fp;

  if (ctl == WORLD_SIZE) ctl = 0;

  sprintf(buf, "world/mobs.%d", ctl);
  fp = (FILE *)fopen(buf, "w");  /* append */

  if (!fp)
  {
    log("Unable to open zone writing file.");
    return;
  }

  i = ctl;
  ctl += 1000;

  for (; i< ctl; i++) {
    room = real_roomp(i);
    if (room && !IS_SET(room->room_flags, DEATH)) {
      /*
       *  first write out monsters
       */
      for (p = room->people; p; p = p->next_in_room) {
	if (!IS_PC(p)) {
	  cmd = 'M';
	  arg1 = MobVnum(p);
	  arg2 = mob_index[p->nr].number;
	  arg3 = i;
	  Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);
	  fprintf(fp, "Z 1 %d 1\n", p->specials.zone);

	  /* save hatreds && fears */
	  if (IS_SET(p->hatefield, HATE_SEX))
	    fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->hates.sex);
	  if (IS_SET(p->hatefield, HATE_RACE))
	    fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->hates.race);
	  if (IS_SET(p->hatefield, HATE_GOOD))
	    fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->hates.good);
	  if (IS_SET(p->hatefield, HATE_EVIL))
	    fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->hates.evil);
	  if (IS_SET(p->hatefield, HATE_CLASS))
	    fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->hates.class);
	  if (IS_SET(p->hatefield, HATE_VNUM))
	    fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->hates.vnum);

	  if (IS_SET(p->fearfield, FEAR_SEX))
	    fprintf(fp, "H 1 %d %d -1\n", OP_SEX, p->fears.sex);
	  if (IS_SET(p->fearfield, FEAR_RACE))
	    fprintf(fp, "H 1 %d %d -1\n", OP_RACE, p->fears.race);
	  if (IS_SET(p->fearfield, FEAR_GOOD))
	    fprintf(fp, "H 1 %d %d -1\n", OP_GOOD, p->fears.good);
	  if (IS_SET(p->fearfield, FEAR_EVIL))
	    fprintf(fp, "H 1 %d %d -1\n", OP_EVIL, p->fears.evil);
	  if (IS_SET(p->fearfield, FEAR_CLASS))
	    fprintf(fp, "H 1 %d %d -1\n", OP_CLASS, p->fears.class);
	  if (IS_SET(p->fearfield, FEAR_VNUM))
	    fprintf(fp, "H 1 %d %d -1\n", OP_VNUM, p->fears.vnum);

	  for (j = 0; j<MAX_WEAR; j++) {
	    if (p->equipment[j]) {
	      if (p->equipment[j]->item_number >= 0) {
		cmd = 'E';
		arg1 = ObjVnum(p->equipment[j]);
		arg2 = obj_index[p->equipment[j]->item_number].number;
		arg3 = j;
		strcpy(buf, p->equipment[j]->short_description);
		Zwrite(fp, cmd,1,arg1, arg2, arg3,
		       buf);
		RecZwriteObj(fp, p->equipment[j]);
	      }
	    }
	  }
	  for (o = p->carrying; o; o=o->next_content) {
	    if (o->item_number >= 0) {
	      cmd = 'G';
	      arg1 = ObjVnum(o);
	      arg2 = obj_index[o->item_number].number;
	      arg3 = 0;
	      strcpy(buf, o->short_description);
	      Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
	      RecZwriteObj(fp, o);
	    }
	  }
	}
      }
    }
  }
  fprintf(fp, "S\n");
  fclose(fp);

#endif
}

int ReadTextZone( FILE *fl)
{
  while (1){
    char c, buf[255], count=0, last_cmd=1;
    int i, j, k, tmp, zone=0;
    struct char_data *mob, *master;
    struct room_data *rp;
    struct obj_data *obj, *obj_to;

    count++;
    fscanf(fl, " "); /* skip blanks */
    fscanf(fl, "%c", &c);


    if (c == 'S' || c == EOF)
      break;

    if (c == '*'){
      fgets(buf, 80, fl); /* skip command */
      continue;
    }

    fscanf(fl, " %d %d %d", &tmp, &i, &j);
    if (c == 'M' || c == 'O' || c == 'C' || c == 'E' || c == 'P' || c == 'D')
      fscanf(fl, " %d", &k);

    fgets(buf, 80, fl);/* read comment */
    if(last_cmd || tmp <= 0)
      switch(c) {
      case 'M': /* read a mobile */
	i = real_mobile(i);
	if ((mob_index[i].number < j) && !CheckKillFile(mob_index[i].virtual)) {
	  mob = read_mobile(i, REAL);
	  char_to_room(mob, k);

	  last_cmd = 1;
	  master = mob;
	} else
	  last_cmd = 0;
	break;

      case 'C': /* read a mobile.  Charm them to follow prev. */
	i = real_mobile(i);
	if ((mob_index[i].number < j) && !CheckKillFile(mob_index[i].virtual)){
	  mob = read_mobile(i, REAL);
	  if (master) {
	    char_to_room(mob, master->in_room);
	    /*
	      add the charm bit to the dude.
	      */
	    add_follower(mob, master);
	    SET_BIT(mob->specials.affected_by, AFF_CHARM);
	    SET_BIT(mob->specials.act, k);
	  } else {
	    extract_char(mob);
	    last_cmd = 0;
	  }
	  last_cmd = 1;
	}
	else
	  last_cmd = 0;
	break;

      case 'Z':  /* set the last mobile to this zone */
	if (mob) {
	  mob->specials.zone =i;

	  if (GET_RACE(mob)>RACE_ROCK_GNOME)
	    if (!strchr(zone_table[i].races, GET_RACE(mob))) {
	      zone_table[i].races[strlen(zone_table[i].races)] =
		GET_RACE(mob);
	    }
	  last_cmd = 1;
	} else {
	  last_cmd = 0;
	}
	break;

      case 'O': /* read an object */
	i = real_object(i);
	if (obj_index[i].number < j) {
	  if (j >= 0 && ((rp = real_roomp(j)) != NULL)) {
	    if((tmp>0&&ObjRoomCount(i,rp)<tmp) ||
	       (tmp<=0&&ObjRoomCount(i,rp)<(-tmp)+1)){
	      if ((obj = read_object(i, REAL)) != NULL) {
		obj_to_room(obj, k);
		last_cmd = 1;
	      } else {
		last_cmd = 0;
	      }
	    } else {
	      last_cmd = 0;
	    }
	  } else if (obj = read_object(i, VIRTUAL)) {
	    sprintf(buf, "Error finding room #%d", k);
	    log(buf);
	    last_cmd = 1;
	  }  else {
	    last_cmd = 0;
	  }
	}
	break;

      case 'P': /* object to object */
	i = real_object(i);
	if (obj_index[i].number < j)  {
	  obj = read_object(i, VIRTUAL);
	  obj_to = get_obj_num(k);
	  if (obj_to && obj) {
	    obj_to_obj(obj, obj_to);
	    last_cmd = 1;
	  } else {
	    last_cmd = 0;
	  }
	} else
	  last_cmd = 0;
	break;

      case 'G': /* obj_to_char */
	i = real_object(i);
	if (obj_index[i].number < j &&
	    (obj = read_object(i, REAL)) && mob) {
	  obj_to_char(obj, mob);
	  last_cmd = 1;
#ifndef NEW_RENT
	} else {
	  last_cmd = 0;
#endif
	}
	break;

      case 'H': /* hatred to char */

	if (AddHatred(mob, i, j))
	    last_cmd = 1;
	else
	    last_cmd = 0;
	break;

      case 'F': /* fear to char */

	if (AddFears(mob, i, j))
	    last_cmd = 1;
	else
	    last_cmd = 0;
	break;

      case 'E': /* object to equipment list */
	i = real_object(i);
	if (obj_index[i].number < j &&
	    (obj = read_object(i, REAL))) {
	  if (!mob->equipment[k]) {
	    equip_char(mob, obj, k);
	  } else {
	    sprintf(buf,"eq error - zone %d, cmd %d, item %d, mob %d, loc %d",
		    zone, 1, obj_index[i].virtual,
		    mob_index[mob->nr].virtual, k);
	    log_sev(buf, 6);
	  }
	  last_cmd = 1;
	}    else
	  last_cmd = 0;
	break;

      case 'D': /* set state of door */
	rp = real_roomp(i);
	if (rp && rp->dir_option[j]) {
	    switch (k) {
	    case 0:
		  REMOVE_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
		      REMOVE_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
		      break;
		case 1:
		      SET_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
		      REMOVE_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
		      break;
		case 2:
		      SET_BIT(rp->dir_option[j]->exit_info, EX_LOCKED);
		      SET_BIT(rp->dir_option[j]->exit_info, EX_CLOSED);
		      break;
		}
	      last_cmd = 1;
	  } else {
	      /* that exit doesn't exist anymore */
	  }
	break;

      default:
	break;
      }
    }

}

void clean_playerfile()
{
  struct junk {
    struct char_file_u dummy;
    bool AXE;
  };

  struct junk grunt;

  time_t timeH;
  char buf[80];
  FILE *f,*f2;
  int i,j,max, num_processed, num_deleted, num_demoted, ones;

  num_processed = num_deleted = num_demoted = ones = 0;
  timeH=time(0);

  if (!(f = fopen(PLAYER_FILE, "rb+")))        {
    perror("clean player file");
    exit(0);
  }

  if (!(f2 = fopen("temp", "w+")))        {
    perror("clean player file");
    exit(0);
  }


  for (;!feof(f);) {
    fread(&(grunt.dummy), sizeof(struct char_file_u), 1, f);
     if (!feof(f)) {           /* we have someone */
       num_processed++;
       grunt.AXE = FALSE;
       if(!str_cmp(grunt.dummy.name,"111111")) {
	 sprintf(buf,"%s was deleted (111111 name hopefully).",
		 grunt.dummy.name);
	 log(buf);
	 ones++;
	 num_deleted++;
	 grunt.AXE = TRUE;
       } else {
	 for(j=0,max=0;j<MAX_CLASS;j++)
	  if(grunt.dummy.level[j] > max)
	     max = grunt.dummy.level[j];
	 if(max < LOW_IMMORTAL) {
	   j=1;
	   if(max > 15)
	     j++;
	   if(max > 30)
	     j++;
	   if(max > 45)
	     j++;


#if CHECK_RENT_INACTIVE
#ifndef STRANGE_WACK

	   /* Purge rent files! after inactivity of 1 month */
 if(!grunt.AXE &&
    timeH-grunt.dummy.last_logon > (long) RENT_INACTIVE*(SECS_PER_REAL_DAY*30))
	     {
		char uname[50];

		sprintf(uname,"%s",grunt.dummy.name);
		uname[0]=tolower(uname[0]);

		sprintf(buf,"Purging rent file for %s, inactive for %d month.",
		uname,RENT_INACTIVE);
		log(buf);
		sprintf(buf,"rm rent/%s",uname);
		system(buf);
		sprintf(buf,"rm rent/%s.aux",grunt.dummy.name);
		system(buf);
	   }

#endif
#endif

#ifndef STRANGE_WACK
	   if(!grunt.AXE &&
	      timeH-grunt.dummy.last_logon > (long) j*(SECS_PER_REAL_DAY*30) &&
		!IS_SET(grunt.dummy.user_flags,NO_DELETE))      {
		     num_deleted++;
	     grunt.AXE = TRUE;
	     sprintf(buf,"%s deleted after %d months of inactivity.",
		     grunt.dummy.name,j);
	     log(buf);
       }        /* even the no_deletes get deleted after a time */

	if (IS_SET(grunt.dummy.user_flags,NO_DELETE))   {
	   if(timeH-grunt.dummy.last_logon > (long) (j*2)*(SECS_PER_REAL_DAY*30))        {
		     num_deleted++;
	     grunt.AXE = TRUE;
	     sprintf(buf,"%s deleted after %d months of inactivity.(NO_DELETE)",
		     grunt.dummy.name,j);
	     log(buf);
	    }
	  }


	 } else
	 if(max > LOW_IMMORTAL) {

	/* delete people with levels greater than BIG_GUY */
	       if (max > BIG_GUY) {
	     num_deleted++;
	     grunt.AXE = TRUE;
	     sprintf(buf,"%s deleted after %d months of inactivity.(TOHIGHLEVEL)",
		     grunt.dummy.name,j);
	     log(buf);
	       } else

	   if(timeH-grunt.dummy.last_logon > (long) SECS_PER_REAL_DAY*30) {
	     num_demoted++;
	     sprintf(buf,"%s demoted from %d to %d due to inactivity.",
		     grunt.dummy.name,max,max-1);
	     log(buf);
	     grunt.dummy.last_logon = timeH; /* so it doesn't happen twice */
	     max--;
	     max = MAX(51,max); /* should not be necessary */
			/* 6 */
	     for(j=0;j<MAX_CLASS;j++) grunt.dummy.level[j]=max;

	   }

	 }
       }
#endif

#if STRANGE_WACK
 } /* level < LOW_IMMORT */

/* used to clear up corrupted players files, bogus chars in the name... etc.. */

for(i=0;i<strlen(grunt.dummy.name);i++)
 if(!(toupper(grunt.dummy.name[i])>='A'&&toupper(grunt.dummy.name[i])<='Z')){
   sprintf(buf,"%s was deleted (strange name).",grunt.dummy.name);
   log(buf);
   grunt.AXE=1;
   break;
 }

	/* used to clear chars with bogus levels and such */
if(!grunt.AXE)
 for(i=0;i<MAX_CLASS;i++)
  if(grunt.dummy.level[i]<0) {
   sprintf(buf,"%s was deleted (strange levels).",grunt.dummy.name);
   log(buf);
   grunt.AXE=1;
   break;
  }

#endif

      if(!grunt.AXE)
	 fwrite(&(grunt.dummy), sizeof(struct char_file_u), 1, f2);
     }
  }

#if STRANGE_WACK
	}
#endif

  sprintf(buf,"-- %d characters were processed.", num_processed);
  log(buf);
  sprintf(buf,"-- %d characters were deleted.  ", num_deleted);
  log(buf);
  sprintf(buf,"-- %d of these were allread deleted. (11111s)", ones);
  log(buf);
  sprintf(buf,"-- %d gods were demoted due to inactivity.", num_demoted);
  log(buf);
  sprintf(buf,"mv %s %s.bak", PLAYER_FILE, PLAYER_FILE);
  system(buf);
  sprintf(buf,"mv temp %s", PLAYER_FILE);
  system(buf);
  log("Cleaning done.");

}
