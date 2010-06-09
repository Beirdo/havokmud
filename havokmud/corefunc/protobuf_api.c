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
 * Handles protobuf API calls
 */

#include "config.h"
#include "environment.h"
#include <stdarg.h>
#include "interthread.h"
#include "protos.h"
#include "protobufs.h"
#include "havokrequest.pb-c.h"
#include "memory.h"

#define MAX_BUFFER_SIZE 256

char *pb_get_setting(char *name) 
{
    HavokRequest       *req;
    HavokRequest       *resp;
    char               *value;

    if( !name ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = HAVOK_REQUEST__REQ_TYPE__GET_SETTING;
    req->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( req->settings_data );
    req->settings_data->setting_name = memstrlink(name);

    resp = (HavokRequest *)protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return( NULL );
    }

    if( !resp->settings_data || !resp->settings_data->setting_value ) {
        value = NULL;
    } else {
        value = memstrlink( resp->settings_data->setting_value );
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );

    return( value );
}

void pb_set_setting( char *name, char *format, ... )
{
    HavokRequest   *req;
    char            buffer[MAX_BUFFER_SIZE];
    va_list         arguments;

    if( !name || !format ) {
        return;
    }

    va_start( arguments, format );
    vsnprintf( buffer, MAX_BUFFER_SIZE, format, arguments );
    va_end( arguments );

    req = protobufCreateRequest();
    if( !req ) {
        return;
    }

    req->request_type  = HAVOK_REQUEST__REQ_TYPE__SET_SETTING;
    req->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( req->settings_data );
    req->settings_data->setting_name  = memstrlink(name);
    req->settings_data->setting_value = memstrdup(buffer);

    protobufQueue( req, NULL, NULL, FALSE );
}

PlayerAccount_t *pb_load_account( char *email )
{
    HavokRequest       *req;
    HavokRequest       *resp;
    PlayerAccount_t    *acct;

    if( !email ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = HAVOK_REQUEST__REQ_TYPE__LOAD_ACCOUNT;
    req->account_data = CREATE(ReqAccountType);
    req_account_type__init( req->account_data );
    req->account_data->email = memstrlink(email);

    resp = (HavokRequest *)protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return( NULL );
    }

    if( !resp->account_data ) {
        acct = NULL;
    } else {
        acct = CREATE(PlayerAccount_t);
        if( acct ) {
            acct->email = memstrlink( resp->account_data->email );
            acct->id = resp->account_data->id;
            acct->pwd = memstrlink( resp->account_data->passwd );
            acct->ansi = resp->account_data->ansi;
            acct->confirmed = resp->account_data->confirmed;
            acct->confcode = memstrlink( resp->account_data->confcode );
        }
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );

    return( acct );
}

void pb_save_account( PlayerAccount_t *account )
{
    HavokRequest       *req;
    HavokRequest       *resp;

    if( !account ) {
        return;
    }

    req = protobufCreateRequest();
    if( !req ) {
        return;
    }

    req->request_type  = HAVOK_REQUEST__REQ_TYPE__SAVE_ACCOUNT;
    req->account_data = CREATE(ReqAccountType);
    req_account_type__init( req->account_data );
    req->account_data->email = memstrlink(account->email);
    req->account_data->id = account->id;
    req->account_data->passwd = memstrlink( account->pwd );
    req->account_data->ansi = account->ansi;
    req->account_data->confirmed = account->confirmed;
    req->account_data->confcode = memstrlink( account->confcode );
    req->account_data->has_id = TRUE;
    req->account_data->has_ansi = TRUE;
    req->account_data->has_confirmed = TRUE;

    resp = (HavokRequest *)protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return;
    }

    if( resp->account_data ) {
        account->id = resp->account_data->id;
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

