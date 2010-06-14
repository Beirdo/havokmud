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
*
* Copyright 2010 Gavin Hurlbut
* All rights reserved
*/

#ifndef db_api_h_
#define db_api_h_

#include "environment.h"
#include "structs.h"
#include <stdarg.h>
#include "protobuf_api.h"

typedef struct {
    HavokResponse *(*get_setting)( char * );
    void (*set_setting)( char *, char * );
    HavokResponse *(*load_account)( char * );
    HavokResponse *(*save_account)( PlayerAccount_t * );
} DatabaseAPIFuncs_t;

extern DatabaseAPIFuncs_t db_api_funcs;

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

