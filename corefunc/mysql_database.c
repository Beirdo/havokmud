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
 * Handles MySQL database connections
 */

#include "config.h"
#include "environment.h"

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdarg.h>

#include "oldstructs.h"
#include "oldutils.h"
#include "oldexterns.h"
#include "logging.h"
#include "interthread.h"
#include "protos.h"
#include "memory.h"
#include "db_api.h"
#include "protobuf_api.h"

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @file
 * @brief Contains the API for the system to use to access the MySQL database.
 */

HavokResponse *db_mysql_get_setting(char *name);
void db_mysql_set_setting( char *name, char *value );
HavokResponse *db_mysql_load_account( char *email );
HavokResponse *db_mysql_save_account( PlayerAccount_t *account );
HavokResponse *db_mysql_get_pc_list( int account_id );

void chain_set_setting( MYSQL_RES *res, QueryItem_t *item );
void chain_save_account( MYSQL_RES *res, QueryItem_t *item );

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );
void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid );
void result_insert_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                            long insertid );
void result_get_pc_list( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid );

QueryTable_t QueryTable[] = {
    /* 0 */
    { "SELECT `value` FROM `settings` WHERE `name` = ? LIMIT 1", NULL, NULL,
      FALSE },
    /* 1 */
    { "SELECT `value` FROM `settings` WHERE `name` = ? LIMIT 1",
      chain_set_setting, NULL, FALSE },
    /* 2 */
    { "UPDATE `settings` SET `value` = ? WHERE `name` = ?", NULL, NULL, FALSE },
    /* 3 */
    { "INSERT INTO `settings` (`name`, `value`) VALUES (?, ?)", NULL, NULL,
      FALSE },
    /* 4 */
    { "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, `confcode` "
      "FROM `accounts` WHERE `email` = ?", NULL, NULL, FALSE },
    /* 5 */
    { "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, `confcode` "
      "FROM `accounts` WHERE `id` = ?", chain_save_account, NULL, FALSE },
    /* 6 */
    { "UPDATE `accounts` SET `email` = ?, `passwd` = ?, `ansi` = ?, "
      "`confirmed` = ?, `confcode` = ? WHERE `id` = ?", NULL, NULL, FALSE },
    /* 7 */
    { "INSERT INTO `accounts` (`id`, `email`, `passwd`, `ansi`, `confirmed`, "
      "`confcode`) VALUES (?, ?, ?, ?, ?, ?)",
      NULL, NULL, FALSE },
    /* 8 */
    { "SELECT `id`, `account_id`, `name`, `complete`, `race_id`, "
      "`align_moral`, `align_ethical`, `strength`, `dexterity`, "
      "`constitution`, `intelligence`, `wisdom`, `charisma`, `social_class`, "
      "`birth_order`, `siblings`, `parents_married`, `max_hit_points`, "
      "`hit_points`, `height`, `weight`, `age`, `hair_color`, `eye_color`, "
      "`hair_length`, `skin_tone`, `experience` FROM `pcs` "
      "WHERE `account_id` = ? ORDER BY `id`", NULL, NULL, FALSE },
    /* END */
    { NULL, NULL, NULL, FALSE }
};

void db_mysql_init( void )
{
    db_api_funcs.get_setting  = db_mysql_get_setting;
    db_api_funcs.set_setting  = db_mysql_set_setting;
    db_api_funcs.load_account = db_mysql_load_account;
    db_api_funcs.save_account = db_mysql_save_account;
    db_api_funcs.get_pc_list  = db_mysql_get_pc_list;
}

HavokResponse *db_mysql_get_setting(char *name)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;
    char               *result;

    if( !name ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_string( &data[0], name, MYSQL_TYPE_VAR_STRING );
    result = NULL;

    db_queue_query( 0, QueryTable, data, 1, result_get_setting, (void *)&result,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    resp->request_type = REQ_TYPE__GET_SETTING;
    resp->settings_data = CREATE(ReqSettingsType);
    req_settings_type__init( resp->settings_data );
    resp->settings_data->setting_name = memstrlink( name );
    resp->settings_data->setting_value = result;

    return( resp );
}
        
void db_mysql_set_setting( char *name, char *value )
{
    MYSQL_BIND     *data;
    
    if( !name || !value ) {
        return;
    }
    
    data = CREATEN(MYSQL_BIND, 2);

    bind_string( &data[0], name, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[1], value, MYSQL_TYPE_VAR_STRING );
    
    db_queue_query( 1, QueryTable, data, 2, NULL, NULL, NULL);
}
    
HavokResponse *db_mysql_load_account( char *email )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    PlayerAccount_t    *result;
    HavokResponse      *resp;

    if( !email ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_string( &data[0], email, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 4, QueryTable, data, 1, result_load_account, 
                    (void *)&result, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    if( !result ) {
        return( NULL );
    }

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    resp->request_type = REQ_TYPE__LOAD_ACCOUNT;
    resp->account_data = CREATE(ReqAccountType);
    req_account_type__init( resp->account_data );
    resp->account_data->email = memstrlink( result->email );
    resp->account_data->id = result->id;
    resp->account_data->passwd = memstrlink( result->pwd );
    resp->account_data->ansi = result->ansi;
    resp->account_data->confirmed = result->confirmed;
    resp->account_data->confcode = memstrlink( result->confcode );
    resp->account_data->has_id = TRUE;
    resp->account_data->has_ansi = TRUE;
    resp->account_data->has_confirmed = TRUE;

    memfree( result->email );
    memfree( result->pwd );
    memfree( result->confcode );
    memfree( result );

    return( resp );
}

HavokResponse *db_mysql_save_account( PlayerAccount_t *account )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    HavokResponse      *resp;

    if( !account ) {
        return;
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 7);

    bind_numeric( &data[0], account->id, MYSQL_TYPE_LONG );
    bind_string( &data[1], account->email, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[2], account->pwd, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[3], (account->ansi ? 1 : 0), MYSQL_TYPE_TINY );
    bind_numeric( &data[4], (account->confirmed ? 1 : 0), MYSQL_TYPE_TINY );
    bind_string( &data[5], (account->confcode ? account->confcode : ""), 
                 MYSQL_TYPE_VAR_STRING );
    bind_null_blob( &data[6], account );

    db_queue_query( 5, QueryTable, data, 7, NULL, NULL, mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    resp->request_type = REQ_TYPE__SAVE_ACCOUNT;
    resp->account_data = CREATE(ReqAccountType);
    req_account_type__init( resp->account_data );
    resp->account_data->email = memstrlink( account->email );
    resp->account_data->id = account->id;
    resp->account_data->passwd = memstrlink( account->pwd );
    resp->account_data->ansi = account->ansi;
    resp->account_data->confirmed = account->confirmed;
    resp->account_data->confcode = memstrlink( account->confcode );
    resp->account_data->has_id = TRUE;
    resp->account_data->has_ansi = TRUE;
    resp->account_data->has_confirmed = TRUE;

    memfree( account->email );
    memfree( account->pwd );
    memfree( account->confcode );
    memfree( account );

    return( resp );
}

HavokResponse *db_mysql_get_pc_list( int account_id )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    PlayerPC_t         *result;
    HavokResponse      *resp;
    int                 i;

    if( !account_id ) {
        return( NULL );
    }

    mutex = CREATE(pthread_mutex_t);
    thread_mutex_init( mutex );

    data = CREATEN(MYSQL_BIND, 1);

    bind_numeric( &data[0], account_id, MYSQL_TYPE_LONG );

    db_queue_query( 8, QueryTable, data, 1, result_get_pc_list, 
                    (void *)&result, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    memfree( mutex );

    if( !result ) {
        return( NULL );
    }

    resp = protobufCreateResponse();
    if( !resp ) {
        return( NULL );
    }

    for( i = 0; result[i].account_id == account_id; i++ );

    resp->request_type = REQ_TYPE__GET_PC_LIST;
    resp->n_pc_data = i;
    resp->pc_data = CREATEN(ReqPCType *, i);

    for( i = 0; i < resp->n_pc_data; i++ ) {
        resp->pc_data[i] = CREATE(ReqPCType);
        req_pctype__init( resp->pc_data[i] );

        resp->pc_data[i]->id = result[i].id;
        resp->pc_data[i]->account_id = result[i].account_id;
        resp->pc_data[i]->name = memstrlink( result[i].name );
        resp->pc_data[i]->complete = result[i].complete;
        resp->pc_data[i]->race_id = result[i].race_id;
        resp->pc_data[i]->align_moral = result[i].align_moral;
        resp->pc_data[i]->align_ethical = result[i].align_ethical;
        resp->pc_data[i]->strength = result[i].strength;
        resp->pc_data[i]->dexterity = result[i].dexterity;
        resp->pc_data[i]->constitution = result[i].constitution;
        resp->pc_data[i]->intelligence = result[i].intelligence;
        resp->pc_data[i]->wisdom = result[i].wisdom;
        resp->pc_data[i]->charisma = result[i].charisma;
        resp->pc_data[i]->social_class = result[i].social_class;
        resp->pc_data[i]->birth_order = result[i].birth_order;
        resp->pc_data[i]->siblings = result[i].siblings;
        resp->pc_data[i]->parents_married = result[i].parents_married;
        resp->pc_data[i]->max_hit_points = result[i].max_hit_points;
        resp->pc_data[i]->hit_points = result[i].hit_points;
        resp->pc_data[i]->height = result[i].height;
        resp->pc_data[i]->weight = result[i].weight;
        resp->pc_data[i]->age = result[i].age;
        resp->pc_data[i]->hair_color = result[i].hair_color;
        resp->pc_data[i]->eye_color = result[i].eye_color;
        resp->pc_data[i]->hair_length = memstrlink( result[i].hair_length );
        resp->pc_data[i]->skin_tone = result[i].skin_tone;
        resp->pc_data[i]->experience = result[i].experience;
        resp->pc_data[i]->has_id = TRUE;
        resp->pc_data[i]->has_account_id = TRUE;
        resp->pc_data[i]->has_complete = TRUE;
        resp->pc_data[i]->has_race_id = TRUE;
        resp->pc_data[i]->has_align_moral = TRUE;
        resp->pc_data[i]->has_align_ethical = TRUE;
        resp->pc_data[i]->has_strength = TRUE;
        resp->pc_data[i]->has_dexterity = TRUE;
        resp->pc_data[i]->has_constitution = TRUE;
        resp->pc_data[i]->has_intelligence = TRUE;
        resp->pc_data[i]->has_wisdom = TRUE;
        resp->pc_data[i]->has_charisma = TRUE;
        resp->pc_data[i]->has_social_class = TRUE;
        resp->pc_data[i]->has_birth_order = TRUE;
        resp->pc_data[i]->has_siblings = TRUE;
        resp->pc_data[i]->has_parents_married = TRUE;
        resp->pc_data[i]->has_max_hit_points = TRUE;
        resp->pc_data[i]->has_hit_points = TRUE;
        resp->pc_data[i]->has_height = TRUE;
        resp->pc_data[i]->has_weight = TRUE;
        resp->pc_data[i]->has_age = TRUE;
        resp->pc_data[i]->has_hair_color = TRUE;
        resp->pc_data[i]->has_eye_color = TRUE;
        resp->pc_data[i]->has_skin_tone = TRUE;
        resp->pc_data[i]->has_experience = TRUE;

        memfree( result[i].name );
        memfree( result[i].hair_length );
    }
    memfree( result );

    return( resp );
}

/*
 * Query chaining functions
 */

void chain_set_setting( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[1];
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }
    
    if( count ) {
        /* update */
        /* Swap the arguments around */
        memcpy( temp,     &data[0], sizeof(MYSQL_BIND) );
        memcpy( &data[0], &data[1], sizeof(MYSQL_BIND) );
        memcpy( &data[1], temp,     sizeof(MYSQL_BIND) );
        db_queue_query( 2, QueryTable, data, 2, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 3, QueryTable, data, 2, NULL, NULL, NULL );
    }
}

void chain_save_account( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[1];
    PlayerAccount_t *account;
    
    data = item->queryData;
    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    account = (PlayerAccount_t *)data[6].buffer;
    
    if( count ) {
        /* update */
        /* swap argument order */
        memcpy(  temp,     &data[0], sizeof(MYSQL_BIND) );
        memmove( &data[0], &data[1], sizeof(MYSQL_BIND) * 5 );
        memcpy(  &data[5], temp,     sizeof(MYSQL_BIND) );
        db_queue_query( 6, QueryTable, data, 6, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 7, QueryTable, data, 6, result_insert_account, account,
                        NULL );
    }
}



/*
 * Query result callbacks
 */

void result_get_setting( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid )
{
    char          **resp;
    char           *value;
    int             count;
    MYSQL_ROW       row;

    resp = (char **)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    value = memstrlink(row[0]);
    *resp = value;
}

void result_load_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                          long insertid )
{
    PlayerAccount_t   **resp;
    PlayerAccount_t    *account;
    int                 count;
    MYSQL_ROW           row;

    resp = (PlayerAccount_t **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }
    row = mysql_fetch_row(res);

    account = CREATE(PlayerAccount_t);
    account->id        = atoi(row[0]);
    account->email     = memstrdup(row[1]);
    account->pwd       = memstrdup(row[2]);
    account->ansi      = (atoi(row[3]) ? TRUE : FALSE);
    account->confirmed = (atoi(row[4]) ? TRUE : FALSE);
    account->confcode  = memstrdup(row[5]);
    *resp = account;
}


void result_insert_account( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                            long insertid )
{
    PlayerAccount_t    *account;

    if( !arg ) {
        return;
    }

    account = (PlayerAccount_t *)arg;
    account->id = insertid;
}

void result_get_pc_list( MYSQL_RES *res, MYSQL_BIND *input, void *arg,
                         long insertid )
{
    PlayerPC_t        **resp;
    PlayerPC_t         *pc;
    int                 count;
    MYSQL_ROW           row;
    int                 i;

    resp = (PlayerPC_t **)arg;

    count = mysql_num_rows(res);
    if( count == 0 ) {
        *resp = NULL;
        return;
    }

    pc = CREATEN(PlayerPC_t, count);

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        pc[i].id              = atoi(row[0]);
        pc[i].account_id      = atoi(row[1]);
        pc[i].name            = memstrdup(row[2]);
        pc[i].complete        = (atoi(row[3]) ? TRUE : FALSE);
        pc[i].race_id         = atoi(row[4]);
        pc[i].align_moral     = atoi(row[5]);
        pc[i].align_ethical   = atoi(row[6]);
        pc[i].strength        = atoi(row[7]);
        pc[i].dexterity       = atoi(row[8]);
        pc[i].constitution    = atoi(row[9]);
        pc[i].intelligence    = atoi(row[10]);
        pc[i].wisdom          = atoi(row[11]);
        pc[i].charisma        = atoi(row[12]);
        pc[i].social_class    = atoi(row[13]);
        pc[i].birth_order     = atoi(row[14]);
        pc[i].siblings        = atoi(row[15]);
        pc[i].parents_married = (atoi(row[16]) ? TRUE : FALSE);
        pc[i].max_hit_points  = atoi(row[17]);
        pc[i].hit_points      = atoi(row[18]);
        pc[i].height          = atoi(row[19]);
        pc[i].weight          = atoi(row[20]);
        pc[i].age             = atoi(row[21]);
        pc[i].hair_color      = atoi(row[22]);
        pc[i].eye_color       = atoi(row[23]);
        pc[i].hair_length     = memstrdup(row[24]);
        pc[i].skin_tone       = atoi(row[25]);
        pc[i].experience      = atoi(row[26]);
    }

    *resp = pc;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
