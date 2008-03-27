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
* $Id: object_data.c 1670 2008-03-26 22:53:22Z gjhurlbu $
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
    "$Id: object_data.c 1670 2008-03-26 22:53:22Z gjhurlbu $";

/**
 * @file
 * @brief Handles loading, saving of mobiles
 */

#define MOB_DIR "mobiles"
FILE           *mob_f;           /* file containing mob prototypes */

struct index_data *mob_index;   /* index table for mobile file */
int             top_of_mobt = 0;        /* top of mobile index table */
int             top_of_sort_mobt = 0;
int             top_of_alloc_mobt = 99999;
int             mob_tick_count = 0;
int             top_of_scripts = 0;
long            total_mbc = 0;


BalancedBTree_t *mobileMasterTree  = NULL;
BalancedBTree_t *mobileKeywordTree = NULL;
BalancedBTree_t *mobileTypeTree    = NULL;

long            mob_count = 0;

struct index_data *generate_indices(FILE * fl, int *top, int *sort_top,
                                    int *alloc_top, char *dirname);
struct index_data *insert_index(struct index_data *index, void *data,
                                long vnum);
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


#if 0
/*
 * generate index table for object or monster file
 */
struct index_data *generate_indices(FILE * fl, int *top, int *sort_top,
                                    int *alloc_top, char *dirname)
{
    FILE           *f;
    DIR            *dir;
    struct index_data *index = NULL;
    struct dirent  *ent;
    long            i = 0,
                    di = 0,
                    vnum,
                    j;
    long            bc = MAX_INDICES;
    long            dvnums[MAX_INDICES];
    char            buf[82],
                    tbuf[128];

    /*
     * scan main obj file
     */
    rewind(fl);
    for (;;) {
        if (fgets(buf, sizeof(buf), fl)) {
            if (*buf == '#') {
                if (!i) {
                    /*
                     * first cell
                     */
                    CREATE(index, struct index_data, bc);
                } else if (i >= bc) {
                    if (!(index = (struct index_data *) realloc(index,
                                      (i + 50) * sizeof(struct index_data)))) {
                        perror("load indices");
                        assert(0);
                    }
                    bc += 50;
                }
                sscanf(buf, "#%ld", &index[i].vnum);
                sprintf(tbuf, "%s/%ld", dirname, index[i].vnum);
                if ((f = fopen(tbuf, "rt")) == NULL) {
                    index[i].pos = ftell(fl);
                    index[i].name = (index[i].vnum < 99999) ?
                                     fread_string(fl) : strdup("omega");
                } else {
                    index[i].pos = -1;
                    fscanf(f, "#%*d\n");
                    index[i].name = (index[i].vnum < 99999) ?
                                     fread_string(f) : strdup("omega");
                    dvnums[di++] = index[i].vnum;
                    fclose(f);
                }
                index[i].number = 0;
                index[i].func = 0;
                index[i].data = NULL;
                i++;
            } else {
                if (*buf == '%') {
                    /* EOF */
                    break;
                }
            }
        } else {
            fprintf(stderr, "generate indices");
            assert(0);
        }
    }
    *sort_top = i - 1;
    *alloc_top = bc;
    *top = i;

    /*
     * scan for directory entrys
     */
    if ((dir = opendir(dirname)) == NULL) {
        Log("unable to open index directory %s", dirname);
        return (index);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (*ent->d_name == '.') {
            continue;
        }
        vnum = atoi(ent->d_name);
        if (vnum == 0) {
            continue;
        }
        /*
         * search if vnum was already sorted in main database
         */
        for (j = 0; j < di; j++) {
            if (dvnums[j] == vnum) {
                break;
            }
        }

        if (dvnums[j] == vnum) {
            continue;
        }

        sprintf(buf, "%s/%s", dirname, ent->d_name);
        if ((f = fopen(buf, "rt")) == NULL) {
            Log("Can't open file %s for reading\n", buf);
            continue;
        }

        if (!i) {
            CREATE(index, struct index_data, bc);
        } else if (i >= bc) {
            if (!(index = (struct index_data *) realloc(index,
                          (i + 50) * sizeof(struct index_data)))) {
                perror("load indices");
                assert(0);
            }
            bc += 50;
        }

        fscanf(f, "#%*d\n");
        index[i].vnum = vnum;
        index[i].pos = -1;
        index[i].name = (index[i].vnum < 99999) ? fread_string(f) :
                         strdup("omega");
        index[i].number = 0;
        index[i].func = 0;
        index[i].data = NULL;
        fclose(f);
        i++;
    }

    *alloc_top = bc;
    *top = i;
    closedir(dir);

    return (index);
}
#endif

/*
 * read a mobile from MOB_FILE
 */

/** @todo This is borked as it is now indexing mob_index using vnums.
 *  @todo rewrite to use trees/lists like the objects
 */
struct char_data *read_mobile(int nr)
{
    struct char_data *mob;
    int             i;
    long            bc;
    char            buf[100];

    i = nr;
    if (nr < 0) {
        sprintf(buf, "Mobile (V) %d does not exist in database.", i);
        return( NULL );
    }

    CREATE(mob, struct char_data, 1);

    if (!mob) {
        Log("Cannot create mob?! db.c read_mobile");
        return (FALSE);
    }

    bc = sizeof(struct char_data);
    clear_char(mob);
    mob->nr = nr;

    /*
     * mobile in external file
     */
#if 0
    if (mob_index[nr].pos == -1) {
        sprintf(buf, "%s/%ld", MOB_DIR, mob_index[nr].vnum);
        if ((f = fopen(buf, "rt")) == NULL) {
            Log("can't open mobile file for mob %ld", mob_index[nr].vnum);
            free_char(mob);
            return (0);
        }
        fscanf(f, "#%*d\n");

        bc += read_mob_from_new_file(mob, f);
        fclose(f);
    } else {
        rewind(mob_f);
        fseek(mob_f, mob_index[nr].pos, 0);
        bc += read_mob_from_new_file(mob, mob_f);
    }
#endif

    total_mbc += bc;
    mob_count++;

    /*
     * assign common proc flags
     */
    switch( mob->specials.proc ) {
    case PROC_QUEST:
        mob_index[mob->nr].func = *QuestMobProc;
        break;
    case PROC_SHOPKEEPER:
        mob_index[mob->nr].func = *shopkeeper;
        break;
    case PROC_GUILDMASTER:
        mob_index[mob->nr].func = *generic_guildmaster;
        break;
    case PROC_SWALLOWER:
        mob_index[mob->nr].func = *Tyrannosaurus_swallower;
        break;
    case PROC_OLD_BREATH:
        mob_index[mob->nr].func = *BreathWeapon;
        break;
    case PROC_FIRE_BREATH:
        mob_index[mob->nr].func = *FireBreather;
        break;
    case PROC_GAS_BREATH:
        mob_index[mob->nr].func = *GasBreather;
        break;
    case PROC_FROST_BREATH:
        mob_index[mob->nr].func = *FrostBreather;
        break;
    case PROC_ACID_BREATH:
        mob_index[mob->nr].func = *AcidBreather;
        break;
    case PROC_LIGHTNING_BREATH:
        mob_index[mob->nr].func = *LightningBreather;
        break;
    case PROC_DEHYDRATION_BREATH:
        mob_index[mob->nr].func = *DehydBreather;
        break;
    case PROC_VAPOR_BREATH:
        mob_index[mob->nr].func = *VaporBreather;
        break;
    case PROC_SOUND_BREATH:
        mob_index[mob->nr].func = *SoundBreather;
        break;
    case PROC_SHARD_BREATH:
        mob_index[mob->nr].func = *ShardBreather;
        break;
    case PROC_SLEEP_BREATH:
        mob_index[mob->nr].func = *SleepBreather;
        break;
    case PROC_LIGHT_BREATH:
        mob_index[mob->nr].func = *LightBreather;
        break;
    case PROC_DARK_BREATH:
        mob_index[mob->nr].func = *DarkBreather;
        break;
    case PROC_RECEPTIONIST:
        mob_index[mob->nr].func = *receptionist;
        SET_BIT(mob->specials.act, ACT_SENTINEL);
        break;
    case PROC_REPAIRGUY:
        mob_index[mob->nr].func = *RepairGuy;
        break;
    case 0:
    default:
        break;
    }

    return (mob);
}

int read_mob_from_file(struct char_data *mob, FILE * mob_fi)
{
    int             savebase = 40;
    int             i,
                    nr;
    long            tmp,
                    tmp2,
                    tmp3,
                    bc = 0;
    char            letter;

    if( !mob ) {
        return( -1 );
    }
    
    memset( mob, 0, sizeof(struct char_data) );

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
    mob->player.title = 0;

    /*
     *** Numeric data ***
     */

    mob->mult_att = 1.0;
    mob->specials.spellfail = 101;

    fscanf(mob_fi, "%ld ", &tmp);
    mob->specials.act = tmp;
    SET_BIT(mob->specials.act, ACT_ISNPC);

    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.affected_by = tmp;

    fscanf(mob_fi, " %ld ", &tmp);
    mob->specials.alignment = tmp;

    tmp = 0;
    tmp2 = 0;
    tmp3 = 0;

    mob->player.class = CLASS_WARRIOR;

    fscanf(mob_fi, " %c ", &letter);

    if (letter == 'S') {
        fscanf(mob_fi, "\n");

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

        fscanf(mob_fi, " %ldd%ld+%ld ", &tmp, &tmp2, &tmp3);
        mob->points.max_hit = dice(tmp, tmp2) + tmp3;
        mob->points.hit = mob->points.max_hit;

        fscanf(mob_fi, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
        mob->points.damroll = tmp3;
        mob->specials.damnodice = tmp;
        mob->specials.damsizedice = tmp2;

        mob->points.mana = 10;
        mob->points.max_mana = 10;

        mob->points.move = 50;
        mob->points.max_move = 50;

        fscanf(mob_fi, " %ld ", &tmp);
        if (tmp == -1) {
            fscanf(mob_fi, " %ld ", &tmp);
            mob->points.gold = (int)((float)((number(900, 1100) * tmp) / 1000));

            fscanf(mob_fi, " %ld ", &tmp);
            tmp = MAX(tmp, GET_LEVEL(mob, WARRIOR_LEVEL_IND) *
                                          mob->points.max_hit);
            GET_EXP(mob) = tmp;

            fscanf(mob_fi, " %ld \n", &tmp);
            GET_RACE(mob) = tmp;
            if (IsGiant(mob)) {
                mob->abilities.str += number(1, 4);
            }
            if (IsSmall(mob)) {
                mob->abilities.str -= 1;
            }
        } else {
            mob->points.gold = (int)((float)((number(900, 1100) * tmp) / 1000));

            fscanf(mob_fi, " %ld \n", &tmp);
            if (tmp >= 0) {
                GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
            } else {
                GET_EXP(mob) = -tmp;
            }
            GET_EXP(mob) = tmp;
        }

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.position = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.default_pos = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        if (tmp < 3) {
            mob->player.sex = tmp;
            mob->immune = 0;
            mob->M_immune = 0;
            mob->susc = 0;
        } else if (tmp < 6) {
            mob->player.sex = (tmp - 3);

            fscanf(mob_fi, " %ld ", &tmp);
            mob->immune = tmp;

            fscanf(mob_fi, " %ld ", &tmp);
            mob->M_immune = tmp;

            fscanf(mob_fi, " %ld ", &tmp);
            mob->susc = tmp;
        } else {
            mob->player.sex = 0;
            mob->immune = 0;
            mob->M_immune = 0;
            mob->susc = 0;
        }

        fscanf(mob_fi, "\n");

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

    } else if ((letter == 'A') || (letter == 'N') || (letter == 'B') ||
               (letter == 'L')) {
        if ((letter == 'A') || (letter == 'B') || (letter == 'L')) {
            fscanf(mob_fi, " %ld ", &tmp);
            mob->mult_att = (float) tmp;
        }

        fscanf(mob_fi, "\n");

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
        mob->points.armor = 10 * tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->points.max_hit =
            dice(GET_LEVEL(mob, WARRIOR_LEVEL_IND), 8) + tmp;
        mob->points.hit = mob->points.max_hit;
#if 0
        HpBonus = mob->points.max_hit;
#endif
        fscanf(mob_fi, " %ldd%ld+%ld \n", &tmp, &tmp2, &tmp3);
        mob->points.damroll = tmp3;
        mob->specials.damnodice = tmp;
        mob->specials.damsizedice = tmp2;

        mob->points.mana = 10;
        mob->points.max_mana = 10;

        mob->points.move = 50;
        mob->points.max_move = 50;

        fscanf(mob_fi, " %ld ", &tmp);

        if (tmp == -1) {
            fscanf(mob_fi, " %ld ", &tmp);
            mob->points.gold = (int)((float)((number(900, 1100) * tmp) / 1000));

            fscanf(mob_fi, " %ld ", &tmp);
            if (tmp >= 0) {
                GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
            } else {
                GET_EXP(mob) = -tmp;
            }
            fscanf(mob_fi, " %ld ", &tmp);
            GET_RACE(mob) = tmp;

            if (IsGiant(mob)) {
                mob->abilities.str += number(1, 4);
            }
            if (IsSmall(mob)) {
                mob->abilities.str -= 1;
            }
        } else {
            mob->points.gold = (int)((float)((number(900, 1100) * tmp) / 1000));

            /*
             * this is where the new exp will come into play
             */
            fscanf(mob_fi, " %ld \n", &tmp);
            if (tmp >= 0) {
                GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
            } else {
                GET_EXP(mob) = -tmp;
            }
        }

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.position = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.default_pos = tmp;

        fscanf(mob_fi, " %ld \n", &tmp);
        if (tmp < 3) {
            mob->player.sex = tmp;
            mob->immune = 0;
            mob->M_immune = 0;
            mob->susc = 0;
        } else if (tmp < 6) {
            mob->player.sex = (tmp - 3);

            fscanf(mob_fi, " %ld ", &tmp);
            mob->immune = tmp;

            fscanf(mob_fi, " %ld ", &tmp);
            mob->M_immune = tmp;

            fscanf(mob_fi, " %ld ", &tmp);
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
            if (mob->player.sounds && *mob->player.sounds) {
                bc += strlen(mob->player.sounds);
            }
            mob->player.distant_snds = fread_string(mob_fi);
            if (mob->player.distant_snds && *mob->player.distant_snds) {
                bc += strlen(mob->player.distant_snds);
            }
        } else {
            mob->player.sounds = 0;
            mob->player.distant_snds = 0;
        }

        if (letter == 'B') {
            SET_BIT(mob->specials.act, ACT_HUGE);
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

    } else {
        /*
         * The old monsters are down below here
         */

        fscanf(mob_fi, "\n");

        fscanf(mob_fi, " %ld ", &tmp);
        mob->abilities.str = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->abilities.intel = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->abilities.wis = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->abilities.dex = tmp;

        fscanf(mob_fi, " %ld \n", &tmp);
        mob->abilities.con = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        fscanf(mob_fi, " %ld ", &tmp2);

        mob->points.max_hit = number(tmp, tmp2);
        mob->points.hit = mob->points.max_hit;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->points.armor = 10 * tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->points.mana = tmp;
        mob->points.max_mana = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->points.move = tmp;
        mob->points.max_move = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->points.gold = (int)((float) ((number(900, 1100) * tmp) / 1000));

        fscanf(mob_fi, " %ld \n", &tmp);
        if (tmp >= 0) {
            GET_EXP(mob) = (DetermineExp(mob, tmp) + mob->points.gold);
        } else {
            GET_EXP(mob) = -tmp;
        }

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.position = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->specials.default_pos = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->player.sex = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->player.class = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        GET_LEVEL(mob, WARRIOR_LEVEL_IND) = tmp;

        fscanf(mob_fi, " %ld ", &tmp);
        mob->player.time.birth = time(0);
        mob->player.time.played = 0;
        mob->player.time.logon = time(0);

        fscanf(mob_fi, " %ld ", &tmp);
        mob->player.weight = tmp;

        fscanf(mob_fi, " %ld \n", &tmp);
        mob->player.height = tmp;

        for (i = 0; i < 3; i++) {
            fscanf(mob_fi, " %ld ", &tmp);
            GET_COND(mob, i) = tmp;
        }
        fscanf(mob_fi, " \n ");

        for (i = 0; i < MAX_SAVES; i++) {
            fscanf(mob_fi, " %ld ", &tmp);
            mob->specials.apply_saving_throw[i] = tmp;
        }

        fscanf(mob_fi, " \n ");

        /*
         * Set the damage as some standard 1d4
         */
        mob->points.damroll = 0;
        mob->specials.damnodice = 1;
        mob->specials.damsizedice = 6;

        /*
         * Calculate THAC0 as a formular of Level
         */
        mob->points.hitroll = MAX(1, GET_LEVEL(mob, WARRIOR_LEVEL_IND) - 3);
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
    for (i = 0; i < top_of_scripts; i++) {
        if (script_data[i].vnum == mob_index[nr].vnum) {
            SET_BIT(mob->specials.act, ACT_SCRIPT);
            mob->script = i;
            break;
        }
    }

    /*
     * tell the spec_proc (if there is one) that we've been born
     * insert in list
     */

    mob->next = character_list;
    character_list = mob;

#ifdef LOW_GOLD
    if (mob->points.gold >= 50) {
        mob->points.gold /= 5;
    } else if (mob->points.gold < 10) {
        mob->points.gold = 0;
    }
#endif

    /*
     * Meh this creates such a lot of spam
     */
#if 0
    if (mob->points.gold > GET_LEVEL(mob, WARRIOR_LEVEL_IND)*1500) {
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
    mob_index[nr].number++;

#ifdef BYTE_COUNT
    fprintf(stderr, "Mobile [%d]: byte count: %d\n", mob_index[nr].vnum, bc);
#endif
    return (bc);
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
    for (i = 0; i < top_of_scripts; i++) {
        if (script_data[i].vnum == mob_index[nr].vnum) {
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
    mob_index[nr].number++;

#ifdef BYTE_COUNT
    fprintf(stderr, "Mobile [%d]: byte count: %d\n", mob_index[nr].vnum, bc);
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

/*
 * returns the real number of the monster with given virtual number
 */
int real_mobile(int virtual)
{
    int             bot,
                    top,
                    mid;

    bot = 0;
    top = top_of_sort_mobt;

    /*
     * perform binary search on mob-table
     */
    for (;;) {
        mid = (bot + top) / 2;

        if ((mob_index + mid)->vnum == virtual) {
            return (mid);
        }
        if (bot >= top) {
            /*
             * start unsorted search now
             */
            for (mid = top_of_sort_mobt; mid < top_of_mobt; mid++) {
                if ((mob_index + mid)->vnum == virtual) {
                    return (mid);
                }
            }
            return (-1);
        }
        if ((mob_index + mid)->vnum > virtual) {
            top = mid - 1;
        } else {
            bot = mid + 1;
        }
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
