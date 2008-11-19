
bool down_river_condition( struct char_data *ch, void *arg ) 
{
    struct room_data   *rp;
    int                 pulse;

    if( !arg ) {
        return( FALSE );
    }
    pulse = *(int *)arg;

    if( IS_NPC(ch) ) {
        return( FALSE );
    }

    if( ch->in_room == NOWHERE ) {
        return( FALSE );
    }

    rp = roomFindNum(ch->in_room);
    if( !rp ) {
        return( FALSE );
    }

    if( rp->sector_type != SECT_WATER_NOSWIM ) {
        return( FALSE );
    }

    if( pulse % rp->river_speed != 0 ) {
        return( FALSE );
    }

    if( rp->river_dir <= 5 && rp->river_dir >= 0 ) {
        return( TRUE );
    }

    return( FALSE );
}


bool down_river_callback( struct char_data *ch, void *arg )
{
    struct obj_data *obj_object,
                   *next_obj;
    int             rd,
                    or;
    char            buf[80];
    struct room_data *rp;
    LinkedListItem_t   *item,
                       *nextItem;

    rp = roomFindNum(ch->in_room);
    rd = rp->river_dir;

    LinkedListLock( rp->contentList );
    for( item = rp->contentList->head; item; item = nextItem ) {
        nextItem = item->next;
        obj_object = CONTENT_LINK_TO_OBJ(item);
        if (rp->dir_option[rd]) {
            objectTakeFromRoom(obj_object, LOCKED);
            objectPutInRoom(obj_object,
                            rp->dir_option[rd]->to_room, UNLOCKED);
        }
    }
    LinkedListUnlock( rp->contentList );

    /*
     * flyers don't get moved
     */
    if( IS_AFFECTED(ch, AFF_FLYING) || MOUNTED(ch) ) {
        return( FALSE );
    }

    if (rp->dir_option[rd] && rp->dir_option[rd]->to_room &&
        (EXIT(ch, rd)->to_room != NOWHERE)) {
        if (ch->specials.fighting) {
            stop_fighting(ch);
        }

        if (IS_IMMORTAL(ch) &&
            IS_SET(ch->specials.act, PLR_NOHASSLE)) {
            send_to_char("The waters swirl beneath your feet.\n\r", ch);
        } else {
            sprintf(buf, "You drift %s...\n\r", direction[rd].dir);
            send_to_char(buf, ch);
            if (RIDDEN(ch)) {
                send_to_char(buf, RIDDEN(ch));
            }

            char_from_room(ch);
            if (RIDDEN(ch)) {
                char_from_room(RIDDEN(ch));
                char_to_room(RIDDEN(ch), rp->dir_option[rd]->to_room);
            }

            char_to_room(ch, rp->dir_option[rd]->to_room);

            do_look(ch, NULL, 15);
            if (RIDDEN(ch)) {
                do_look(RIDDEN(ch), NULL, 15);
            }
        }

        if (IS_SET(RM_FLAGS(ch->in_room), DEATH) && !IS_IMMORTAL(ch)) {
            if (RIDDEN(ch)) {
                NailThisSucker(RIDDEN(ch));
            }
            NailThisSucker(ch);
        }
    } else {
        return( FALSE );
    }

    return( TRUE );
}


void down_river(int pulse)
{
    struct char_data *ch,
                   *tmp;
    struct obj_data *obj_object,
                   *next_obj;
    int             rd,
                    or;
    char            buf[80];
    struct room_data *rp;
    LinkedListItem_t   *item,
                       *nextItem;

    if (pulse < 0) {
        return;
    }

    playerFindAll( down_river_condition, &pulse, down_river_callback, NULL,
                   NULL );
}


bool TeleportPulseStuff_cond( struct char_data *ch, void *arg )
{
    if( !ch ) {
        return( FALSE );
    }

    if( IS_MOB(ch) || IS_PC(ch) ) {
        return( TRUE );
    }
    return( FALSE );
}

bool TeleportPulseStuff_callback( struct char_data *ch, void *arg )
{
    struct char_data *next,
                   *tmp,
                   *bk,
                   *n2;
    int             tick;
    struct room_data *rp,
                   *dest;
    struct obj_data *obj_object,
                   *temp_obj;
    LinkedListItem_t   *item,
                       *nextItem;

    if( !ch || !arg ) {
        return( FALSE );
    }

    tick = *(int *)arg;

    if (IS_MOB(ch)) {
        if (ch->specials.tick == tick && !ch->specials.fighting) {
            mobile_activity(ch);
            return( TRUE );
        }
        return( FALSE );
    } 
    
    if (IS_PC(ch)) {
        rp = roomFindNum(ch->in_room);
        if (rp && rp->tele_targ > 0 && rp->tele_targ != rp->number &&
            rp->tele_time > 0 && !(pulse % rp->tele_time)) {

            dest = roomFindNum(rp->tele_targ);
            if (!dest) {
                Log("invalid tele_targ");
                return( FALSE );
            }

            LinkedListLock( rp->contentList );
            for( item = rp->contentList->head; item; item = nextItem ) {
                nextItem = item->next;
                obj_object = CONTENT_LINK_TO_OBJ(item);
                objectTakeFromRoom(obj_object, LOCKED);
                objectPutInRoom(obj_object, rp->tele_targ, UNLOCKED);
            }
            LinkedListUnlock( rp->contentList );

            bk = 0;

            while (rp->people) {
                tmp = rp->people;
                if (!tmp) {
                    break;
                }
                if (tmp == bk) {
                    break;
                }
                bk = tmp;

                /*
                 * the list of people in the room has changed
                 */
                char_from_room(tmp);
                char_to_room(tmp, rp->tele_targ);

                if (IS_SET(TELE_LOOK, rp->tele_mask) && IS_PC(tmp)) {
                    do_look(tmp, NULL, 15);
                }

                if (IS_SET(dest->room_flags, DEATH) && !IS_IMMORTAL(tmp)) {
                    if (tmp == next) {
                        next = tmp->next;
                    }
                    NailThisSucker(tmp);
                    return( TRUE );
                }

                if (dest->sector_type == SECT_AIR) {
                    n2 = tmp->next;
                    if (check_falling(tmp)) {
                        if (tmp == next) {
                            next = n2;
                        }
                    }
                }
            }

            if (IS_SET(TELE_COUNT, rp->tele_mask)) {
                /*
                 * reset it for next count
                 */
                rp->tele_time = 0;
            }

            if (IS_SET(TELE_RANDOM, rp->tele_mask)) {
                rp->tele_time = number(1, 10) * 100;
            }
        }
        return( TRUE );
    }

    return( FALSE );
}

void TeleportPulseStuff(int pulse)
{
    /*
     * check_mobile_activity(pulse); Teleport(pulse);
     */


    /* this is dependent on P_M = 3*P_T */
    tm = pulse % PULSE_MOBILE;

    if (tm == 0) {
        tick = 0;
    } else if (tm == PULSE_TELEPORT) {
        tick = 1;
    } else if (tm == PULSE_TELEPORT * 2) {
        tick = 2;
    }

    playerFindAll( TeleportPulseStuff_cond, NULL, TeleportPulseStuff_callback,
                   &tick, NULL );
}

char           *advicelist[] = {
    "Havok's webpage is located at $c000Whttp://havok.haaksman.org/",
    "Remember, when you want to leave, find an innkeeper to $c0015RENT$c0007 or"
        " ask her for an $c0015OFFER$c0007.(From Temple. $c000Ws2e4neu$c0007)",
    "After earning enough exp to level, remember to see your guildmaster to "
        "gain and learn new skills.",
    "To disable this advice channel, type $c000WSET ADVICE DISABLE",
    "Use the $c000WOOC $c000wcommand to talk out of character.",
    "Remember, the $c000WYELL$c000w command is used for zone communication and"
        " $c000WSHOUT$c000w is a world wide channel.",
    "If you have any ideas for the mud, please use the $c000WIDEA$c000w command"
        " to inform the gods of it.",
    "To see a list of all the socials in the game, type $c000WHELP "
        "SOCIAL$c0007.",
    "West of the Temple is a good place to get experience and some good "
        "starting gear.",
    "East of the Temple is donations. Feel free to grub around there. There is"
        " also another donations in New Thalos",
    "North of the Temple is the hall of rules.  Ignorance of these rules is "
        "unacceptable.",
    "Don't forget to eat and drink or you may die of hunger and thirst.",
    "To see who is grouped, type $c000WGWHO$c0007.",
    "To turn off any certain channel, type $c000WNOOOC, NOSHOUT, NOYELL$c000w,"
        " etc.",
    "If you have any problems, ask for a god and they may assist you at any "
        "time.",
    "Remember, no foul language over public channels please.",
    "To see if anyone is in the arena battling, type$c000W WHOARENA",
    "Karysinya Coach is a quick and easy way to travel the lands. (From "
        "Meeting Square go $c000Wsesu$c000w)",
    "Type $c000WWORLD $c000wto see some interesting facts about Havok.",
    "After level five, $c000WATTRIBUTES $c000wcommand is used to see what "
        "spell affects and stats your character has.",
    "Did you know that you can request immortality if your character gains "
        "200mil XP or more?",
    "Immortals/Gods are the people that rule the mud. Type $c000WWIZLIST$c000w"
        " to see a list of all of them.",
    "Before attacking someone, use the $c000WCONSIDER$c000w command to "
        "determine how strong they are.",
    "To see a list of people in the same zone as you, type "
        "$c000WWHOZONE$c000w.",
    "Some items have ego which means you have to be a certain level to use the"
        " item.",
    "See our discussion board, go 2 east & 1 north of meeting square. "
        "($c000Wlook board, read <message#>$c0007)",
    "At times, you may see our resident bot Seth on, whom will spell you up "
        "and summon you when needed.",
    "Typing $c000WQUIT$c000w will not save your character's equipment. Try "
        "$c0015HELP RENT$c0007.",
    "Bug the immortals to add more help and advice to this list.",
    "Type $c000WNEWS$c000w to see the last news and events that occurred on "
        "Havok",
    "Use $c000WBPROMPT$c000w to set your battle prompt the same way as the "
        "regular $c000WPROMPT$c000w command.",
    "Type $c000WHELP COLOR$c000w to get a list of the color code characters.",
    "You cannot $c000WSHOUT$c000w or use $c000WOOC$c000w until level 2.",
    "If you're curious about the function of a spell, type $c0015HELP SPELL "
        "<spellname>$c0007.",
    "If you're curious about the function of a skill, type $c0015HELP SKILL "
        "<skillname>$c0007.",
    "You may want to read $c0015HELP NEWBIE$c0007 for some useful information "
        "to get you started.",
    "When confused, try $c0015HELP <feature>$c0007, there may be a helpfile "
        "about it!",
    "Some of Havok's monsters are smart. They may attack you, talk to you, "
        "cast spells on you, or help each other out.",
    "In the $c0015ARENA$c0007, players can battle each other. Dont worry, "
        "there are no penalties for dying in the Arena.",
    "Use $c0015GET ALL.<ITEMNAME>$c0007 to pick up all of a specific item in "
        "a room, such as all.coins"
};
int advicecount = NELEMENTS(advicelist);

void AdvicePulseStuff(int pulse)
{
    int             numberadvice;
    struct descriptor_data *i;
    register struct char_data *ch;
    char            buffer[150];

    if (pulse < 0 || number(0, 1)) {
        return;
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;

            if (IS_PC(ch) && ch->in_room != NOWHERE &&
                IS_SET(ch->player.user_flags, NEW_USER)) {
                numberadvice = number(0, advicecount - 1);
                snprintf(buffer, 150, "$c000GAdvice: '$c000w%s$c000G'\n\r",
                        advicelist[numberadvice]);
                buffer[149] = '\0';

                if (AWAKE(ch)) {
                    send_to_char(buffer, ch);
                }
            }
        }
    }
}

void DarknessPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;
    int             j = 0;

    if (pulse < 0 || number(0, 1)) {
        return;
    }
    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            for (j = 0; j <= (MAX_WEAR - 1); j++) {
                /*
                 * scrap antisun before doing the darkness check
                 */
                if (ch->equipment[j] && ch->equipment[j]->item_number >= 0 &&
                    IS_OBJ_STAT(ch->equipment[j], anti_flags, ITEM_ANTI_SUN)) {
                    AntiSunItem(ch, 0, 0, ch->equipment[j], PULSE_COMMAND);
                }
            }

            if (IS_PC(ch) && IsDarkrace(ch) && AWAKE(ch) &&
                !affected_by_spell(ch, SPELL_GLOBE_DARKNESS) &&
                !IS_AFFECTED(ch, AFF_DARKNESS) && !IS_UNDERGROUND(ch) &&
                !IS_DARK(ch->in_room)) {
                act("$n uses $s innate powers of darkness.", FALSE, ch,
                    0, 0, TO_ROOM);
                act("You use your innate powers of darkness.", FALSE,
                    ch, 0, 0, TO_CHAR);
                cast_globe_darkness(GetMaxLevel(ch), ch, "",
                                    SPELL_TYPE_SPELL, ch, 0);
            }
        }
    }
}

struct obj_data *find_tqp(int tqp_nr)
{
    struct obj_data    *tqp;
    struct index_data  *index;
    LinkedListItem_t   *item;

    index = objectIndex( TRAVELQP );
    if( !index ) {
        return( NULL );
    }

    LinkedListLock( index->list );
    for( item = index->list->head, nr = 1, tqp = NULL; item && !tqp; 
         item = item->next, nr++ ) {
        if( nr == tqp_nr ) {
            tqp = GLOBAL_LINK_TO_OBJ(item);
        }
    }
    LinkedListUnlock( index->list );

    return( tqp );
}

int count_tqp(void)
{
    struct index_data  *index;

    index = objectIndex( TRAVELQP );
    if( !index ) {
        return( 0 );
    }

    return( index->number );
}

void traveling_qp(int pulse)
{
    struct char_data *ch = NULL,
                   *newch;
    struct room_data *room;
    struct obj_data *travelqp = 0,
                   *qt;
    int             to_room = 0;
    int             k,
                    f;

    qp_patience++;              /* some secs have passed */

    for (k = 1; k <= TQP_AMOUNT; k++) {
        f = 1;
        if (!(travelqp = find_tqp(k))) {
            f = 0;
        } else if (init_counter() < MIN_INIT_TQP) {
            objectExtract(travelqp);
            f = 0;
        } else if (!(ch = travelqp->carried_by)) {
            Log("not carried, extracting");
            objectExtract(travelqp);
            f = 0;
        } else if (!IS_NPC(ch)) {
#if 0
            ch->player.q_points++;
#endif
            send_to_char("You found yourself some booty, and are rewarded by"
                         " the gods with a $c000Rq$c000Yu$c000Ge$c000Bs"
                         "$c000Ct$c000w token.\n\r", ch);
            if ((qt = objectRead(QUEST_POTION))) {
                objectGiveToChar(qt, ch);
            }
            Log("carried by player, gained a QT");
            qlog(ch, "found a quest token.");
            objectExtract(travelqp);
            f = 0;
        } else if (qp_patience < 8) {
            /*
             * hasn't been in inventory long enough yet
             */
            f = 0;
        } else if (qp_patience <= 60 && number(k, 3) == 3) {
            /*
             * hasn't been sitting here too long yet
             * 67, 50, 0% chance of not moving
             */
            f = 0;
        }

        /*
         * find a new mob
         */
        if (f) {
            newch = NULL;
            while (!newch) {
                /*
                 * this may cause endless loop, maybe use for(1..100)
                 */
                to_room = number(0, top_of_world);
                room = roomFindNum(to_room);
                if (room && (newch = room->people) &&
                    (IS_PC(newch) || newch == ch || newch->specials.fighting ||
                     IS_SET(newch->specials.act, ACT_POLYSELF))) {
                    newch = NULL;
                }
            }

            objectTakeFromChar(travelqp);
            act("$n ceases to be outlined by a $c000Rm$c000Yu$c000Gl$c000Bt"
                "$c000Ci$c000wcolored hue.", FALSE, ch, 0, 0, TO_ROOM);
            objectGiveToChar(travelqp, newch);
            act("$n is suddenly surrounded by a $c000Rm$c000Yu$c000Gl$c000Bt"
                "$c000Ci$c000wcolored hue!", FALSE, newch, 0, 0, TO_ROOM);
            qp_patience = 0;
        }
    }
}

#define ARENA_ZONE 124
void ArenaPulseStuff(int pulse)
{
    struct descriptor_data *i;
    struct char_data *ch = NULL;
    char            buf[80];
    int             location = 0;

    if (pulse < 0)
        return;

    if (MinArenaLevel == 0 && MaxArenaLevel == 0 &&
        countPeople(ARENA_ZONE) == 1) {
        /*
         * arena must be closed
         * last one standing in arena!
         * let's see who this is
         */
        for (i = descriptor_list; i; i = i->next) {
            if (!i->connected) {
                ch = i->character;
                if (!ch) {
                    Log("Weirdness. Found a char in arena, but now he's gone?");
                }

                if (IS_PC(ch) && roomFindNum(ch->in_room)->zone == ARENA_ZONE) {
                    /*
                     * we have a winner - move and declare
                     */
                    if ((location = ch->player.hometown)) {
                        char_from_room(ch);
                        char_to_room(ch, location);
                        send_to_char("You have won the arena, and are sent "
                                     "back home.\n\r\n\r", ch);
                        act("$n appears in the middle of the room.",
                            TRUE, ch, 0, 0, TO_ROOM);
                        do_look(ch, NULL, 15);
                        send_to_char("\n\r", ch);
                    }
                    sprintf(buf, "%s has been declared winner of this "
                                 "Arena!!\n\r", GET_NAME(ch));
                    SendToAll(buf);
                }
            }
        }

        if (!ch) {
            Log("Weirdness. Found a char in arena, but now he's gone?");
        }
    }
}

void AuctionPulseStuff(int pulse)
{
    struct obj_data *auctionobj;
    char            buf[MAX_STRING_LENGTH];

    if (pulse < 0 || !auctioneer ||
        !(auctionobj = auctioneer->specials.auction)) {
        return;
    }

    switch (auct_loop) {
    case 1:
    case 2:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c.  Minimum bid set at "
                     "$c000w%d$c000c coins.\n\r",
                auctionobj->short_description, minbid);
        SendToAll(buf);
        auct_loop++;
        break;
    case 3:
        sprintf(buf, "$c000cAuction:  No interest in $c000w%s$c000c.  Item "
                     "withdrawn.\n\r", auctionobj->short_description);
        SendToAll(buf);
        auct_loop = 0;
        intbid = 0;
        minbid = 0;

        /*
         * return item to auctioneer
         */
        auctioneer->specials.auction = 0;

        assert(!auctionobj->in_obj);
        assert(auctionobj->in_room == NOWHERE);
        assert(!auctionobj->carried_by);

        auctionobj->equipped_by = 0;
        auctionobj->eq_pos = -1;

        objectGiveToChar(auctionobj, auctioneer);
        send_to_char("Your item is returned to you.\n\r.", auctioneer);
        do_save(auctioneer, "", 0);

        auct_loop = 0;
        intbid = 0;
        minbid = 0;
        bidder = 0;
        auctioneer = 0;
        break;

    case 4:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c, current bid of "
                     "$c000w%ld$c000c coins, to $c000w%s$c000c.  Going "
                     "once..\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        SendToAll(buf);
        auct_loop++;
        break;

    case 5:
        sprintf(buf, "$c000cAuction:  $c000w%s$c000c, current bid of "
                     "$c000w%ld$c000c coins, to $c000w%s$c000c.  Going "
                     "twice...\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        SendToAll(buf);
        auct_loop++;
        break;

    case 6:
        sprintf(buf, "$c000cAuction:  Gone!  $c000w%s$c000c was sold for "
                     "$c000w%ld$c000c coins to $c000w%s$c000c.\n\r",
                auctionobj->short_description, intbid, GET_NAME(bidder));
        SendToAll(buf);

        /*
         * return money to auctioneer
         */
        GET_GOLD(auctioneer) += intbid;
        oldSendOutput(auctioneer, "You receive %ld coins for the item you "
                              "auctioned.\n\r", intbid);
        /*
         * return item to bidder
         */
        oldSendOutput(bidder, "You receive %s.\n\r", auctionobj->short_description);
        auctioneer->specials.auction = 0;
        auctionobj->in_obj = 0;
        auctionobj->in_room = -1;
        auctionobj->carried_by = 0;

        auctionobj->equipped_by = 0;
        auctionobj->eq_pos = -1;

        objectGiveToChar(auctionobj, bidder);

        bidder->specials.minbid = 0;

        do_save(bidder, "", 0);
        do_save(auctioneer, "", 0);

        auct_loop = 0;
        intbid = 0;
        minbid = 0;
        bidder = 0;
        auctioneer = 0;
        break;

    default:
        break;
    }
}

void TrollRegenPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;

    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            if (IS_PC(ch) && GET_RACE(ch) == RACE_TROLL &&
                GET_HIT(ch) < GET_MAX_HIT(ch)) {
                troll_regen(ch);
            }
        }
    }
}

bool MakeSound_cond( struct char_data *ch, void *arg )
{
    if (!IS_PC(ch) && ch->player.sounds && !number(0, 5) &&
        strcmp(ch->player.sounds, "")) {
        return( TRUE );
    }
    return( FALSE );
}

bool MakeSound_callback( struct char_data *ch, void *arg )
{
    if (ch->specials.default_pos > POSITION_SLEEPING) {
        if (GET_POS(ch) > POSITION_SLEEPING) {
            /*
             * Make the sound
             */
            MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
        } else if (GET_POS(ch) == POSITION_SLEEPING) {
            /*
             * snore
             */
            sprintf(buffer, "%s snores loudly.\n\r", ch->player.short_descr);
            MakeNoise(ch->in_room, buffer, "You hear a loud snore nearby.\n\r");
        }
    } else if (GET_POS(ch) == ch->specials.default_pos) {
        /*
         * Make the sound
         */
        MakeNoise(ch->in_room, ch->player.sounds, ch->player.distant_snds);
    }
}


void RiverPulseStuff(int pulse)
{
    struct descriptor_data *i;
    register struct char_data *ch;
    struct char_data *tmp;
    register struct obj_data *obj_object;
    struct obj_data *next_obj;
    int             rd,
                    or;
    char            buf[80],
                    buffer[100];
    struct room_data *rp;
    LinkedListItem_t   *item,
                       *nextItem;

    if (pulse < 0) {
        return;
    }

    for (i = descriptor_list; i; i = i->next) {
        if (!i->connected) {
            ch = i->character;
            if ((IS_PC(ch) || RIDDEN(ch)) && ch->in_room != NOWHERE &&
                (rp = roomFindNum(ch->in_room) &&
                (rp->sector_type == SECT_WATER_NOSWIM ||
                 rp->sector_type == SECT_UNDERWATER) &&
                rp->river_speed > 0 && !(pulse % rp->river_speed) &&
                rp->river_dir <= 5 && rp->river_dir >= 0) {

                rd = rp->river_dir;

                LinkedListLock( rp->contentList );
                for( item = rp->contentList->head; item; item = nextItem ) {
                    nextItem = item->next;
                    obj_object = CONTENT_LINK_TO_OBJ(item);

                    if (rp->dir_option[rd]) {
                        objectTakeFromRoom(obj_object, LOCKED);
                        objectPutInRoom(obj_object,
                                        rp->dir_option[rd]->to_room, UNLOCKED);
                    }
                }
                LinkedListUnlock( rp->contentList );

                /*
                 * flyers don't get moved
                 */
                if (IS_IMMORTAL(ch) && IS_SET(ch->specials.act, PLR_NOHASSLE)) {
                    send_to_char("The waters swirl and eddy about you.\n\r",
                                 ch);
                } else if ((!IS_AFFECTED(ch, AFF_FLYING) ||
                            roomFindNum(ch->in_room)->sector_type ==
                               SECT_UNDERWATER) && !MOUNTED(ch)) {
                    if (rp && rp->dir_option[rd] &&
                        rp->dir_option[rd]->to_room &&
                        EXIT(ch, rd)->to_room != NOWHERE) {
                        if (ch->specials.fighting) {
                            stop_fighting(ch);
                        }
                        sprintf(buf, "You drift %s...\n\r", direction[rd].dir);
                        send_to_char(buf, ch);
                        if (RIDDEN(ch)) {
                            send_to_char(buf, RIDDEN(ch));
                        }

                        char_from_room(ch);
                        if (RIDDEN(ch)) {
                            char_from_room(RIDDEN(ch));
                            char_to_room(RIDDEN(ch),
                                         rp->dir_option[rd]->to_room);
                        }
                        char_to_room(ch, rp->dir_option[rd]->to_room);
                        do_look(ch, NULL, 15);
                        if (RIDDEN(ch)) {
                            do_look(RIDDEN(ch), NULL, 15);
                        }

                        if (IS_SET(RM_FLAGS(ch->in_room), DEATH) &&
                            !IS_IMMORTAL(ch)) {
                            NailThisSucker(ch);
                            if (RIDDEN(ch)) {
                                NailThisSucker(RIDDEN(ch));
                            }
                        }
                    }
                }
            }
        }
    }

    /**
     * @todo WTF is the MakeSound stuff doing inside RiverPulseStuff()??
     */
    if (!number(0, 4)) {
        playerFindAll( MakeSound_cond, NULL, MakeSound_callback, NULL, NULL );
    }
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

