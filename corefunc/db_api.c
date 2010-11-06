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
 *
 * Comments :
 *
 * Handles database API calls
 */

#include "config.h"
#include "environment.h"
#include <string.h>
#include "interthread.h"
#include "protos.h"
#include "db_api.h"
#include "logging.h"
#include "protobuf_api.h"

DatabaseAPIFuncs_t db_api_funcs;

void db_init( void )
{
    memset( &db_api_funcs, 0, sizeof(db_api_funcs) );
}

HavokResponse *db_get_setting( HavokRequest *req ) 
{
    if( db_api_funcs.get_setting ) {
        return( db_api_funcs.get_setting(req) );
    } else {
        LogPrintNoArg( LOG_CRIT, "Database API: no get_setting" );
        return( NULL );
    }
}

HavokResponse *db_set_setting( HavokRequest *req )
{
    if( !db_api_funcs.set_setting ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no set_setting" );
        return( NULL );
    }

    return( db_api_funcs.set_setting( req ) );
}

HavokResponse *db_load_account( HavokRequest *req )
{
    if( !db_api_funcs.load_account ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no load_account" );
        return( NULL );
    }

    return( db_api_funcs.load_account(req) );
}

HavokResponse *db_save_account( HavokRequest *req )
{
    if( !db_api_funcs.save_account ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no save_account" );
        return( NULL );
    }

    return( db_api_funcs.save_account( req ) );
}

HavokResponse *db_get_pc_list( HavokRequest *req )
{
    if( !db_api_funcs.get_pc_list ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no get_pc_list" );
        return( NULL );
    }

    return( db_api_funcs.get_pc_list( req ) );
}

HavokResponse *db_load_pc( HavokRequest *req )
{
    if( !db_api_funcs.load_pc ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no load_pc" );
        return( NULL );
    }

    return( db_api_funcs.load_pc( req ) );
}

HavokResponse *db_save_pc( HavokRequest *req )
{
    if( !db_api_funcs.save_pc ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no save_pc" );
        return( NULL );
    }

    return( db_api_funcs.save_pc( req ) );
}

HavokResponse *db_find_pc( HavokRequest *req )
{
    if( !db_api_funcs.find_pc ) {
        LogPrintNoArg( LOG_CRIT, "Database API: no find_pc" );
        return( NULL );
    }

    return( db_api_funcs.find_pc( req ) );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
