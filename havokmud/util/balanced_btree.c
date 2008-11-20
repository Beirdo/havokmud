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
*/

/* INCLUDE FILES */
#define ___ARGH
#include "environment.h"
#include <pthread.h>
#include <stdio.h>
#include "balanced_btree.h"
#include <stdlib.h>
#include <string.h>
#include "logging.h"

/**
 * @file
 * @brief Balanced Binary Tree Data Structure
 */


/* INTERNAL CONSTANT DEFINITIONS */

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */
int KeyCompareInt( void *left, void *right );
int KeyCompareString( void *left, void *right );
int KeyComparePartialString( void *left, void *right );
int KeyComparePthread( void *left, void *right );
int KeyComparePointer( void *left, void *right );
BalancedBTreeItem_t *BalancedBTreeFindParent( BalancedBTree_t *btree,
                                              BalancedBTreeItem_t *item );
int BalancedBTreeWeight( BalancedBTreeItem_t *root );
void BalancedBTreeReplace( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                           BalancedBTreeItem_t *replace );
void BalancedBTreeRebalance( BalancedBTree_t *btree, 
                             BalancedBTreeItem_t *root );
void BalancedBTreePrune( BalancedBTreeItem_t *item );
void BalancedBTreeUnvisit( BalancedBTreeItem_t *item );

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

void BalancedBTreeLock( BalancedBTree_t *btree )
{
    if( btree == NULL )
    {
        return;
    }

    pthread_mutex_lock( &btree->mutex );
}


void BalancedBTreeUnlock( BalancedBTree_t *btree )
{
    if( btree == NULL )
    {
        return;
    }

    pthread_mutex_unlock( &btree->mutex );
}


BalancedBTree_t *BalancedBTreeCreate( BalancedBTree_t *btree,
                                      BalancedBTreeKeyType_t type )
{
    if( !btree ) {
        btree = (BalancedBTree_t *)malloc(sizeof(BalancedBTree_t));
        if( btree == NULL )
        {
            LogPrintNoArg( LOG_CRIT, "Couldn't create btree" );
            return( NULL );
        }
    }

    btree->root = NULL;
    btree->keyType = type;
    switch( type ) {
    case BTREE_KEY_INT:
        btree->keyCompare    = KeyCompareInt;
        btree->keyCompareAlt = KeyCompareInt;
        break;
    case BTREE_KEY_STRING:
        btree->keyCompare    = KeyCompareString;
        btree->keyCompareAlt = KeyComparePartialString;
        break;
    case BTREE_KEY_PTHREAD:
        btree->keyCompare    = KeyComparePthread;
        btree->keyCompareAlt = KeyComparePthread;
        break;
    case BTREE_KEY_POINTER:
        btree->keyCompare    = KeyComparePointer;
        btree->keyCompareAlt = KeyComparePointer;
        break;
    default:
        btree->keyCompare    = NULL;
        btree->keyCompareAlt = NULL;
        break;
    }

    pthread_mutex_init( &btree->mutex, NULL );

    return( btree );
}

void BalancedBTreeDestroy( BalancedBTree_t *btree )
{
    /* Assumes the btree is locked by the caller */
    BalancedBTreePrune( btree->root );

    pthread_mutex_unlock( &btree->mutex );
    pthread_mutex_destroy( &btree->mutex );

    free( btree );
}


void BalancedBTreeAdd( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                       Locked_t locked, bool rebalance )
{
    int         res;

    if( btree == NULL )
    {
        return;
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    if( item == NULL ) {
        if( rebalance ) {
            BalancedBTreeRebalance( btree, btree->root );
        }

        if( locked == UNLOCKED ) {
            BalancedBTreeLock( btree );
        }
        return;
    }

    item->btree = btree;

    item->left  = NULL;
    item->right = NULL;

    item->parent = BalancedBTreeFindParent( btree, item );
    if( item->parent == NULL) {
        btree->root = item;
    } else {
        res = btree->keyCompare( item->key, item->parent->key );
        if( res > 0 ) {
            /* item greater than parent */
            item->parent->right = item;
        } else if( res < 0 ) {
            /* item less than parent */
            item->parent->left  = item;
        } else {
            switch( btree->keyType ) {
            case BTREE_KEY_INT:
                LogPrint( LOG_CRIT, "Duplicate key (INT): %d", 
                                    *(int *)item->key );
                break;
            case BTREE_KEY_STRING:
                LogPrint( LOG_CRIT, "Duplicate key (STRING): %s", 
                                    *(char **)item->key );
                break;
            case BTREE_KEY_PTHREAD:
                LogPrint( LOG_CRIT, "Duplicate key (PTHREAD): %d", 
                                    *(int *)item->key );
                break;
            default:
                LogPrintNoArg( LOG_CRIT, "Duplicate key" );
                break;
            }
        }
    }

    if( rebalance ) {
        BalancedBTreeRebalance( btree, btree->root );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }
}


void BalancedBTreePrune( BalancedBTreeItem_t *item )
{
    if( !item ) {
        return;
    }

    /* Assumes a locked tree */
    if( item->left ) {
        BalancedBTreePrune( item->left );
    }

    if( item->right ) {
        BalancedBTreePrune( item->right );
    }

    BalancedBTreeRemove( item->btree, item, LOCKED, FALSE );
    free( item );
}


void BalancedBTreeRemove( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                          Locked_t locked, bool rebalance )
{
    BalancedBTreeItem_t    *replace;

    if( btree == NULL )
    {
        return;
    }

    if( item != NULL && item->btree != btree )
    {
        LogPrint( LOG_CRIT, "Item %p not on btree %p! (on %p)", (void *)item, 
                  (void *)btree, (void *)item->btree );
        return;
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    if( item == NULL ) {
        if( rebalance ) {
            BalancedBTreeRebalance( btree, btree->root );
        }

        if( locked == UNLOCKED ) {
            BalancedBTreeLock( btree );
        }
        return;
    }


    /* OK, it's on the btree, and we have it locked */
    if( BalancedBTreeWeight( item->left ) > 
        BalancedBTreeWeight( item->right ) ) {
        /* There's more stuff on the left subtree, relink taking
         * the greatest entry on the left subtree, moving it here
         */
        replace = BalancedBTreeFindGreatest( item->left );
    } else {
        /* Either more on the right subtree or they are balanced, relink
         * taking the least entry on the right subtree, moving it here
         */
        replace = BalancedBTreeFindLeast( item->right );
    }
    BalancedBTreeReplace( btree, item, replace );

    item->btree = NULL;

    if( rebalance ) {
        BalancedBTreeRebalance( btree, btree->root );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }
}


void *BalancedBTreeFind( BalancedBTree_t *btree, void *key,
                         Locked_t locked, bool alternate )
{
    BalancedBTreeItem_t    *item;
    bool                    found;
    int                     res;

    if( btree == NULL || key == NULL ) {
        return( NULL );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    for( found = FALSE, item = btree->root; item && !found; ) {
        if( alternate ) {
            res = btree->keyCompareAlt( key, item->key );
        } else {
            res = btree->keyCompare( key, item->key );
        }
        if( res == 0 ) {
            /* Found it */
            found = TRUE;
            continue;
        } else if ( res < 0 ) {
            item = item->left;
        } else {
            item = item->right;
        }
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }

    return( item );
}


int KeyCompareInt( void *left, void *right )
{
    int     res;
    int     l, r;

    if( !left || !right ) {
        return( 0 );
    }

    l = *(int *)left;
    r = *(int *)right;

    res = l - r;
    
    if( res > 1 ) {
        res = 1;
    } else if( res < -1 ) {
        res = -1;
    }

    return( res );
}

int KeyComparePthread( void *left, void *right )
{
    long int    res;
    pthread_t   l, r;

    if( !left || !right ) {
        return( 0 );
    }

    l = *(pthread_t *)left;
    r = *(pthread_t *)right;

    res = (long int)(l - r);
    
    if( res > 1 ) {
        res = 1;
    } else if( res < -1 ) {
        res = -1;
    }

    return( (int)res );
}

int KeyComparePointer( void *left, void *right )
{
    long long int   res;
    char          *l, *r;

    if( !left || !right ) {
        return( 0 );
    }

    l = *(char **)left;
    r = *(char **)right;

    res = (long long int)(l - r);
    
    if( res > 1 ) {
        res = 1;
    } else if( res < -1 ) {
        res = -1;
    }

    return( (int)res );
}

int KeyCompareString( void *left, void *right )
{
    int     res;
    char   *l, *r;

    if( !left || !right ) {
        return( 0 );
    }

    l = *(char **)left;
    r = *(char **)right;

    res = strcasecmp( l, r );
    
    if( res > 1 ) {
        res = 1;
    } else if( res < -1 ) {
        res = -1;
    }

    return( res );
}

int KeyComparePartialString( void *left, void *right )
{
    int     res;
    int     len;
    char   *l, *r;

    if( !left || !right ) {
        return( 0 );
    }

    l = *(char **)left;
    r = *(char **)right;
    len = strlen(l);

    res = strncasecmp( l, r, len );
    
    if( res > 1 ) {
        res = 1;
    } else if( res < -1 ) {
        res = -1;
    }

    return( res );
}

BalancedBTreeItem_t *BalancedBTreeFindParent( BalancedBTree_t *btree,
                                              BalancedBTreeItem_t *item )
{
    BalancedBTreeItem_t    *item2;
    BalancedBTreeItem_t    *parent;
    int                     res;

    if( btree == NULL || item == NULL ) {
        return( NULL );
    }

    parent = NULL;
    for( item2 = btree->root; item2;  ) {
        parent = item2;
        res = btree->keyCompare( item->key, item2->key );
        if ( res < 0 ) {
            item2 = item2->left;
        } else {
            item2 = item2->right;
        }
    }

    return( parent );
}

int BalancedBTreeWeight( BalancedBTreeItem_t *root )
{
    int     res;

    if ( root == NULL ) {
        return( 0 );
    }

    res = 1;
    
    if( root->left != NULL ) {
        res += BalancedBTreeWeight( root->left );
    }

    if( root->right != NULL ) {
        res += BalancedBTreeWeight( root->right );
    }

    return( res );
}

BalancedBTreeItem_t *BalancedBTreeFindGreatest( BalancedBTreeItem_t *root )
{
    BalancedBTreeItem_t *prev;

    prev = NULL;
    while( root ) {
        prev = root;
        root = root->right;
    }
    return( prev );
}
 
BalancedBTreeItem_t *BalancedBTreeFindLeast( BalancedBTreeItem_t *root )
{
    BalancedBTreeItem_t *prev;

    prev = NULL;
    while( root ) {
        prev = root;
        root = root->left;
    }
    return( prev );
}

void BalancedBTreeReplace( BalancedBTree_t *btree, BalancedBTreeItem_t *item, 
                           BalancedBTreeItem_t *replace )
{
    /* This expects that the replace is a node with a maximum of one subtree.
     * As we searched for greatest or least, this is a valid assumption.  The
     * greatest may have a left subtree, the least a right subtree.
     */

    if( replace == item ) {
        return;
    }

    if( replace ) {
        /* Relink the replacement's parent to any subtree */
        if( replace->parent != item ) {
            if( replace->parent->left == replace ) {
                /* We must be the least */
                replace->parent->left = replace->right;
                if( replace->right ) {
                    replace->right->parent = replace->parent;
                }
            } else if( replace->parent->right == replace ) {
                /* We must be the greatest */
                replace->parent->right = replace->left;
                if( replace->left ) {
                    replace->left->parent = replace->parent;
                }
            }
            replace->right = item->right;
            if( replace->right ) {
                replace->right->parent = replace;
            }
            replace->left  = item->left;
            if( replace->left ) {
                replace->left->parent = replace;
            }
        } else {
            if( item->right == replace ) {
                /* We must be the least */
                replace->left = item->left;
                if( replace->left ) {
                    replace->left->parent = replace;
                }
            } else if( item->left == replace ) {
                /* We must be the greatest */
                replace->right = item->right;
                if( replace->right ) {
                    replace->right->parent = replace;
                }
            }
        }

        replace->parent = item->parent;

        if( replace->parent ) {
            if( replace->parent->left == item ) {
                replace->parent->left = replace;
            } else {
                replace->parent->right = replace;
            }
        } else {
            btree->root = replace;
        }
    } else {
        /* No children, relink our parent to NULL where we were */
        if( !item->parent ) {
            /* Last item on the tree */
            btree->root = NULL;
        } else {
            if( item->parent->left == item ) {
                item->parent->left = NULL;
            } else {
                item->parent->right = NULL;
            }
        }
    }

    /* Now the original is not linked to by anything, NULL its links */
    item->left   = NULL;
    item->right  = NULL;
    item->parent = NULL;
}
 
void BalancedBTreeRebalance( BalancedBTree_t *btree, 
                             BalancedBTreeItem_t *root )
{
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *left;
    BalancedBTreeItem_t    *right;
    BalancedBTreeItem_t    *parent;
    int                     res;

    if( root == NULL ) {
        return;
    }

    for( res = BalancedBTreeWeight( root->left ) - 
               BalancedBTreeWeight( root->right ); res < -1 || res > 1;
         res = BalancedBTreeWeight( root->left ) - 
               BalancedBTreeWeight( root->right ) ) {

        /* Too much stuff on either left or right subtree */
        if( res > 1 ) {
            /* Too much on left, need to shift to the right */
            item = BalancedBTreeFindGreatest( root->left );

            left   = item->left;
            right  = item->right;   /* This is NULL! */
            parent = item->parent;

            item->parent = root->parent;
            item->right  = root;
            if( root->left != item ) {
                item->left = root->left;
                if( item->left != NULL ) {
                    item->left->parent = item;
                }
            }

            root->parent = item;
            root->left   = NULL;

            if( parent != root ) {
                parent->right = left;
                if( left != NULL ) {
                    left->parent = parent;
                }
            }
        } else {
            /* Too much on right, need to shift to the left */
            item = BalancedBTreeFindLeast( root->right );

            left   = item->left;    /* This is NULL! */
            right  = item->right;
            parent = item->parent;

            item->parent = root->parent;
            item->left   = root;
            if( root->right != item ) {
                item->right  = root->right;
                if( item->right != NULL ) {
                    item->right->parent = item;
                }
            }

            root->parent = item;
            root->right  = NULL;

            if( parent != root ) {
                parent->left = right;
                if( right != NULL ) {
                    right->parent = parent;
                }
            }
        }

        if( item->parent != NULL ) {
            if( item->parent->left == root ) {
                item->parent->left = item;
            } else {
                item->parent->right = item;
            }
        } else {
            btree->root = item;
        }
        root = item;
    }

    BalancedBTreeRebalance( btree, root->left );
    BalancedBTreeRebalance( btree, root->right );
}

void BalancedBTreeClearVisited( BalancedBTree_t *btree, Locked_t locked )
{
    if( btree == NULL ) {
        return;
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    BalancedBTreeUnvisit( btree->root );

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }
}

void BalancedBTreeUnvisit( BalancedBTreeItem_t *item )
{
    if( !item ) {
        return;
    }

    item->visited = 0;
    BalancedBTreeUnvisit( item->left );
    BalancedBTreeUnvisit( item->right );
}

BalancedBTreeItem_t *BalancedBTreeFindLeastInRange( BalancedBTree_t *btree,
                                                    Locked_t locked,
                                                    void *begin, void *end )
{
    BalancedBTreeItem_t    *find;
    BalancedBTreeItem_t    *item;
    BalancedBTreeItem_t    *retitem;
    int                     res;

    if( btree == NULL ) {
        return( NULL );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    retitem = NULL;

    BalancedBTreeClearVisited( btree, LOCKED );

    find = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
    find->key = begin;

    item = BalancedBTreeFindParent( btree, find );
    free( find );

    if( !item ) {
        item = btree->root;
    }

    /* Check the item */
    res = btree->keyCompare( item->key, begin );
    if( res == 0 ) {
        /* Found exactly */
        retitem = item;
    } else if( res > 0 ) {
        /* item is greater, do a range check against end */
        res = btree->keyCompare( item->key, end );
        if( res <= 0 ) {
            /* item <= end */
            retitem = item;
        }
    } else {
        /* We have a problem..   gotta search the tree iteratively until we
         * find one in range (or until > range)
         */
        retitem = BalancedBTreeFindNextInRange(btree, item, LOCKED, begin, end);
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }

    return( retitem );
}

BalancedBTreeItem_t *BalancedBTreeFindNextInRange( BalancedBTree_t *btree, 
                                                   BalancedBTreeItem_t *item, 
                                                   Locked_t locked, 
                                                   void *begin, void *end)
{
    BalancedBTreeItem_t    *retitem;
    int                     res;
    int                     found;

    if( btree == NULL || item == NULL ) {
        return( NULL );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    retitem = NULL;
    found = 0;

    while( item && !found ) {
        item->visited++;

        if( item->right && !item->right->visited ) {
            item = BalancedBTreeFindLeast( item->right );
        } else {
            item = item->parent;
        }

        if( !item->visited ) {
            res = btree->keyCompare( item->key, begin );
            if( res >= 0 ) {
                /* item->key >= begin */
                res = btree->keyCompare( item->key, end );
                if( res <= 0 ) {
                    /* item->key <= end */
                    retitem = item;
                } else {
                    /* item->key > end */
                    retitem = NULL;
                }
                found = 1;
            }
        }
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }

    return( retitem );
}

BalancedBTreeItem_t *BalancedBTreeFindNext( BalancedBTree_t *btree, 
                                            BalancedBTreeItem_t *item, 
                                            Locked_t locked)
{
    BalancedBTreeItem_t    *retitem;
    int                     found;

    if( btree == NULL || item == NULL ) {
        return( NULL );
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeLock( btree );
    }

    retitem = NULL;
    found = 0;

    while( item && !found ) {
        item->visited++;

        if( item->right && !item->right->visited ) {
            item = BalancedBTreeFindLeast( item->right );
        } else {
            item = item->parent;
        }

        if( !item->visited ) {
            retitem = item;
            found = 1;
        }
    }

    if( locked == UNLOCKED )
    {
        BalancedBTreeUnlock( btree );
    }

    return( retitem );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
