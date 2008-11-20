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
*
*--------------------------------------------------------*/

/**
 * @file
 * @brief Memory Allocation interface
 */

#ifndef memory_h_
#define memory_h_

#include "environment.h"
#include <pthread.h>
#include "linked_list.h"
#include "balanced_btree.h"
#include <time.h>

/* CVS generated ID string (optional for h files) */
static char memory_h_ident[] _UNUSED_ = 
    "$Id$";

#define CREATE(type)	(type *)memalloc(sizeof(type))

struct _MemoryFragment_t;
struct _MemoryBlock_t;
struct _MemoryPool_t;

typedef struct _MemoryFragment_t {
	LinkedListItem_t		blockLinkage;
	LinkedListItem_t		poolLinkage;
	BalancedBTreeItem_t	    sizeItem;
    BalancedBTreeItem_t     addrItem;
	struct _MemoryBlock_t  *block;
	struct _MemoryPool_t   *pool;
	int						size;
	void                   *start;
	void				   *end;
	time_t					timestamp;
} MemoryFragment_t;

typedef struct _MemoryBlock_t {
	LinkedListItem_t		linkage;
	LinkedList_t		    fragmentList;
	int						size;
	void				   *start;
	void				   *end;
} MemoryBlock_t;

typedef struct _MemoryPool_t {
	BalancedBTree_t 		sizeTree;
	BalancedBTree_t			addrTree;
} MemoryPool_t;

typedef struct _MemoryString_t {
	int					count;
	BalancedBTreeItem_t	item;
	char				buf[1];	/* NOTE: +strlen at alloc, must be last item */
} MemoryString_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
void *memalloc( int size );
void memfree( void *buffer );
char *memstrdup( char *orig );
char *memstrlink( char *orig );
void *MemoryCoalesceThread( void *arg );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
