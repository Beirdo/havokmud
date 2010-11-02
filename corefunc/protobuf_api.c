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
#include "protobuf_api.h"
#include "havokrequest.pb-c.h"
#include "memory.h"

#define MAX_BUFFER_SIZE 256

void pb_fill_pc( PlayerPC_t *pc, ReqPCType *pb );
void pb_fill_pc_pb( PlayerPC_t *pc, ReqPCType *pb );

char *pb_get_setting(char *name) 
{
    HavokRequest       *req;
    HavokResponse      *resp;
    char               *value;

    if( !name ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = REQ_TYPE__GET_SETTING;
    req->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( req->settings_data );
    req->settings_data->setting_name = memstrlink(name);

    resp = protobufQueue( req, NULL, NULL, TRUE );
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

    req->request_type  = REQ_TYPE__SET_SETTING;
    req->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( req->settings_data );
    req->settings_data->setting_name  = memstrlink(name);
    req->settings_data->setting_value = memstrdup(buffer);

    protobufQueue( req, NULL, NULL, FALSE );
}

PlayerAccount_t *pb_load_account( char *email )
{
    HavokRequest       *req;
    HavokResponse      *resp;
    PlayerAccount_t    *acct;

    if( !email ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = REQ_TYPE__LOAD_ACCOUNT;
    req->account_data = CREATE(ReqAccountType);
    req_account_type__init( req->account_data );
    req->account_data->email = memstrlink(email);

    resp = protobufQueue( req, NULL, NULL, TRUE );
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
    HavokResponse      *resp;

    if( !account ) {
        return;
    }

    req = protobufCreateRequest();
    if( !req ) {
        return;
    }

    req->request_type  = REQ_TYPE__SAVE_ACCOUNT;
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

    resp = protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return;
    }

    if( resp->account_data ) {
        account->id = resp->account_data->id;
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );
}

PlayerPC_t *pb_get_pc_list( int account_id )
{
    HavokRequest       *req;
    HavokResponse      *resp;
    PlayerPC_t         *pcs;
    PlayerPC_t         *pc;
    int                 i;

    if( !account_id ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = REQ_TYPE__GET_PC_LIST;
    req->account_data = CREATE(ReqAccountType);
    req_account_type__init( req->account_data );
    req->account_data->id = account_id;

    resp = protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return( NULL );
    }

    pcs = NULL;
    if( resp->n_pc_data ) {
        pcs = CREATEN(PlayerPC_t, resp->n_pc_data + 1);

        if( pcs ) {
            for( i = 0; i < resp->n_pc_data; i++ ) {
                pb_fill_pc( &pcs[i], resp->pc_data[i] );
            }
        }
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );

    return( pcs );
}

void pb_fill_pc( PlayerPC_t *pc, ReqPCType *pb )
{
    pc->id              = pb->id;
    pc->account_id      = pb->account_id;
    pc->name            = memstrlink( pb->name );
    pc->complete        = pb->complete;
    pc->race_id         = pb->race_id;
    pc->align_moral     = pb->align_moral;
    pc->align_ethical   = pb->align_ethical;
    pc->strength        = pb->strength;
    pc->dexterity       = pb->dexterity;
    pc->constitution    = pb->constitution;
    pc->intelligence    = pb->intelligence;
    pc->wisdom          = pb->wisdom;
    pc->charisma        = pb->charisma;
    pc->social_class    = pb->social_class;
    pc->birth_order     = pb->birth_order;
    pc->siblings        = pb->siblings;
    pc->parents_married = pb->parents_married;
    pc->max_hit_points  = pb->max_hit_points;
    pc->hit_points      = pb->hit_points;
    pc->height          = pb->height;
    pc->weight          = pb->weight;
    pc->age             = pb->age;
    pc->hair_color      = pb->hair_color;
    pc->eye_color       = pb->eye_color;
    pc->hair_length     = memstrlink( pb->hair_length );
    pc->skin_tone       = pb->skin_tone;
    pc->experience      = pb->experience;
}

void pb_fill_pc_pb( PlayerPC_t *pc, ReqPCType *pb )
{
    pb->id              = pc->id;
    pb->account_id      = pc->account_id;
    pb->name            = memstrlink( pc->name );
    pb->complete        = pc->complete;
    pb->race_id         = pc->race_id;
    pb->align_moral     = pc->align_moral;
    pb->align_ethical   = pc->align_ethical;
    pb->strength        = pc->strength;
    pb->dexterity       = pc->dexterity;
    pb->constitution    = pc->constitution;
    pb->intelligence    = pc->intelligence;
    pb->wisdom          = pc->wisdom;
    pb->charisma        = pc->charisma;
    pb->social_class    = pc->social_class;
    pb->birth_order     = pc->birth_order;
    pb->siblings        = pc->siblings;
    pb->parents_married = pc->parents_married;
    pb->max_hit_points  = pc->max_hit_points;
    pb->hit_points      = pc->hit_points;
    pb->height          = pc->height;
    pb->weight          = pc->weight;
    pb->age             = pc->age;
    pb->hair_color      = pc->hair_color;
    pb->eye_color       = pc->eye_color;
    pb->hair_length     = memstrlink( pc->hair_length );
    pb->skin_tone       = pc->skin_tone;
    pb->experience      = pc->experience;
}

PlayerPC_t *pb_load_pc( int account_id, int pc_id )
{
    HavokRequest       *req;
    HavokResponse      *resp;
    PlayerPC_t         *pc;

    if( !account_id || !pc_id ) {
        return( NULL );
    }

    req = protobufCreateRequest();
    if( !req ) {
        return( NULL );
    }

    req->request_type  = REQ_TYPE__LOAD_PC;
    req->pc_data = CREATE(ReqPCType);
    req_pctype__init( req->pc_data );
    req->pc_data->id = pc_id;
    req->pc_data->account_id = account_id;

    resp = protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return( NULL );
    }

    pc = NULL;
    if( resp->n_pc_data ) {
        pc = CREATE(PlayerPC_t);

        pb_fill_pc( pc, resp->pc_data[0] );
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );

    return( pc );
}

void pb_save_pc( PlayerPC_t *pc )
{
    HavokRequest       *req;
    HavokResponse      *resp;

    if( !pc || !pc->account_id ) {
        return;
    }

    req = protobufCreateRequest();
    if( !req ) {
        return;
    }

    req->request_type  = REQ_TYPE__SAVE_PC;
    req->pc_data = CREATE(ReqPCType);
    req_pctype__init( req->pc_data );
    pb_fill_pc_pb( pc, req->pc_data );

    resp = protobufQueue( req, NULL, NULL, TRUE );
    if( !resp ) {
        return;
    }

    pc = NULL;
    if( resp->n_pc_data ) {
        pc->id = resp->pc_data[0]->id;
    }

    protobufDestroyMessage( (ProtobufCMessage *)resp );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

