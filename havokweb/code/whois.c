/*  Whois code
 *  By: Greg Hovey
 *  Designed for Havok and should work on other dalemuds out there.
 *  Says name, last time online and Email address.
 *  I basically just finished of the daleMud function...
 *  If you have any questions.. Feel free email me at hoveyg@hotmail.com
 */

void do_finger(struct char_data *ch, char *argument, int cmd)
{
  char name[128],buf[254];
  struct char_data *temp = 0;
  struct char_data *finger = 0;
  struct char_file_u tmp_store;

  dlog("in do_finger");

  argument= one_argument(argument,name);


  if (!*name) {
    send_to_char("Whois whom?!?!\n\r",ch);
    return;
  }

  CREATE(finger, struct char_data,1);
  clear_char(finger);

  if(load_char(name, &tmp_store) > -1) {
    temp = finger;
    store_to_char(&tmp_store, finger);
    load_char_extra(finger);

    if (IS_NPC(finger)) {
      send_to_char("No person by that name\n\r",ch);
      return;
    }
    sprintf(buf,"\n\r$c0015%s$c0012's adventurer information:\n\r",
	    GET_NAME(finger));
    send_to_char(buf,ch);
    if(IS_IMMORTAL(finger) && !IS_IMMORTAL(ch))//if vic is immortal & U arn't
      sprintf(buf,"$c0012Last time sited    : $c0015Unknown\n\r");
    else if(get_char(name))
      sprintf(buf,"$c0012Last time sited    : $c0015Currently Playing\n\r");
    else
      sprintf(buf,"$c0012Last time sited    : $c0015%-50s\r"
	      ,asctime(localtime(&tmp_store.last_logon)));
    send_to_char(buf,ch);//act(buf,FALSE,ch,0,0,TO_CHAR);

    if(finger->specials.email==NULL)
      sprintf(buf, "$c0012Known message drop : $c0015None\n\r");
    else
      sprintf(buf, "$c0012Known message drop : $c0015%-60s\n\r"
	      ,GET_EMAIL(finger));

    send_to_char(buf,ch);

  } /* end found finger*/
  else
    send_to_char("Character not found!!\n\r",ch);

  free(finger);

}
