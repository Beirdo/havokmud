/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008 Gavin Hurlbut
 *
 *  havokmud is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*HEADER---------------------------------------------------
* $Id$
*
* Copyright 2008 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "structs.h"
#include "newstructs.h"
#include "protos.h"
#include "utils.h"
#include "logging.h"
#include "interthread.h"
#include "balanced_btree.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Handles loading, saving of mobiles
 */

#define MOB_DIR "mobiles"
FILE           *mob_f;           /* file containing mob prototypes */

int             top_of_mobt = 0;        /* top of mobile index table */
int             top_of_sort_mobt = 0;
int             top_of_alloc_mobt = 99999;
int             mob_tick_count = 0;
int             top_of_scripts = 0;


BalancedBTree_t *mobileMasterTree  = NULL;
BalancedBTree_t *mobileKeywordTree = NULL;
BalancedBTree_t *mobileTypeTree    = NULL;

long            mob_count = 0;

int             read_mob_from_file(struct char_data *mob, FILE * mob_fi);
int             read_mob_from_new_file(struct char_data *mob,
                                       FILE * mob_fi);


void initializeMobiles( void )
{
    mobileMasterTree = BalancedBTreeCreate( BTREE_KEY_INT );
    mobileKeywordTree = BalancedBTreeCreate( BTREE_KEY_STRING );
    mobileTypeTree = BalancedBTreeCreate( BTREE_KEY_INT );
#if 0
    db_load_mobile_tree( mobileMasterTree );
#endif
}

struct index_data *mobileIndex( int vnum )
{
    struct index_data      *index;
    BalancedBTreeItem_t    *item;

    item = BalancedBTreeFind( mobileMasterTree, &vnum, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    index = (struct index_data *)item->item;
    return( index );
}


void mobileInsert(struct char_data *mob, long vnum)
{
    BalancedBTreeItem_t    *item;
    struct index_data      *index;

    CREATE(index, struct index_data, 1);
    CREATE(item, BalancedBTreeItem_t, 1);

    index->vnum = vnum;
    StringToKeywords( GET_NAME(mob), &index->keywords );
    index->number = 0;
    index->func = NULL;
    index->list = LinkedListCreate();

    item->key = &index->vnum;
    item->item = (void *)index;
    BalancedBTreeAdd( mobileMasterTree, item, UNLOCKED, TRUE );
}


/*
 * read a mobile from MOB_FILE
 */

typedef struct {
    int_func    func;
    long        actbit;
} CommonProcDef_t;

static CommonProcDef_t procDefs[] = {
    { NULL, 0 },                    /* PROC_NONE */
    { shopkeeper, 0 },              /* PROC_SHOPKEEPER */
    { generic_guildmaster, 0 },     /* PROC_GUILDMASTER */
    { Tyrannosaurus_swallower, 0 }, /* PROC_SWALLOWER */
    { NULL, 0 },                    /* PROC_DRAIN */
    { QuestMobProc, 0 },            /* PROC_QUEST */
    { BreathWeapon, 0 },            /* PROC_OLD_BREATH */
    { FireBreather, 0 },            /* PROC_FIRE_BREATH */
    { GasBreather, 0 },             /* PROC_GAS_BREATH */
    { FrostBreather, 0 },           /* PROC_FROST_BREATH */
    { AcidBreather, 0 },            /* PROC_ACID_BREATH */
    { LightningBreather, 0 },       /* PROC_LIGHTNING_BREATH */
    { DehydBreather, 0 },           /* PROC_DEHYDRATION_BREATH */
    { VaporBreather, 0 },           /* PROC_VAPOR_BREATH */
    { SoundBreather, 0 },           /* PROC_SOUND_BREATH */
    { ShardBreather, 0 },           /* PROC_SHARD_BREATH */
    { SleepBreather, 0 },           /* PROC_SLEEP_BREATH */
    { LightBreather, 0 },           /* PROC_LIGHT_BREATH */
    { DarkBreather, 0 },            /* PROC_DARK_BREATH */
    { receptionist, ACT_SENTINEL }, /* PROC_RECEPTIONIST */
    { RepairGuy, 0 }                /* PROC_REPAIRGUY */
};
static int procDefCount = NELEMENTS(procDefs);

struct char_data *mobileRead(int nr)
{
    struct char_data   *mob;
    struct index_data  *index;
    int                 procNum;

    index = mobileIndex( nr );
    if( !index ) {
        Log("Mobile (V) %d does not exist in database.", nr);
        return( NULL );
    }

    CREATE(mob, struct char_data, 1);
    if (!mob) {
        Log("Cannot create mob?!");
        return( NULL );
    }

    clear_char(mob);

#if 0
    if( !db_read_mobile(mob, nr) ) {
        free_char( mob );
        return( NULL );
    }
#endif

    /*
     * assign common proc flags
     */
    procNum = mob->specials.proc;
    if( procNum >= PROC_NONE && procNum < procDefCount ) {
        if( procDefs[procNum].func ) {
            index->func = procDefs[procNum].func;
        }
        SET_BIT( mob->specials.act, procDefs[procNum].actbit );
    }

    mob_count++;

    return (mob);
}


int read_mob_from_new_file(struct char_data *mob, FILE * mob_fi)
{
    int             savebase = 40;
    int             i,
                    nr;
    long            tmp,
                    tmp2,
                    tmp3,
                    bc = 0;
    float           att;
    struct index_data *index;

    nr = mob->nr;

    mob->player.name = fread_string(mob_fi);
    if (*mob->player.name) {
        bc += strlen(mob->player.name);
    }
    mob->player.short_descr = fread_string(mob_fi);
    if (*mob->player.short_descr) {
        bc += strlen(mob->player.short_descr);
    }
    mob->player.long_descr = fread_string(mob_fi);
    if (*mob->player.long_descr) {
        bc += strlen(mob->player.long_descr);
    }
    mob->player.description = fread_string(mob_fi);
    if (mob->player.description && *mob->player.description) {
        bc += strlen(mob->player.description);
    }
    mob->player.sounds = fread_string(mob_fi);
    if (mob->player.sounds && *mob->player.sounds) {
        bc += strlen(mob->player.sounds);
    }
    mob->player.distant_snds = fread_string(mob_fi);
    if (mob->player.distant_snds && *mob->player.distant_snds) {
        bc += strlen(mob->player.distant_snds);
    }
    mob->player.title = 0;

    /*
     *** Numeric data ***
     */
    mob->specials.spellfail = 101;

    fscanf(mob_fi, "%ld ", &tmp);
    mob->specials.act = tmp;
    SET_BIT(mob->specials.act, ACT_ISNPC);

    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.affected_by = tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.alignment = tmp;

    fscanf(mob_fi, " %f \n", &att);
    mob->mult_att = att;

    fscanf(mob_fi, " %ld ", &tmp);
    GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

    mob->abilities.str = 9 + number(1, (MAX(1, tmp / 5 - 1)));
    mob->abilities.intel = 9 + number(1, (MAX(1, tmp / 5 - 1)));
    mob->abilities.wis = 9 + number(1, (MAX(1, tmp / 5 - 1)));
    mob->abilities.dex = 9 + number(1, (MAX(1, tmp / 5 - 1)));
    mob->abilities.con = 9 + number(1, (MAX(1, tmp / 5 - 1)));
    mob->abilities.chr = 9 + number(1, (MAX(1, tmp / 5 - 1)));

    fscanf(mob_fi, " %ld ", &tmp);
    mob->points.hitroll = 20 - tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    if (tmp > 10 || tmp < -10) {
        tmp /= 10;
    }
    mob->points.armor = 10 * tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->points.max_hit = tmp;
    mob->points.hit = tmp;
#if 0
    mob->points.max_hit = dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8)+tmp;
    mob->points.hit = mob->points.max_hit;
#endif
    fscanf(mob_fi, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
    mob->points.damroll = tmp3;
    mob->specials.damnodice = tmp;
    mob->specials.damsizedice = tmp2;

    mob->points.mana = 10;
    mob->points.max_mana = 10;

    mob->points.move = 50;
    mob->points.max_move = 50;

    fscanf(mob_fi, " -1 ");

    fscanf(mob_fi, " %ld ", &tmp);
    mob->points.gold = (int)((float) ((number(900, 1100) * tmp) / 1000));

    fscanf(mob_fi, " %ld ", &tmp);
    if (tmp < 1) {
        tmp = 1;
    }
    GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);

    fscanf(mob_fi, " %ld \n", &tmp);
    GET_RACE(mob) = tmp;

    if (IsGiant(mob)) {
        mob->abilities.str += number(1, 4);
    }
    if (IsSmall(mob)) {
        mob->abilities.str -= 1;
    }

    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.position = tmp;
    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.default_pos = tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->player.sex = tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->immune = tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->M_immune = tmp;

    fscanf(mob_fi, " %ld \n", &tmp);
    mob->susc = tmp;

    fscanf(mob_fi, " %ld \n", &tmp);
    mob->specials.proc = tmp;

    mob->specials.talks = fread_string(mob_fi);
    if (mob->specials.talks && *mob->specials.talks) {
        bc += strlen(mob->specials.talks);
    }
    mob->specials.quest_yes = fread_string(mob_fi);
    if (mob->specials.quest_yes && *mob->specials.quest_yes) {
        bc += strlen(mob->specials.quest_yes);
    }
    mob->specials.quest_no = fread_string(mob_fi);
    if (mob->specials.quest_no && *mob->specials.quest_no) {
        bc += strlen(mob->specials.quest_no);
    }
    mob->player.class = 0;
    mob->player.time.birth = time(0);
    mob->player.time.played = 0;
    mob->player.time.logon = time(0);
    mob->player.weight = 200;
    mob->player.height = 198;

    for (i = 0; i < 3; i++) {
        GET_COND(mob, i) = -1;
    }
    for (i = 0; i < MAX_SAVES; i++) {
#if 0
        mob->specials.apply_saving_throw[i] = MAX(20-GET_LEVEL(mob,
              WARRIOR_LEVEL_IND), 2);
#endif
        savebase = 40;
        if (number(0, 1)) {
            savebase += number(0, 3);
        } else {
            savebase -= number(0, 3);
        }
        mob->specials.apply_saving_throw[i] = MAX(savebase -
                (int) (GET_LEVEL(mob, WARRIOR_LEVEL_IND) * 0.533), 6);
    }

    mob->tmpabilities = mob->abilities;

    for (i = 0; i < MAX_WEAR; i++) {
        /*
         * Initialisering Ok
         */
        mob->equipment[i] = NULL;
    }
#if 0
    mob->desc = 0;
#endif

    if (!IS_SET(mob->specials.act, ACT_ISNPC)) {
        SET_BIT(mob->specials.act, ACT_ISNPC);
    }
    mob->generic = 0;
    mob->commandp = 0;
    mob->commandp2 = 0;
    mob->waitp = 0;

    mob->last_tell = NULL;

    /*
     * Check to see if associated with a script, if so, set it up
     */
    if (IS_SET(mob->specials.act, ACT_SCRIPT)) {
        REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
    }
    /** @todo script structure inside mob structure or onto its own tree */
    for (i = 0; i < top_of_scripts; i++) {
        if (script_data[i].vnum == nr) {
            SET_BIT(mob->specials.act, ACT_SCRIPT);
            mob->script = i;
            break;
        }
    }
    /*
     * tell the spec_proc (if there is one) that we've been born
     */
    /*
     * insert in list
     */

    mob->next = character_list;
    character_list = mob;

    /*
     * Meh this creates such a lot of spam
     */
#if 0
    if (mob->points.gold >GET_LEVEL(mob, WARRIOR_LEVEL_IND)*1500) {
        char buf[200];
        Log("%s has gold > level * 1500 (%d)", mob->player.short_descr,
            mob->points.gold);
    }
#endif

    /*
     * set up things that all members of the race have
     */
    SetRacialStuff(mob);
    SpaceForSkills(mob);
    SetDefaultLang(mob);

    /*
     * change exp for wimpy mobs (lower)
     */
    if (IS_SET(mob->specials.act, ACT_WIMPY)) {
        GET_EXP(mob) -= GET_EXP(mob) / 10;
    }
    /*
     * change exp for agressive mobs (higher)
     */
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        GET_EXP(mob) += GET_EXP(mob) / 10;
        /*
         * big bonus for fully aggressive mobs for now
         */
        if (!IS_SET(mob->specials.act, ACT_WIMPY)
            || IS_SET(mob->specials.act, ACT_META_AGG)) {
            GET_EXP(mob) += (GET_EXP(mob) / 2);
        }
    }

    /*
     * set up distributed movement system
     */
    mob->specials.tick = mob_tick_count++;

    if (mob_tick_count == TICK_WRAP_COUNT) {
        mob_tick_count = 0;
    }
    index = mobileIndex(nr);
    index->number++;

#ifdef BYTE_COUNT
    fprintf(stderr, "Mobile [%d]: byte count: %d\n", nr, bc);
#endif
    return (bc);
}

/*
 * ---------- Start of write_mob_to_file ----------
 */
/*
 * write a mobile to a file
 */

 /*
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
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
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
  */

void write_mob_to_file(struct char_data *mob, FILE * mob_fi)
{
    int             i,
                    xpflag;
    long            tmp;
    float           tmpexp;
    char            buff[MAX_STRING_LENGTH];

    /*
     *** String data ***
     */
    fwrite_string(mob_fi, mob->player.name);
    fwrite_string(mob_fi, mob->player.short_descr);
    remove_cr(buff, mob->player.long_descr);
    fwrite_string(mob_fi, buff);
    remove_cr(buff, mob->player.description);
    fwrite_string(mob_fi, buff);
    if (mob->player.sounds) {
        remove_cr(buff, mob->player.sounds);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->player.distant_snds) {
        remove_cr(buff, mob->player.distant_snds);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }

    /*
     *** Numeric data ***
     */
    tmp = mob->specials.act;
    REMOVE_BIT(tmp, ACT_ISNPC);
    REMOVE_BIT(tmp, ACT_HATEFUL);
    REMOVE_BIT(tmp, ACT_GUARDIAN);
    REMOVE_BIT(tmp, ACT_HUNTING);
    REMOVE_BIT(tmp, ACT_AFRAID);

    fprintf(mob_fi, "%ld ", tmp);
    fprintf(mob_fi, " %ld ", mob->specials.affected_by);
    fprintf(mob_fi, " %d ", mob->specials.alignment);
    fprintf(mob_fi, " %.1f \n", mob->mult_att);

    fprintf(mob_fi, " %d ", GET_LEVEL(mob, WARRIOR_LEVEL_IND));
    fprintf(mob_fi, " %d ", (mob->points.hitroll - 20) * -1);
    fprintf(mob_fi, " %d ", mob->points.armor / 10);
    fprintf(mob_fi, " %d ", mob->points.max_hit);
    fprintf(mob_fi, " %dd%d+%d \n", mob->specials.damnodice,
            mob->specials.damsizedice, mob->points.damroll);

    tmpexp = GET_EXP(mob);

    /*
     * revert exp for agressive mobs (lower)
     */
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        /*
         * big bonus for fully aggressive mobs for now
         */
        if (!IS_SET(mob->specials.act, ACT_WIMPY) ||
            IS_SET(mob->specials.act, ACT_META_AGG)) {
            tmpexp = tmpexp / 1.5;
        }
        tmpexp = tmpexp / 1.1;
    }
    /*
     * revert exp for wimpy mobs (higher)
     */
    if (IS_SET(mob->specials.act, ACT_WIMPY)) {
        tmpexp = tmpexp / 0.9;
    }
    i = (int) tmpexp;

    fprintf(mob_fi, " %d ", -1);
    fprintf(mob_fi, " %d ", mob->points.gold);

#if 0
    xpflag = GetExpFlags(mob, i);
#else
    xpflag = 0;
#endif
    if (xpflag < 1) {
        xpflag = 1;
    }
    if (xpflag > 31) {
        xpflag = 31;
    }
    fprintf(mob_fi, " %d ", xpflag);
    fprintf(mob_fi, " %d \n", GET_RACE(mob));

    fprintf(mob_fi, " %d ", mob->specials.position);
    fprintf(mob_fi, " %d ", mob->specials.default_pos);

    fprintf(mob_fi, " %d ", mob->player.sex);
    fprintf(mob_fi, " %ld ", mob->immune);
    fprintf(mob_fi, " %ld ", mob->M_immune);
    fprintf(mob_fi, " %ld \n", mob->susc);

    fprintf(mob_fi, " %d \n", mob->specials.proc);

    if (mob->specials.talks) {
        remove_cr(buff, mob->specials.talks);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->specials.quest_yes) {
        remove_cr(buff, mob->specials.quest_yes);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->specials.quest_no) {
        remove_cr(buff, mob->specials.quest_no);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
}

void save_new_mobile_structure(struct char_data *mob, FILE * mob_fi)
{
    int             i,
                    xpflag;
    long            tmp;
    float           tmpexp;
    char            buff[MAX_STRING_LENGTH];

    /*
     *** String data ***
     */
    fwrite_string(mob_fi, mob->player.name);
    fwrite_string(mob_fi, mob->player.short_descr);
    remove_cr(buff, mob->player.long_descr);
    fwrite_string(mob_fi, buff);
    remove_cr(buff, mob->player.description);
    fwrite_string(mob_fi, buff);
    if (mob->player.sounds) {
        remove_cr(buff, mob->player.sounds);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->player.distant_snds) {
        remove_cr(buff, mob->player.distant_snds);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }

    /*
     *** Numeric data ***
     */
    tmp = mob->specials.act;
    REMOVE_BIT(tmp, ACT_ISNPC);
    REMOVE_BIT(tmp, ACT_HATEFUL);
    REMOVE_BIT(tmp, ACT_GUARDIAN);
    REMOVE_BIT(tmp, ACT_HUNTING);
    REMOVE_BIT(tmp, ACT_AFRAID);

    fprintf(mob_fi, "%ld ", tmp);
    fprintf(mob_fi, " %ld ", mob->specials.affected_by);
    fprintf(mob_fi, " %d ", mob->specials.alignment);
    fprintf(mob_fi, " %.1f \n", mob->mult_att);

    fprintf(mob_fi, " %d ", GET_LEVEL(mob, WARRIOR_LEVEL_IND));
    fprintf(mob_fi, " %d ", (mob->points.hitroll - 20) * -1);
    fprintf(mob_fi, " %d ", mob->points.armor / 10);
    fprintf(mob_fi, " %d ", mob->points.max_hit);
    fprintf(mob_fi, " %dd%d+%d \n", mob->specials.damnodice,
            mob->specials.damsizedice, mob->points.damroll);

    tmpexp = GET_EXP(mob);

    /*
     * revert exp for agressive mobs (lower)
     */
    if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        /*
         * big bonus for fully aggressive mobs for now
         */
        if (!IS_SET(mob->specials.act, ACT_WIMPY) ||
            IS_SET(mob->specials.act, ACT_META_AGG)) {
            tmpexp = tmpexp / 1.5;
        }
        tmpexp = tmpexp / 1.1;
    }

    /*
     * revert exp for wimpy mobs (higher)
     */
    if (IS_SET(mob->specials.act, ACT_WIMPY)) {
        tmpexp = tmpexp / 0.9;
    }
    i = (int) tmpexp;
    if (mob->points.gold < 0) {
        mob->points.gold = 0;
    }
    i -= mob->points.gold;

    fprintf(mob_fi, " %d ", -1);
    fprintf(mob_fi, " %d ", mob->points.gold);

#if 0
    xpflag = GetExpFlags(mob, i);
#else
    xpflag = 0;
#endif
    if (xpflag < 1) {
        xpflag = 1;
    }
    if (xpflag > 31) {
        xpflag = 31;
    }
    fprintf(mob_fi, " %d ", xpflag);
    fprintf(mob_fi, " %d \n", GET_RACE(mob));

    fprintf(mob_fi, " %d ", mob->specials.position);
    fprintf(mob_fi, " %d ", mob->specials.default_pos);

    fprintf(mob_fi, " %d ", mob->player.sex);
    fprintf(mob_fi, " %ld ", mob->immune);
    fprintf(mob_fi, " %ld ", mob->M_immune);
    fprintf(mob_fi, " %ld \n", mob->susc);

    fprintf(mob_fi, " %d \n", mob->specials.proc);

    if (mob->specials.talks) {
        remove_cr(buff, mob->specials.talks);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->specials.quest_yes) {
        remove_cr(buff, mob->specials.quest_yes);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
    if (mob->specials.quest_no) {
        remove_cr(buff, mob->specials.quest_no);
        fwrite_string(mob_fi, buff);
    } else {
        fwrite_string(mob_fi, "");
    }
}

void InitScripts(void)
{
    char            buf[255],
                    buf2[255];
    FILE           *f1,
                   *f2;
    int             i,
                    count;
    struct char_data *mob;

    if (!script_data) {
        top_of_scripts = 0;
    }
    return;
    /*
     * DISABLED
     */

    /*
     * what is ths for? turn off all the scripts ???
     * -yes, just in case the script file was removed, saves pointer probs
     */

    for (mob = character_list; mob; mob = mob->next) {
        if (IS_MOB(mob) && IS_SET(mob->specials.act, ACT_SCRIPT)) {
            mob->commandp = 0;
            REMOVE_BIT(mob->specials.act, ACT_SCRIPT);
        }
    }
    if (!(f1 = fopen("scripts.dat", "r"))) {
        Log("Unable to open file \"scripts.dat\".");
        return;
    }

    if (script_data) {
        for (; i < top_of_scripts; i++) {
            if (script_data[i].script) {
                free(script_data[i].script);
            }
            if (script_data[i].filename) {
                free(script_data[i].filename);
            }
        }
        if (script_data) {
            free(script_data);
        }
        top_of_scripts = 0;
    }

    script_data = NULL;
    script_data = (struct scripts *) malloc(sizeof(struct scripts));

    while (1) {
        if (fgets(buf, 254, f1) == NULL) {
            break;
        }
        if (buf[strlen(buf) - 1] == '\n') {
            buf[strlen(buf) - 1] = '\0';
        }
        sscanf(buf, "%s %d", buf2, &i);

        sprintf(buf, "scripts/%s", buf2);
        if (!(f2 = fopen(buf, "r"))) {
            Log("Unable to open script \"%s\" for reading.", buf2);
        } else {
            script_data = (struct scripts *) realloc(script_data,
                               (top_of_scripts + 1) * sizeof(struct scripts));

            count = 0;
            while (!feof(f2)) {
                fgets(buf, 254, f2);
                if (buf[strlen(buf) - 1] == '\n') {
                    buf[strlen(buf) - 1] = '\0';
                }
                /*
                 * you really don't want to do a lot of reallocs all at
                 * once
                 */
                if (count == 0) {
                    script_data[top_of_scripts].script =
                        (struct foo_data *)malloc(sizeof(struct foo_data));
                } else {
                    script_data[top_of_scripts].script = (struct foo_data *)
                        realloc(script_data[top_of_scripts].script,
                                sizeof(struct foo_data) * (count + 1));
                }
                script_data[top_of_scripts].script[count].line =
                    (char *) malloc(sizeof(char) * (strlen(buf) + 1));

                strcpy(script_data[top_of_scripts].script[count].line, buf);

                count++;
            }

            script_data[top_of_scripts].vnum = i;
            script_data[top_of_scripts].filename =
                (char *) malloc((strlen(buf2) + 1) * sizeof(char));
            strcpy(script_data[top_of_scripts].filename, buf2);
            Log("Script %s assigned to mobile %d.", buf2, i);
            top_of_scripts++;
            fclose(f2);
        }
    }

    if (top_of_scripts) {
        Log("%d scripts assigned.", top_of_scripts);
    } else {
        Log("No scripts found to assign.");
    }

    fclose(f1);
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
