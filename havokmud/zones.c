void load_one_room(FILE * fl, struct room_data *rp);

void fake_setup_dir(FILE * fl, long room, int dir)
{
    int             tmp;
    char           *temp;

    /* descr */
    temp = fread_string(fl);
    if (temp) {
        free(temp);
    }
    /* key */
    temp = fread_string(fl);
    if (temp) {
        free(temp);
    }
    fscanf(fl, " %d ", &tmp);
    fscanf(fl, " %d ", &tmp);
    fscanf(fl, " %d ", &tmp);
}

void Zwrite(FILE * fp, char cmd, int tf, int arg1, int arg2, int arg3,
            char *desc)
{
    char            buf[100];

    if (*desc) {
        sprintf(buf, "%c %d %d %d %d   ; %s\n", cmd, tf, arg1, arg2, arg3,
                desc);
        fputs(buf, fp);
    } else {
        sprintf(buf, "%c %d %d %d %d\n", cmd, tf, arg1, arg2, arg3);
        fputs(buf, fp);
    }
}

void RecZwriteObj(FILE * fp, struct obj_data *o)
{
    struct obj_data    *t;
    struct index_data  *index;
    LinkedListItem_t   *item;

    if (ITEM_TYPE(o) == ITEM_TYPE_CONTAINER) {
        LinkedListLock( o->containList );
        for( item = o->containList->head; item; item = item->next ) {
            t = CONTAIN_LINK_TO_OBJ(item);
            Zwrite(fp, 'P', 1, t->item_number, t->index->number, o->item_number,
                   t->short_description);
            RecZwriteObj(fp, t);
        }
        LinkedListUnlock( o->containList );
    }
}

int SaveZoneFile(FILE * fp, int start_room, int end_room)
{
    struct char_data *p;
    struct obj_data *o;
    struct room_data *room;
    char            cmd,
                    buf[80];
    int             i,
                    j,
                    arg1,
                    arg2,
                    arg3;
    struct index_data  *index;
    LinkedListItem_t   *item;

    for (i = start_room; i <= end_room; i++) {
        room = roomFindNum(i);
        if (!room) {
            continue;
        }

        /*
         *  first write out monsters
         */
        for (p = room->people; p; p = p->next_in_room) {
            if (IS_NPC(p)) {
                index = mobileIndex(p->nr);
                cmd = 'M';
                arg1 = MobVnum(p);
                arg2 = index->number;
                arg3 = i;
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, p->player.short_descr);

                for (j = 0; j < MAX_WEAR; j++) {
                    if (p->equipment[j] &&
                        p->equipment[j]->item_number >= 0) {
                        cmd = 'E';
                        arg1 = p->equipment[j]->item->number;
                        if (p->equipment[j]->index->MaxCount) {
                            arg2 = p->equipment[j]->index->MaxCount;
                        } else {
                            arg2 = 65535;
                        }
                        arg3 = j;
                        strcpy(buf, p->equipment[j]->short_description);
                        Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
                        RecZwriteObj(fp, p->equipment[j]);
                    }
                }

                for (o = p->carrying; o; o = o->next_content) {
                    if (o->item_number >= 0) {
                        cmd = 'G';
                        arg1 = o->item_number;
                        if (o->index->MaxCount) {
                            arg2 = o->index->MaxCount;
                        } else {
                            arg2 = 65535;
                        }
                        arg3 = 0;
                        strcpy(buf, o->short_description);
                        Zwrite(fp, cmd, 1, arg1, arg2, arg3, buf);
                        RecZwriteObj(fp, o);
                    }
                }
            }
        }

        /*
         *  write out objects in rooms
         */
        LinkedListLock( room->contentList );
        for( item = room->contentList->head; item; item = item->next ) {
            o = CONTENT_LINK_TO_OBJ(item);
            if (o->item_number >= 0) {
                cmd = 'O';
                arg1 = o->item_number;
                if (o->index->MaxCount) {
                    arg2 = o->index->MaxCount;
                } else {
                    arg2 = 65535;
                }
                arg3 = i;
                strcpy(buf, o->short_description);
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, buf);
                RecZwriteObj(fp, o);
            }
        }
        LinkedListUnlock( room->contentList );

        /*
         *  lastly.. doors
         */

        for (j = 0; j < 6; j++) {
            /*
             *  if there is an door type exit, write it.
             */
            if (room->dir_option[j] && room->dir_option[j]->exit_info) {
            /* is a door */
                cmd = 'D';
                arg1 = i;
                arg2 = j;
                arg3 = 0;
                if (IS_SET(room->dir_option[j]->exit_info, EX_CLOSED)) {
                    arg3 = 1;
                }
                if (IS_SET(room->dir_option[j]->exit_info, EX_LOCKED)) {
                    arg3 = 2;
                }
                Zwrite(fp, cmd, 0, arg1, arg2, arg3, room->name);
            }
        }
    }
    fprintf(fp, "S\n");
    return 1;
}

int LoadZoneFile(FILE * fl, int zon)
{
    int             cmd_no = 0,
                    expand,
                    tmp,
                    cc = 22;
    char            buf[81];

    if (zone_table[zon].cmd) {
        free(zone_table[zon].cmd);
    }

    /*
     * read the command table
     */
    cmd_no = 0;
    for (expand = 1; !feof(fl);) {
        if (expand) {
            if (!cmd_no) {
                CREATE(zone_table[zon].cmd, struct reset_com, cc);
            } else if (cmd_no >= cc) {
                cc += 5;
                if (!(zone_table[zon].cmd =
                      (struct reset_com *) realloc(zone_table[zon].cmd,
                                           (cc * sizeof(struct reset_com))))) {
                    perror("reset command load");
                    assert(0);
                }
            }
        }

        expand = 1;

        /* skip blanks */
        fscanf(fl, " ");
        fscanf(fl, "%c", &zone_table[zon].cmd[cmd_no].command);

        if (zone_table[zon].cmd[cmd_no].command == 'S') {
            break;
        }
        if (zone_table[zon].cmd[cmd_no].command == '*') {
            expand = 0;
            /* skip command */
            fgets(buf, 80, fl);
            continue;
        }

        fscanf(fl, " %d %d %d", &tmp, &zone_table[zon].cmd[cmd_no].arg1,
               &zone_table[zon].cmd[cmd_no].arg2);
        zone_table[zon].cmd[cmd_no].if_flag = tmp;

        switch (zone_table[zon].cmd[cmd_no].command) {
        case 'M':
        case 'O':
        case 'C':
        case 'E':
        case 'P':
        case 'D':
            fscanf(fl, " %d", &zone_table[zon].cmd[cmd_no].arg3);
            break;
        }
        /* read comment */
        fgets(buf, 80, fl);
        cmd_no++;
    }
    return 1;
}

void CleanZone(int zone)
{
    struct room_data *rp;
    struct char_data *vict,
                   *next_v;
    struct obj_data *obj,
                   *next_o;
    int             start,
                    end,
                    i;
    struct index_data  *index;
    LinkedListItem_t   *item,
                       *nextItem;

    start = zone ? (zone_table[zone - 1].top + 1) : 0;
    end = zone_table[zone].top;

    for (i = start; i <= end; i++) {
        rp = roomFindNum(i);
        if (!rp) {
            continue;
        }
        for (vict = rp->people; vict; vict = next_v) {
            next_v = vict->next_in_room;
            if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF))) {
                extract_char(vict);
            }
        }

        LinkedListLock( rp->contentList );
        for( item = rp->contentList->head; item; item = nextItem ) {
            nextItem = item->next;
            obj = CONTENT_LINK_TO_OBJ(item);
            obj->index->number--;

            /*
             * object maxxing.(GH)
             *
             * Do not clean out corpses, druid trees or quest items. Bit
             * of kludge to avoid deinit getting rid of quest items when a
             * scavenge quest is going on. Maybe make a SYS_NODEINIT? Hard
             * to use for small imms. -Lennya
             */
            if (!IS_CORPSE(obj) && ITEM_TYPE(obj) != ITEM_TYPE_TREE &&
                !IS_OBJ_STAT(obj, extra_flags, ITEM_QUEST)) {
                objectExtract(obj);
            }
        }
        LinkedListUnlock( rp->contentList );
    }
}

int ZoneCleanable(int zone)
{
    struct room_data *rp;
    struct char_data *vict;
    int             start,
                    end,
                    i;

    start = zone ? (zone_table[zone - 1].top + 1) : 0;
    end = zone_table[zone].top;

    for (i = start; i <= end; i++) {
        rp = roomFindNum(i);
        if (!rp) {
            return (TRUE);
        }
        for (vict = rp->people; vict; vict = vict->next_in_room) {
            if (!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) {
                return (FALSE);
            }
        }

    }
    return (TRUE);
}

int FindZone(int zone)
{
    int             i;

    for (i = 0; i <= top_of_zone_table; i++) {
        if (zone_table[i].num == zone) {
            break;
        }
    }

    if (zone_table[i].num != zone) {
        return (-1);
    } else {
        return i;
    }
}

FILE           *MakeZoneFile(struct char_data * c, int zone)
{
    char            buf[256];
    FILE           *fp;

    sprintf(buf, "zones/%d.zon", zone);

    if ((fp = fopen(buf, "wt")) != NULL) {
        return (fp);
    } else {
        return (0);
    }
}

FILE           *OpenZoneFile(struct char_data * c, int zone)
{
    char            buf[256];
    FILE           *fp;

    sprintf(buf, "zones/%d.zon", zone);

    if ((fp = fopen(buf, "rt")) != NULL) {
        return (fp);
    } else {
        return (0);
    }
}


void do_WorldSave(struct char_data *ch, char *argument, int cmd)
{
    char            temp[2048],
                    buf[128];
    long            rstart,
                    rend,
                    i,
                    j,
                    k,
                    x;
    struct extra_descr_data *exptr;
    FILE           *fp;
    struct room_data *rp;
    struct room_direction_data *rdd;
    char           *keyword;

    if (!ch->desc) {
        return;
    }
    rstart = 0;
    rend = top_of_world;

    if ((fp = fopen("tinyworld.wld.new", "w")) == NULL) {
        send_to_char("Can't create .wld file\r\n", ch);
        return;
    }

    sprintf(buf, "%s resorts the world (The game will pause for a few "
                 "moments).\r\n", ch->player.name);
    SendToAll(buf);

    sprintf(buf, "Saving World (%ld rooms)\n\r", (long) top_of_world);
    send_to_char(buf, ch);

    for (i = rstart; i <= rend; i++) {
        rp = roomFindNum(i);
        if (rp == NULL) {
            continue;
        }
        /*
         * strip ^Ms from description
         */
        x = 0;

        if (!rp->description) {
            CREATE(rp->description, char, 8);
            strcpy(rp->description, "Empty");
        }

        for (k = 0; k <= strlen(rp->description); k++) {
            if (rp->description[k] != 13) {
                temp[x++] = rp->description[k];
            }
        }
        temp[x] = '\0';

        if (temp[0] == '\0') {
            strcpy(temp, "Empty");
        }

        fprintf(fp, "#%ld\n%s~\n%s~\n", rp->number, rp->name, temp);
        if (!rp->tele_targ) {
            fprintf(fp, "%ld %ld %ld", rp->zone, rp->room_flags,
                    rp->sector_type);
        } else if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
            fprintf(fp, "%ld %ld -1 %d %d %d %ld", rp->zone,
                    rp->room_flags, rp->tele_time, rp->tele_targ,
                    rp->tele_mask, rp->sector_type);
        } else {
            fprintf(fp, "%ld %ld -1 %d %d %d %d %ld", rp->zone,
                    rp->room_flags, rp->tele_time, rp->tele_targ,
                    rp->tele_mask, rp->tele_cnt, rp->sector_type);
        }

        if (rp->sector_type == SECT_WATER_NOSWIM) {
            fprintf(fp, " %d %d", rp->river_speed, rp->river_dir);
        }

        if (rp->room_flags & TUNNEL) {
            fprintf(fp, " %d ", rp->moblim);
        }

        fprintf(fp, "\n");

        for (j = 0; j < 6; j++) {
            rdd = rp->dir_option[j];
            if (rdd) {
                fprintf(fp, "D%ld\n", j);

                if (rdd->general_description && *rdd->general_description) {
                    if (strlen(rdd->general_description) > 0) {
                        temp[0] = '\0';
                        x = 0;

                        for (k = 0; k <= strlen(rdd->general_description);
                             k++) {
                            if (rdd->general_description[k] != 13) {
                                temp[x++] = rdd->general_description[k];
                            }
                        }
                        temp[x] = '\0';
                        fprintf(fp, "%s~\n", temp);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                if (rdd->keyword) {
                    if (strlen(rdd->keyword) > 0) {
                        fprintf(fp, "%s~\n", rdd->keyword);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                fprintf(fp, "%ld ", rdd->exit_info);
                fprintf(fp, "%ld ", rdd->key);
                fprintf(fp, "%ld ", rdd->to_room);
                fprintf(fp, "%ld", rdd->open_cmd);
                fprintf(fp, "\n");
            }
        }

        /*
         * extra descriptions..
         */

        for (j = 0, exptr = rp->ex_description; j < rp->ex_description_count; 
             exptr++, j++) {
            x = 0;

            if (exptr->description) {
                for (k = 0; k <= strlen(exptr->description); k++) {
                    if (exptr->description[k] != 13) {
                        temp[x++] = exptr->description[k];
                    }
                }
                temp[x] = '\0';

                keyword = KeywordsToString(exptr," ");
                fprintf(fp, "E\n%s~\n%s~\n", keyword, temp);
                free( keyword );
            }
        }

        fprintf(fp, "S\n");

    }
    fclose(fp);

    sprintf(buf, "The world returns to normal as %s finishes the job.\r\n",
            ch->player.name);
    SendToAll(buf);
    send_to_char("\n\rDone\n\r", ch);

    return;
}

void RoomSave(struct char_data *ch, long start, long end)
{
    int             countrooms = 0;
    char            fn[80],
                    temp[2048],
                    dots[500];
    int             rstart,
                    rend,
                    i,
                    j,
                    k,
                    len,
                    x;
    struct extra_descr_data *exptr;
    FILE           *fp = NULL;
    struct room_data *rp;
    struct room_direction_data *rdd;
    char           *keyword; 

    rstart = start;
    rend = end;

    if (rstart <= -1 || rend <= -1 || rstart > WORLD_SIZE ||
        rend > WORLD_SIZE) {
        send_to_char("I don't know those room #s.  make sure they are all\n\r",
                     ch);
        send_to_char("contiguous.\n\r", ch);
        fclose(fp);
        return;
    }

    send_to_char("Saving\n", ch);
    strcpy(dots, "\0");

    for (i = rstart; i <= rend; i++) {
        rp = roomFindNum(i);
        if (rp == NULL) {
            continue;
        }
        sprintf(fn, "rooms/%d", i);
        if ((fp = fopen(fn, "w")) == NULL) {
            send_to_char("Can't write to disk now..try later \n\r", ch);
            return;
        }
        countrooms++;
        strcat(dots, ".");

        /*
         * strip ^Ms from description
         */
        x = 0;

        if (!rp->description) {
            CREATE(rp->description, char, 8);
            strcpy(rp->description, "Empty");
        }

        for (k = 0; k <= strlen(rp->description); k++) {
            if (rp->description[k] != 13) {
                temp[x++] = rp->description[k];
            }
        }
        temp[x] = '\0';

        if (temp[0] == '\0') {
            strcpy(temp, "Empty");
        }

        fprintf(fp, "#%ld\n%s~\n%s~\n", rp->number, rp->name, temp);
        if (!rp->tele_targ) {
            fprintf(fp, "%ld %ld %ld", rp->zone, rp->room_flags,
                    rp->sector_type);
        } else {
            if (!IS_SET(TELE_COUNT, rp->tele_mask)) {
                fprintf(fp, "%ld %ld -1 %d %d %d %ld", rp->zone,
                        rp->room_flags, rp->tele_time, rp->tele_targ,
                        rp->tele_mask, rp->sector_type);
            } else {
                fprintf(fp, "%ld %ld -1 %d %d %d %d %ld", rp->zone,
                        rp->room_flags, rp->tele_time, rp->tele_targ,
                        rp->tele_mask, rp->tele_cnt, rp->sector_type);
            }
        }
        if (rp->sector_type == SECT_WATER_NOSWIM) {
            fprintf(fp, " %d %d", rp->river_speed, rp->river_dir);
        }

        if (rp->room_flags & TUNNEL) {
            fprintf(fp, " %d ", (int) rp->moblim);
        }

        fprintf(fp, "\n");

        for (j = 0; j < 6; j++) {
            rdd = rp->dir_option[j];
            if (rdd) {
                fprintf(fp, "D%d\n", j);

                if (rdd->general_description && *rdd->general_description) {
                    if (strlen(rdd->general_description) > 0) {
                        temp[0] = '\0';
                        x = 0;

                        for (k = 0; k <= strlen(rdd->general_description);
                             k++) {
                            if (rdd->general_description[k] != 13) {
                                temp[x++] = rdd->general_description[k];
                            }
                        }
                        temp[x] = '\0';

                        fprintf(fp, "%s~\n", temp);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                if (rdd->keyword) {
                    do {
                        len = strlen(rdd->keyword);
                        if( len == 0 || (rdd->keyword[len-1] != '\n' &&
                                         rdd->keyword[len-1] != '\r') ) {
                            break;
                        }
                        rdd->keyword[len-1] = '\0';
                    } while( len > 0 );

                    if (len > 0) {
                        fprintf(fp, "%s~\n", rdd->keyword);
                    } else {
                        fprintf(fp, "~\n");
                    }
                } else {
                    fprintf(fp, "~\n");
                }

                fprintf(fp, "%ld ", rdd->exit_info);
                fprintf(fp, "%ld ", rdd->key);
                fprintf(fp, "%ld ", rdd->to_room);
                fprintf(fp, "%ld", rdd->open_cmd);
                fprintf(fp, "\n");
            }
        }

        /*
         * extra descriptions..
         */

        for (j = 0, exptr = rp->ex_description; j < rp->ex_description_count; 
             exptr++, j++) {
            x = 0;

            if (exptr->description) {
                for (k = 0; k <= strlen(exptr->description); k++) {
                    if (exptr->description[k] != 13) {
                        temp[x++] = exptr->description[k];
                    }
                }
                temp[x] = '\0';

                keyword = KeywordsToString(exptr," ");
                fprintf(fp, "E\n%s~\n%s~\n", keyword, temp);
                free( keyword );
            }
        }

        fprintf(fp, "S\n");
        fclose(fp);
    }

    send_to_char(dots, ch);
    sprintf(dots, "\n\rDone: %d rooms saved\n\r", countrooms);
    send_to_char(dots, ch);
}

void RoomLoad(struct char_data *ch, int start, int end)
{
    FILE           *fp;
    int             vnum,
                    found = TRUE;
    char            buf[80];
    struct room_data *rp;

    send_to_char("Searching and loading rooms\n\r", ch);

    for (vnum = start; vnum <= end; vnum++) {
        sprintf(buf, "rooms/%d", vnum);
        if (!(fp = fopen(buf, "r"))) {
            found = FALSE;
            send_to_char(".", ch);
            continue;
        }

        fscanf(fp, "#%*d\n");
        if (!(rp = roomFindNum(vnum))) {
            /*
             * empty room
             */
            rp = (void *) malloc(sizeof(struct room_data));
            if (rp) {
                bzero(rp, sizeof(struct room_data));
            }
#ifdef HASH
            /*
             * this still does not work and needs work by someone
             */
            room_enter(&room_db, vnum, rp);
#else
            room_enter(room_db, vnum, rp);
#endif
            send_to_char("+", ch);
        } else {
            if (rp->people) {
                act("$n reaches down and scrambles reality.", FALSE, ch,
                    NULL, rp->people, TO_ROOM);
            }
            cleanout_room(rp);
            send_to_char("-", ch);
        }

        rp->number = vnum;
        load_one_room(fp, rp);
        fclose(fp);
    }

    if (!found) {
        send_to_char("\n\rThe room number(s) that you specified could not all "
                     "be found.\n\r", ch);
    } else {
        send_to_char("\n\rDone.\n\r", ch);
    }
}



int room_of_object(struct obj_data *obj)
{
    if (obj->in_room != NOWHERE) {
        return obj->in_room;
    } else if (obj->carried_by) {
        return obj->carried_by->in_room;
    } else if (obj->equipped_by) {
        return obj->equipped_by->in_room;
    } else if (obj->in_obj) {
        return room_of_object(obj->in_obj);
    } else {
        return NOWHERE;
    }
}


struct FMDZSR_Args_t {
    int             num;
    struct char_data *ch;
};

bool FMDZSR_cond( struct char_data *t, void *arg )
{
    struct FMDZSR_Args_t   *args;

    if( !t || !arg ) {
        return( FALSE );
    }

    args = (RMDZSR_Args_t *)arg;

    if( !args->ch ) {
        return( FALSE );
    }

    args.num--;

    if( args->num < 0 ) {
        return( TRUE );
    }

    if (GET_RACE(t) == GET_RACE(args->ch) && IS_NPC(t) && !IS_PC(t) && 
        !args->num) {
        return( TRUE );
    }
    return( FALSE );
}

bool FMDZSR_callback( struct char_data *t, void *arg )
{
    struct FMDZSR_Args_t   *args;
    struct room_data *rp1,
                   *rp2;

    if( !t || !arg ) {
        return( FALSE );
    }

    args = (RMDZSR_Args_t *)arg;

    if( !args->ch ) {
        return( FALSE );
    }

    if( args->num < 0 ) {
        return( TRUE );
    }

    rp1 = roomFindNum(args->ch->in_room);
    rp2 = roomFindNum(t->in_room);
    if (rp1->zone != rp2->zone) {
        return (t);
    }
}

struct char_data *FindMobDiffZoneSameRace(struct char_data *ch)
{
    struct FMDZSR_Args_t    args;
    bool            yes = TRUE;
    struct char_data *t;

    args.num = number(1, 100);
    args.ch  = ch;

    t = playerFindAll( FMDZSR_cond, &args, FMDZSR_callback, &args, &yes );
    if( args.num < 0 ) {
        return( NULL );
    } else {
        return( t );
    }
}

bool IncrementZoneNr_cond( struct char_data *c, void *arg )
{
    int         nr;

    if( !c || !arg ) {
        return( FALSE );
    }

    nr = *(int *)arg;
    if (c->specials.zone >= nr) {
        return( TRUE );
    }
    return( FALSE );
}

bool IncrementZoneNr_incr_callback( struct char_data *c, void *arg )
{
    c->specials.zone++;
    return( TRUE );
}

bool IncrementZoneNr_decr_callback( struct char_data *c, void *arg )
{
    c->specials.zone--;
    return( TRUE );
}

void IncrementZoneNr(int nr)
{
    struct char_data *c;

    if (nr > top_of_zone_table) {
        return;
    }
    if (nr >= 0) {
        playerFindAll( IncrementZoneNr_cond, &nr, IncrementZoneNr_incr_callback,
                       NULL, NULL );
        }
    } else {
        playerFindAll( IncrementZoneNr_cond, &nr, IncrementZoneNr_decr_callback,
                       NULL, NULL );
        }
    }
}

int CheckSquare(struct char_data *ch, int dir)
{
    int                 room = 0;
    struct room_data   *rm;
    struct room_data   *rp;

    rm = roomFindNum(ch->in_room);

    if (dir == 0 || dir == 2) {
        /*
         * go east and check
         */
        if (rm->dir_option[1]) {
            room = rm->dir_option[1]->to_room;
            rp = roomFindNum(room);
            if (rp->dir_option[dir]) {
                room = rp->dir_option[dir]->to_room;
                rp = roomFindNum(room);
                if (rp->dir_option[3]) {
                    room = rp->dir_option[3]->to_room;
                    return( room );
                }
            }
        }

        if (rm->dir_option[3]) {
            room = rm->dir_option[3]->to_room;
            rp = roomFindNum(room);
            if (rp->dir_option[dir]) {
                room = rp->dir_option[dir]->to_room;
                rp = roomFindNum(room);
                if (rp->dir_option[1]) {
                    room = rp->dir_option[1]->to_room;
                    return( room );
                }
            }
        }
    }

    if (dir == 1 || dir == 3) {
        /*
         * go north and check
         */
        if (rm->dir_option[0])) {
            room = rm->dir_option[0]->to_room;
            rp = roomFindNum(room);
            if (rp->dir_option[dir]) {
                room = rp->dir_option[dir]->to_room;
                rp = roomFindNum(room);
                if (rp->dir_option[2]) {
                    room = rp->dir_option[2]->to_room;
                    return( room );
                }
            }
        }

        if (rm->dir_option[2])) {
            room = rm->dir_option[2]->to_room;
            rp = roomFindNum(room);
            if (rp->dir_option[dir]) {
                room = rp->dir_option[dir]->to_room;
                rp = roomFindNum(room);
                if (rp->dir_option[0]) {
                    room = rp->dir_option[0]->to_room;
                    return( room );
                }
            }
        }
    }

    return( NOWHERE );
}

/*
 * Edit Fast rewriten by Greg Hovey(GH) makes an exit to a new room and
 * back
 */
int make_exit_ok(struct char_data *ch, struct room_data **rpp, int dir)
{
    int             current = 0;
    int             x,
                    sector;
    char            buf[255];
    struct zone_data *zd;
    struct room_data *rm = 0,
                   *new_rm = 0;
    int             square = 0;
    if (GetMaxLevel(ch) < 53 || !rpp || !ch->desc ||
        !IS_SET(ch->player.user_flags, FAST_AREA_EDIT))
        return (FALSE);

    current = ch->in_room;
    rm = roomFindNum(current);
    /*
     * lets find valid room..
     */
    zd = zone_table + (rm->zone - 1);
    sector = rm->sector_type;

    if (GetMaxLevel(ch) < 57 && rm->zone != GET_ZONE(ch)) {
        send_to_char("Sorry, you are not authorized to edit this zone. Get one"
                     " assigned to you.\n\r", ch);
        return (TRUE);
    }

    if (IS_SET(ch->player.user_flags, FAST_MAP_EDIT)) {
        square = CheckSquare(ch, dir);

        if (square != NOWHERE) {
            sprintf(buf, "exit %d 0 0 %d", dir, square);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);

            char_from_room(ch);
            char_to_room(ch, square);
            dir = opdir(dir);

            sprintf(buf, "exit %d 0 0 %d", dir, current);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);
            send_to_char("Reconnecting to existing room\n\r", ch);

            new_rm = roomFindNum(ch->in_room);
            new_rm->room_flags = ROOM_WILDERNESS;

            do_look(ch, NULL, 15);

            return (TRUE);
        }
    }

    x = zd->top;
    zd = zone_table + rm->zone;

    for (x = x + 1; x < zd->top; x++) {
        if (roomFindNum(x) == NULL) {
            CreateOneRoom(x);
            sprintf(buf, "$c0001Room exit created from room %d to %d.\n\r",
                    current, x);
            send_to_char(buf, ch);

            sprintf(buf, "exit %d 0 0 %d", dir, x);
            /*
             * make exit in desired direction..
             */
            do_edit(ch, buf, 0);

            /*
             * move char to that room..
             */
            char_from_room(ch);
            char_to_room(ch, x);
            /*
             * opposite direction..
             */
            dir = opdir(dir);

            if (roomFindNum(current) == NULL) {
                CreateOneRoom(current);
            }
            new_rm = roomFindNum(ch->in_room);
            /*
             * let's set the sector to match the room we came from
             */
            new_rm->sector_type = sector;

            sprintf(buf, "exit %d 0 0 %d", dir, current);
            do_edit(ch, buf, 0);
            /*
             * If wizset map is enabled, set the roomflag MAP_ROOM -Lennya
             */
            if (IS_SET(ch->player.user_flags, FAST_MAP_EDIT)) {
                new_rm = roomFindNum(ch->in_room);
                new_rm->room_flags = ROOM_WILDERNESS;
            }
            do_look(ch, NULL, 15);
            return (TRUE);
        }
    }
    send_to_char("No more empty rooms in your assigned zone!!\n\r", ch);
    return (TRUE);
}


void do_mrebuild(struct char_data *ch, char *argument, int cmd)
{
    char            buf[128];
    long            m_start,
                    m_end,
                    i,
                    nr;
    FILE           *mob_file;
    FILE           *vnum_f;
    struct char_data *mob;
    int             count = 0;

    if (!ch->desc) {
        return;
    }
    m_start = 0;
    m_end = top_of_mobt;

    if ((mob_file = fopen("tinyworld.mob.new", "w")) == NULL) {
        send_to_char("Can't create .mob.new file\r\n", ch);
        return;
    }

    sprintf(buf, "%s resorts the mobiles (The game will pause for a few "
                 "moments).\r\n", ch->player.name);
    SendToAll(buf);

    sprintf(buf, "Saving Mobiles (%ld mobiles)\n\r", (long) m_end);
    send_to_char(buf, ch);

    for (i = m_start; i <= WORLD_SIZE; i++) {
        if ((mob = mobileRead(i))) {

            sprintf(buf, "mobiles/%ld", i);
            if ((vnum_f = fopen(buf, "wt")) == NULL) {
                send_to_char("Can't write to disk now..try later.\n\r", ch);
                return;
            }

            fprintf(vnum_f, "#%ld\n", i);
            mobileWrite(mob, vnum_f);
            fclose(vnum_f);
            mobileInsert(mob, i);

            fprintf(mob_file, "#%ld\n", i);
            mobileWrite(mob, mob_file);
            count++;
            extract_char(mob);
        }
    }

    fwrite_string(mob_file, "#99999\n%");

    fclose(mob_file);

    sprintf(buf, "The world returns to normal as %s finishes the job.\r\n",
            ch->player.name);
    SendToAll(buf);
    send_to_char("\n\rDone.\n\r", ch);
    sprintf(buf, "(%d) mobiles saved!\n\r", count);
    send_to_char(buf, ch);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

