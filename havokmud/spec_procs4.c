#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "protos.h"
/* #include "mail.h" 	 moved to protos.h */

/*   external vars  */
/*   external vars  */
extern struct skillset bardskills[];
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




/* @Name: This is the chest part of the royal rumble proc.
 * @description: Once there is only 1 person left in the arena, the chest will open.
 * @Author: Greg Hovey (Banon)
 * @Assigned to obj/mob/room:
*/
int chestproc(struct char_data *ch, int cmd, char *argument, struct obj_data *obj, int type)
{
     char buf[MAX_INPUT_LENGTH+80];
	 int count;

  if(cmd != 99) //open chest
    return(FALSE);

   dlog("in chestproc");

	if(countPeople(2)>1)  {
		ch_printf(ch,"You can't open the chest yet, there is still people in the battle arena!%d", countPeople(2));
		return(TRUE);
	} else
		return(FALSE);
}


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


int preperationproc(struct char_data *ch, int cmd, char *arg, struct room_data *rp, int type)
{

	  struct char_data *i;
	  int count=0;


	if (cmd!=67)
		return(FALSE);

	for (i=real_roomp(ch->in_room)->people; i; i = i->next_in_room) {
		if(i)
			count++;
	}

	if((countPeople(2)-count )==1){
		send_to_char("1 person in arena.. boot someone new",ch);

	} else
	ch_printf(ch,"Number of people in zone:%d   Number of people in room:%d",countPeople(2), count);


 	return(FALSE);
}

