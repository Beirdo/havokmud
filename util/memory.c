/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2008, 2010 Gavin Hurlbut
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
* Copyright 2008, 2010 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*----------------------------------------------------------
*/

/* INCLUDE FILES */
#include "environment.h"
#include "memory.h"
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/select.h>
#include "logging.h"
#include "interthread.h"
#include "cJSON.h"

/**
 * @file
 * @brief Memory Allocation interface
 */

/* INTERNAL CONSTANT DEFINITIONS */
#define TIME_DEFER 10

/* INTERNAL TYPE DEFINITIONS */

/* INTERNAL MACRO DEFINITIONS */

/* INTERNAL FUNCTION PROTOTYPES */
void meminit( void );
MemoryBlock_t *memoryBlockGetNew( void );
void memoryBlockRelease( MemoryBlock_t *block );
MemoryFragment_t *memoryFragmentGetNew( void );
void memoryFragmentRelease( MemoryFragment_t *fragment );
MemoryFragment_t *memoryFragmentSplit(MemoryFragment_t *fragment, int size);
void memoryFragmentAdd( MemoryPool_t *pool, MemoryFragment_t *fragment, 
                        Locked_t locked );
void memoryFragmentRemove( MemoryPool_t *pool, MemoryFragment_t *fragment,
                           Locked_t locked );
MemoryFragment_t *memoryFragmentFindBySize(MemoryPool_t *pool, int size);
MemoryFragment_t *memoryFragmentFindByAddr(MemoryPool_t *pool, void *buffer);
bool memoryStringFree( char *buffer );


/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

static int memoryPageSize = -1;

static LinkedList_t     memoryBlockUsedList;
static LinkedList_t     memoryBlockFreeList;

static LinkedList_t     memoryFragmentUsedList;
static LinkedList_t     memoryFragmentFreeList;

static MemoryPool_t     fragmentFreePool;
static MemoryPool_t     fragmentAllocPool;
static MemoryPool_t     fragmentDeferPool;

static BalancedBTree_t  memoryStringTree;

static uint64           memAllocTrueBytes;
static uint64           memAllocBytes;
static int              memAllocBlocks;
static cJSON_Hooks      memcJSONHooks;


void meminit( void )
{
    memoryPageSize = getpagesize();
    memAllocBytes  = 0;
    memAllocTrueBytes = 0;
    memAllocBlocks = 0;

    LinkedListCreate( &memoryBlockUsedList );
    LinkedListCreate( &memoryBlockFreeList );
    LinkedListCreate( &memoryFragmentUsedList );
    LinkedListCreate( &memoryFragmentFreeList );

    BalancedBTreeCreate( &fragmentFreePool.sizeTree, BTREE_KEY_INT );
    BalancedBTreeCreate( &fragmentFreePool.addrTree, BTREE_KEY_POINTER );

    BalancedBTreeCreate( &fragmentAllocPool.sizeTree, BTREE_KEY_INT );
    BalancedBTreeCreate( &fragmentAllocPool.addrTree, BTREE_KEY_POINTER );

    BalancedBTreeCreate( &fragmentDeferPool.sizeTree, BTREE_KEY_INT );
    BalancedBTreeCreate( &fragmentDeferPool.addrTree, BTREE_KEY_POINTER );

    BalancedBTreeCreate( &memoryStringTree, BTREE_KEY_STRING );

    memcJSONHooks.malloc_fn = memalloc;
    memcJSONHooks.free_fn = memfree;
    cJSON_InitHooks( &memcJSONHooks );
}

void *memalloc( size_t size )
{
    int                 pageReq;
    int                 reqSize;
    void               *blk;
    MemoryBlock_t      *block;
    MemoryFragment_t   *fragment;

    if( size <= 0 ) {
        return( NULL );
    }

    if( memoryPageSize == -1 ) {
        meminit();
    }

    /* Search the free pool */
    fragment = memoryFragmentFindBySize(&fragmentFreePool, size);
    if( fragment ) {
        memAllocBytes += size;
        memset( fragment->start, 0, size );
        return( fragment->start );
    }

    /* Next check the deferred pool */
    fragment = memoryFragmentFindBySize(&fragmentDeferPool, size);
    if( fragment ) {
        memAllocBytes += size;
        memset( fragment->start, 0, size );
        return( fragment->start );
    }

    /* Allocate from the OS, rounded up to a page size */
    pageReq = (size / memoryPageSize) + (size % memoryPageSize ? 1 : 0);
    reqSize = pageReq * memoryPageSize;
    blk = malloc(reqSize);
    memAllocTrueBytes += reqSize;
    memAllocBlocks++;

    block            = memoryBlockGetNew();
    LinkedListCreate(&block->fragmentList);
    block->size      = reqSize;
    block->start     = blk;
    block->end       = (void *)((char *)blk + reqSize - 1);

    fragment = memoryFragmentGetNew();
    fragment->block = block;
    fragment->pool  = NULL;
    fragment->size  = reqSize;
    fragment->start = blk;
    fragment->end   = (void *)((char *)blk + reqSize -1 );
    fragment->timestamp = 0;
    LinkedListAdd( &block->fragmentList, (LinkedListItem_t *)fragment, UNLOCKED,
                   AT_TAIL );

    fragment = memoryFragmentSplit(fragment, size);
    memAllocBytes += size;
    memset( fragment->start, 0, size );
    return( fragment->start );
}

void *memcalloc( size_t nmemb, size_t size )
{
    return( memalloc( nmemb * size ) );
}

void *memrealloc( void *ptr, size_t size )
{
    MemoryFragment_t   *fragment;
    MemoryFragment_t   *newfrag;
    void               *newptr;

    if( ptr && !size ) {
        memfree( ptr );
        return;
    }

    if( ptr ) {
        fragment = memoryFragmentFindByAddr( &fragmentAllocPool, ptr );
        if( fragment->size == size ) {
            return( ptr );
        } else if( fragment->size > size ) {
            newfrag = memoryFragmentSplit(fragment, size);
            return( ptr );
        } else {
            newptr = memalloc( size );
            memcpy( newptr, ptr, fragment->size );
            memfree( ptr );
            return( newptr );
        }
    }

    newptr = memalloc( size );
    return( newptr );
}

MemoryBlock_t *memoryBlockGetNew( void )
{
    void           *blk;
    MemoryBlock_t  *block;
    int             count;
    int             i;

    LinkedListLock( &memoryBlockFreeList );
    if( LinkedListCount( &memoryBlockFreeList, LOCKED ) == 0 ) {
        /* Ran out, allocate more */
        count = memoryPageSize / sizeof(MemoryBlock_t);
        blk = malloc(memoryPageSize);
        memAllocTrueBytes += memoryPageSize;
        memAllocBlocks++;
        
        block = (MemoryBlock_t *)blk;
        for( i = 0; i < count; i++ ) {
            block->start = NULL;
            LinkedListAdd( &memoryBlockFreeList, (LinkedListItem_t *)block,
                           LOCKED, AT_TAIL );
            block++;
        }
    }
    block = (MemoryBlock_t *)memoryBlockFreeList.head;
    LinkedListRemove( &memoryBlockFreeList, (LinkedListItem_t *)block, LOCKED );
    LinkedListAdd( &memoryBlockUsedList, (LinkedListItem_t *)block, UNLOCKED,
                   AT_TAIL );
    LinkedListUnlock( &memoryBlockFreeList );

    return( block );
}

void memoryBlockRelease( MemoryBlock_t *block )
{
    if( !block ) {
        return;
    }

    LinkedListRemove( &memoryBlockUsedList, (LinkedListItem_t *)block, 
                      UNLOCKED );
    LinkedListAdd( &memoryBlockFreeList, (LinkedListItem_t *)block, UNLOCKED,
                   AT_TAIL );
    if( block->start ) {
        memAllocBlocks--;
        memAllocTrueBytes -= block->size;
        free( block->start );
        block->start = NULL;
    }
}



MemoryFragment_t *memoryFragmentGetNew( void )
{
    void               *frag;
    MemoryFragment_t   *fragment;
    LinkedListItem_t   *item;
    int                 count;
    int                 i;
    static int          offset = -1 * OFFSETOF(poolLinkage, MemoryFragment_t);

    LinkedListLock( &memoryFragmentFreeList );
    if( LinkedListCount( &memoryFragmentFreeList, LOCKED ) == 0 ) {
        /* Ran out, allocate more */
        count = memoryPageSize / sizeof(MemoryFragment_t);
        frag = malloc(memoryPageSize);
        memAllocTrueBytes += memoryPageSize;
        memAllocBlocks++;

        fragment = (MemoryFragment_t *)frag;
        for( i = 0; i < count; i++ ) {
            LinkedListAdd( &memoryFragmentFreeList, &fragment->poolLinkage,
                           LOCKED, AT_TAIL );
            fragment++;
        }
    }
    item = memoryFragmentFreeList.head;
    fragment = (MemoryFragment_t *)PTR_AT_OFFSET(offset, item);
    LinkedListRemove( &memoryFragmentFreeList, item, LOCKED );
    LinkedListAdd( &memoryFragmentUsedList, item, UNLOCKED, AT_TAIL );
    LinkedListUnlock( &memoryFragmentFreeList );

    return( fragment );
}

void memoryFragmentRelease( MemoryFragment_t *fragment )
{
    if( !fragment ) {
        return;
    }

    LinkedListRemove( &fragment->block->fragmentList, &fragment->blockLinkage, 
                      UNLOCKED );
    LinkedListRemove( &memoryFragmentUsedList, &fragment->poolLinkage, 
                      UNLOCKED );
    LinkedListAdd( &memoryFragmentFreeList, &fragment->poolLinkage, UNLOCKED, 
                   AT_TAIL );
}

MemoryFragment_t *memoryFragmentSplit(MemoryFragment_t *fragment, int size)
{
    MemoryFragment_t   *remainder;

    if( !fragment || size <= 0 ) {
        return( NULL );
    }

    if( fragment->size < size ) {
        return( NULL );
    }

    if( fragment->pool != NULL ) {
        memoryFragmentRemove(fragment->pool, fragment, UNLOCKED);
    }

    if( fragment->size != size ) {
        remainder = memoryFragmentGetNew();
        remainder->block = fragment->block;
        remainder->pool  = NULL;
        remainder->size  = fragment->size - size;
        remainder->start = ((char *)fragment->start + size);
        remainder->end   = fragment->end;
        remainder->timestamp = 0;
        LinkedListAdd( &fragment->block->fragmentList, 
                       (LinkedListItem_t *)remainder, UNLOCKED, AT_TAIL );

        fragment->end = (char *)fragment->end + size;
        fragment->size = size;

        memoryFragmentAdd(&fragmentFreePool, remainder, UNLOCKED);
    }

    memoryFragmentAdd(&fragmentAllocPool, fragment, UNLOCKED);

    return( fragment );
}

void memoryFragmentAdd( MemoryPool_t *pool, MemoryFragment_t *fragment, 
                        Locked_t locked )
{
    BalancedBTreeItem_t    *item;
    BalancedBTree_t        *tree;

    if( !pool || !fragment ) {
        return;
    }

    if( locked == UNLOCKED ) {
        BalancedBTreeLock( &pool->addrTree );
        BalancedBTreeLock( &pool->sizeTree );
    }

    item = BalancedBTreeFind( &pool->sizeTree, &fragment->size, LOCKED, FALSE );
    if( !item ) {
        tree = CREATER(BalancedBTree_t);
        BalancedBTreeCreate( tree, BTREE_KEY_POINTER );
        item = CREATER(BalancedBTreeItem_t);
        item->key = CREATER(int);
        *(int *)item->key = fragment->size;
        item->item = tree;
        BalancedBTreeAdd( &pool->sizeTree, item, LOCKED, TRUE );
    } else {
        tree = (BalancedBTree_t *)item->item;
    }

    fragment->sizeItem.key  = &fragment->start;
    fragment->sizeItem.item = fragment;
    BalancedBTreeAdd( tree, &fragment->sizeItem, UNLOCKED, TRUE );
    
    fragment->addrItem.key  = &fragment->start;
    fragment->addrItem.item = fragment;
    BalancedBTreeAdd( &pool->addrTree, &fragment->addrItem, LOCKED, TRUE );

    fragment->pool = pool;

    if( locked == UNLOCKED ) {
        BalancedBTreeUnlock( &pool->sizeTree );
        BalancedBTreeUnlock( &pool->addrTree );
    }
}


void memoryFragmentRemove( MemoryPool_t *pool, MemoryFragment_t *fragment,
                           Locked_t locked )
{
    BalancedBTreeItem_t    *item, *item2;
    BalancedBTree_t        *tree;

    if( !pool || !fragment || fragment->pool != pool ) {
        return;
    }

    if( locked == UNLOCKED ) {
        BalancedBTreeLock( &pool->addrTree );
        BalancedBTreeLock( &pool->sizeTree );
    }
    item = BalancedBTreeFind( &pool->sizeTree, &fragment->size, LOCKED, FALSE );
    if( item ) {
        tree = (BalancedBTree_t *)item->item;
        BalancedBTreeLock( tree );

        item2 = BalancedBTreeFind( tree, &fragment->start, LOCKED, FALSE );
        if( item2 ) {
            BalancedBTreeRemove( tree, item2, LOCKED, TRUE );
        }
        if( tree->root == NULL ) {
            BalancedBTreeRemove( &pool->sizeTree, item, LOCKED, TRUE );
            free( tree );
            free( item->key );
            free( item );
        } else {
            BalancedBTreeUnlock( tree );
        }
    }

    if( locked == UNLOCKED ) {
        BalancedBTreeUnlock( &pool->sizeTree );
    }
    
    item = BalancedBTreeFind( &pool->addrTree, &fragment->start, LOCKED, 
                              FALSE );
    if( item ) {
        BalancedBTreeRemove( &pool->addrTree, item, LOCKED, TRUE );
    }

    if( locked == UNLOCKED ) {
        BalancedBTreeUnlock( &pool->addrTree );
    }

    fragment->pool = NULL;
}

MemoryFragment_t *memoryFragmentFindBySize(MemoryPool_t *pool, int size)
{
    MemoryFragment_t       *fragment;
    BalancedBTreeItem_t    *item;
    BalancedBTree_t        *tree;

    if( !pool || size <= 0 ) {
        return( NULL );
    }

    BalancedBTreeLock( &pool->addrTree );
    BalancedBTreeLock( &pool->sizeTree );
    item = BalancedBTreeFindLeastInRange( &pool->sizeTree, LOCKED, &size, 
                                          NULL );
    if( !item ) {
        BalancedBTreeUnlock( &pool->sizeTree );
        BalancedBTreeUnlock( &pool->addrTree );
        return( NULL );
    }

    tree = (BalancedBTree_t *)item->item;
    BalancedBTreeLock( tree );

    item = BalancedBTreeFindLeast( tree->root );
    BalancedBTreeUnlock( tree );
    BalancedBTreeUnlock( &pool->sizeTree );
    BalancedBTreeUnlock( &pool->addrTree );

    if( !item ) {
        return( NULL );
    }

    fragment = (MemoryFragment_t *)item->item;
    fragment = memoryFragmentSplit(fragment, size);
    return( fragment );
}


MemoryFragment_t *memoryFragmentFindByAddr(MemoryPool_t *pool, void *buffer)
{
    MemoryFragment_t       *fragment;
    BalancedBTreeItem_t    *item;

    if( !pool || !buffer ) {
        return( NULL );
    }

    item = BalancedBTreeFind( &pool->addrTree, &buffer, UNLOCKED, FALSE );
    if( !item ) {
        return( NULL );
    }

    fragment = (MemoryFragment_t *)item->item;
    return( fragment );
}

void memfree( void *buffer )
{
    MemoryFragment_t   *fragment;
    struct timeval      tv;

    /* First check the string tree */
    if( memoryStringFree( buffer ) ) {
        return;
    }

    /* OK, let's check the allocated pool then */
    fragment = memoryFragmentFindByAddr( &fragmentAllocPool, buffer );
    if( !fragment ) {
        /* Whine and exit */
        return;
    }

    memAllocBytes -= fragment->size;

    memoryFragmentRemove( &fragmentAllocPool, fragment, UNLOCKED );

    /* Timestamp it */
    gettimeofday(&tv, NULL);
    fragment->timestamp = tv.tv_sec + TIME_DEFER;
    
    memoryFragmentAdd( &fragmentDeferPool, fragment, UNLOCKED );
}

char *memstrdup( char *orig )
{
    int         len;
    char       *out;

    /* Actually duplicates! */

    len = strlen(orig);
    out = CREATEN(char, len+1);
    if( out ) {
        strcpy( out, orig );
    }

    return( out );
}


char *memstrndup( char *orig, int maxlen )
{
    int         len;
    char       *out;

    /* Actually duplicates! */

    len = strlen(orig);
    if( len > maxlen ) {
        len = maxlen;
    }
    out = CREATEN(char, len+1);
    if( out ) {
        strncpy( out, orig, len );
        out[len] = '\0';
    }

    return( out );
}


char *memstrnlink( char *orig, int len )
{
    char                   *shortorig;
    char                   *retstring;

    /* used when linking to a common string is acceptable (consumer is NOT
     * changing either copy)
     */
    if( !orig || !len ) {
        return( NULL );
    }

    shortorig = CREATEN( char, len + 1 );
    strncpy( shortorig, orig, len );
    shortorig[len] = '\0';

    retstring = memstrlink( shortorig );

    memfree( shortorig );

    return( retstring );
}


char *memstrlink( char *orig )
{
    int                     len;
    BalancedBTreeItem_t    *item;
    MemoryString_t         *string;

    /* used when linking to a common string is acceptable (consumer is NOT
     * changing either copy)
     */
    if( !orig ) {
        return( NULL );
    }

    BalancedBTreeLock( &memoryStringTree );

    item = BalancedBTreeFind( &memoryStringTree, &orig, LOCKED, FALSE );
    if( item ) {
        string = (MemoryString_t *)item->item;
        string->count++;
    } else {
        len = strlen(orig);
        string = (MemoryString_t *)memalloc(sizeof(MemoryString_t) + len);
        strcpy( string->buf, orig );
        string->count = 1;
        string->addr = &string->buf[0];
        string->item.key  = &string->addr;
        string->item.item = string;
        BalancedBTreeAdd( &memoryStringTree, &string->item, LOCKED, TRUE );
    }
    BalancedBTreeUnlock( &memoryStringTree );

    return( string->buf );
}


bool memoryStringFree( char *buffer )
{
    BalancedBTreeItem_t    *item;
    MemoryString_t         *string;
    int                     remove;

    if( !buffer ) {
        return( FALSE );
    }

    remove = 0;
    BalancedBTreeLock( &memoryStringTree );

    item = BalancedBTreeFind( &memoryStringTree, &buffer, LOCKED, FALSE );
    if( item ) {
        string = (MemoryString_t *)item->item;
        string->count--;
        if( string->count <= 0 ) {
            BalancedBTreeRemove( &memoryStringTree, item, LOCKED, TRUE );
            remove = 1;
        }
    }
    BalancedBTreeUnlock( &memoryStringTree );

    if( item && remove ) {
        memfree( item );
    }

    return( item ? TRUE : FALSE );
}

void *MemoryCoalesceThread( void *arg )
{
    struct timeval          to;
    struct timeval          tv;
    BalancedBTree_t        *tree;
    BalancedBTreeItem_t    *item, *item2;
    MemoryFragment_t       *fragment, *fragment2;
    MemoryBlock_t          *block;
    LinkedListItem_t       *listItem, *next;

    pthread_mutex_lock( startupMutex );

    LogPrint( LOG_NOTICE, "Memory Page Size: 0x%08X (%d)", 
              memoryPageSize, memoryPageSize );
    LogPrint( LOG_NOTICE, "Coalescing memory every %.1gs", 
              (double)(TIME_DEFER)/2.0 );

    pthread_mutex_unlock( startupMutex );

    while( !GlobalAbort ) {
        to.tv_sec  = TIME_DEFER / 2;
        to.tv_usec = 500000 * (TIME_DEFER % 2);
        select(0, NULL, NULL, NULL, &to);

        if( GlobalAbort ) {
            continue;
        }

        BalancedBTreeLock( &fragmentDeferPool.addrTree );
        BalancedBTreeLock( &fragmentDeferPool.sizeTree );
        BalancedBTreeLock( &fragmentFreePool.addrTree );
        BalancedBTreeLock( &fragmentFreePool.sizeTree );

        gettimeofday( &tv, NULL );

        tree = &fragmentDeferPool.addrTree;

        BalancedBTreeClearVisited( tree, LOCKED );
        for( item = BalancedBTreeFindLeast( tree->root ); item;
             item = BalancedBTreeFindNext( tree, item, LOCKED ) ) {
            /* Looking for fragments to coalesce */
            fragment = (MemoryFragment_t *)item->item;
            if( fragment->timestamp > tv.tv_sec ) {
                continue;
            }

            block = fragment->block;
            LinkedListLock( &block->fragmentList );

            for( listItem = block->fragmentList.head; listItem; 
                 listItem = next ) {
                next = listItem->next;
                fragment2 = (MemoryFragment_t *)listItem;
                if( fragment == fragment2 ) {
                    continue;
                }

                if( ((char *)fragment2->end + 1) == (char *)fragment->start ) {
                    /* Previous fragment */
                    if( fragment2->pool != &fragmentDeferPool &&
                        fragment2->pool != &fragmentFreePool ) {
                        continue;
                    }

                    if( fragment2->pool == &fragmentDeferPool &&
                        fragment->timestamp > tv.tv_sec ) {
                        continue;
                    }

                    memoryFragmentRemove( fragment2->pool, fragment2, LOCKED );
                    memoryFragmentRemove( &fragmentDeferPool, fragment, 
                                          LOCKED );

                    fragment->size += fragment2->size;
                    fragment->start = fragment2->start;
                    fragment->timestamp = tv.tv_sec + TIME_DEFER;

                    /* In case more were pending before this somehow... */
                    next = listItem->prev;
                    if( !next ) {
                        next = block->fragmentList.head;
                    }

                    memoryFragmentRelease( fragment2 );
                    memoryFragmentAdd( &fragmentDeferPool, fragment, LOCKED );
                } else if( ((char *)fragment->end + 1) == 
                           (char *)fragment2->start ) {
                    /* Next fragment */
                    if( fragment2->pool != &fragmentDeferPool &&
                        fragment2->pool != &fragmentFreePool ) {
                        continue;
                    }

                    if( fragment2->pool == &fragmentDeferPool &&
                        fragment->timestamp > tv.tv_sec ) {
                        continue;
                    }

                    memoryFragmentRemove( &fragmentDeferPool, fragment, 
                                          LOCKED );
                    memoryFragmentRemove( fragment2->pool, fragment2, LOCKED );

                    fragment->size += fragment2->size;
                    fragment->end   = fragment2->end;
                    fragment->timestamp = tv.tv_sec + TIME_DEFER;

                    memoryFragmentRelease( fragment2 );
                    memoryFragmentAdd( &fragmentDeferPool, fragment, LOCKED );
                }
            }

            LinkedListUnlock( &block->fragmentList );
        }

        BalancedBTreeClearVisited( tree, LOCKED );
        item = BalancedBTreeFindLeast( tree->root );
        for( item = BalancedBTreeFindLeast( tree->root ); item; item = item2 ) {
            item2 = BalancedBTreeFindNext( tree, item, LOCKED );
            /* Looking for full blocks now */
            fragment = (MemoryFragment_t *)item->item;
            block = fragment->block;
            if( fragment->timestamp > tv.tv_sec ||
                fragment->size != fragment->block->size ) {
                continue;
            }

            /* This fragment is the whole block, and this block's deletion has
             * already been deferred.  Delete it.
             */
            memoryFragmentRemove( &fragmentDeferPool, fragment, LOCKED );
            memoryFragmentRelease( fragment );
            memoryBlockRelease( block );
        }

        BalancedBTreeUnlock( &fragmentFreePool.sizeTree );
        BalancedBTreeUnlock( &fragmentFreePool.addrTree );
        BalancedBTreeUnlock( &fragmentDeferPool.sizeTree );
        BalancedBTreeUnlock( &fragmentDeferPool.addrTree );
    }

    LogPrintNoArg( LOG_INFO, "Ending MemoryCoalesceThread" );
    return( NULL );
}

void memoryStats( int signum, void *ip )
{
    do_backtrace( signum, ip );

    LogPrint( LOG_INFO, "Memory allocated: %lld bytes (%lld bytes) in %d "
                        "blocks", 
              memAllocBytes, memAllocTrueBytes, memAllocBlocks );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
