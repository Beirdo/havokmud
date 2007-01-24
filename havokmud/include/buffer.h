/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005 Gavin Hurlbut
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
* Copyright 2005 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*
*--------------------------------------------------------*/
#ifndef buffer_h_
#define buffer_h_

#include "environment.h"
#include <pthread.h>

/* CVS generated ID string (optional for h files) */
static char buffer_h_ident[] _UNUSED_ = 
    "$Id$";


typedef struct 
{ 
    uint32 size;
    uint32 head;
    uint32 tail;
    char *data;
    pthread_mutex_t *mutex;
    bool full;
    pthread_cond_t *cNotFull;
    bool empty;
    pthread_cond_t *cNotEmpty;
} BufferObject_t;


#ifdef __cplusplus
extern "C" {
#endif

/* Function Prototypes */
BufferObject_t * BufferCreate( uint32 size );
void BufferDestroy( BufferObject_t *buffer );

void BufferLock( BufferObject_t *buffer ); 
void BufferUnlock( BufferObject_t *buffer ); 
char *BufferGetWrite( BufferObject_t *buffer ); 
char *BufferGetRead( BufferObject_t *buffer ); 
void BufferClear( BufferObject_t *buffer, bool lock );
uint32 BufferAvailWrite( BufferObject_t *buffer, bool lock );
uint32 BufferAvailRead( BufferObject_t *buffer, bool lock );
void BufferWroteBytes( BufferObject_t *buffer, uint32 count );
void BufferReadBytes( BufferObject_t *buffer, uint32 count );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
