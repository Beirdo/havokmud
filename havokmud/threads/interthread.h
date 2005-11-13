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
* $Id: queue.h 41 2004-12-12 01:15:22Z gjhurlbu $
*
* Copyright 2005 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*--------------------------------------------------------*/

#ifndef interthread_h_
#define interthread_h_

#include "environment.h"
#include <pthread.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"

/* CVS generated ID string (optional for h files) */
static char interthread_h_ident[] _UNUSED_ = 
    "$Id: queue.h 41 2004-12-12 01:15:22Z gjhurlbu $";

typedef struct {
    int port;
    int timeout_sec;
    int timeout_usec;
} connectThreadArgs_t;

typedef enum {
    STATE_INITIAL
} PlayerState_t;

struct _PlayerStruct_t;

typedef struct {
    LinkedListItem_t link;
    int fd;
    BufferObject_t *buffer;
    struct _PlayerStruct_t *player;
} ConnectionItem_t;

typedef struct _PlayerStruct_t
{
    LinkedListItem_t    link;
    ConnectionItem_t   *connection;
    BufferObject_t     *in_buffer;
    PlayerState_t       state;
    QueueObject_t      *handlingQ;
    char               *in_remain;
    int                 in_remain_len;
} PlayerStruct_t;

typedef enum
{
    CONN_NEW_CONNECT,
    CONN_INPUT_AVAIL
} ConnInputType_t;

typedef struct
{
    ConnInputType_t     type;
    PlayerStruct_t     *player;
} ConnInputItem_t;

typedef enum
{
    INPUT_INITIAL,
    INPUT_AVAIL
} InputStateType_t;

typedef struct
{
    InputStateType_t    type;
    PlayerStruct_t     *player;
    char               *line;
} InputStateItem_t;


/*
 * Externals used for interthread communication
 */
extern QueueObject_t *ConnectInputQ;
extern QueueObject_t *InputLoginQ;
extern QueueObject_t *InputPlayerQ;
extern QueueObject_t *InputImmortQ;

/*
 * Prototypes of the thread entry points
 */
void *ConnectionThread( void *arg );
void *InputThread( void *arg );
void *LoginThread( void *arg );


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
