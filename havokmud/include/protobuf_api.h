/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2010 Gavin Hurlbut
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
* Copyright 2010 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*
*--------------------------------------------------------*/

/**
 * @file
 * @brief Protobuf Support
 */

#ifndef protobuf_api_h_
#define protobuf_api_h_

#include "google/protobuf-c/protobuf-c.h"
#include "havokrequest.pb-c.h"
#include "havokresponse.pb-c.h"

#define PROTOBUF_API_VERSION 1

typedef void (*ProtobufResFunc_t)( HavokResponse *result, void *arg );

typedef struct {
    HavokRequest       *request;
    HavokResponse      *response;
    ProtobufResFunc_t   callback;
    void               *callbackArg;
    pthread_mutex_t    *mutex;
} ProtobufItem_t;

HavokResponse *protobufQueue( HavokRequest *request, 
                              ProtobufResFunc_t callback,
                              void *arg, bool block );
void protobufDestroyMessage( ProtobufCMessage *msg );
HavokRequest  *protobufCreateRequest( void );
HavokResponse *protobufCreateResponse( void );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
