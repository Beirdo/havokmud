/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
* Copyright 2007 Gavin Hurlbut
* All rights reserved
*
*/

#ifndef balanced_btree_h_
#define balanced_btree_h_

#include "environment.h"
#include "common_data.h"

/* CVS generated ID string (optional for h files) */
static char balanced_btree_h_ident[] _UNUSED_ = 
    "$Id$";

typedef int (*BalancedBTreeCompare_t)(void *left, void *right);

typedef enum
{
    BTREE_KEY_INT,
    BTREE_KEY_STRING,
    BTREE_KEY_PTHREAD,
    BTREE_KEY_POINTER
} BalancedBTreeKeyType_t;

struct _BalancedBTree_t;
struct _BalancedBTreeItem_t;

typedef struct _BalancedBTreeItem_t
{
    struct _BalancedBTreeItem_t *left;
    struct _BalancedBTreeItem_t *right;
    struct _BalancedBTreeItem_t *parent;
    void                        *item;
    void                        *key;
    struct _BalancedBTree_t     *btree;
    int                          visited;
} BalancedBTreeItem_t;

typedef struct _BalancedBTree_t
{
    BalancedBTreeKeyType_t       keyType;
    BalancedBTreeCompare_t       keyCompare;
    BalancedBTreeCompare_t       keyCompareAlt;
    BalancedBTreeItem_t         *root;
    pthread_mutex_t              mutex;
} BalancedBTree_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
BalancedBTree_t *BalancedBTreeCreate( BalancedBTreeKeyType_t type );
void BalancedBTreeDestroy( BalancedBTree_t *btree );
void BalancedBTreeLock( BalancedBTree_t *btree );
void BalancedBTreeUnlock( BalancedBTree_t *btree );
void BalancedBTreeAdd( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                       Locked_t locked, bool rebalance );
void BalancedBTreeRemove( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                       Locked_t locked, bool rebalance );
void *BalancedBTreeFind( BalancedBTree_t *btree, void *key,
                         Locked_t locked, bool alternate );
BalancedBTreeItem_t *BalancedBTreeFindGreatest( BalancedBTreeItem_t *root );
BalancedBTreeItem_t *BalancedBTreeFindLeast( BalancedBTreeItem_t *root );
void BalancedBTreeClearVisited( BalancedBTree_t *btree, Locked_t locked );
BalancedBTreeItem_t *BalancedBTreeFindLeastInRange( BalancedBTree_t *btree,
                                                    Locked_t locked,
                                                    void *begin, void *end );
BalancedBTreeItem_t *BalancedBTreeFindNextInRange( BalancedBTree_t *btree, 
                                                   BalancedBTreeItem_t *item, 
                                                   Locked_t locked, 
                                                   void *begin, void *end);
BalancedBTreeItem_t *BalancedBTreeFindNext( BalancedBTree_t *btree, 
                                            BalancedBTreeItem_t *item, 
                                            Locked_t locked);

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
