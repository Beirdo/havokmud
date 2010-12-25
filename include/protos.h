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
*--------------------------------------------------------*/

/**
 * @file
 * @brief Prototypes and definitions for inter-thread communication
 */


#ifndef _protos_h_
#define _protos_h_

#include "environment.h"
#include <pthread.h>
#include <signal.h>
#include "linked_list.h"
#include "buffer.h"
#include "queue.h"
#include "protected_data.h"
#include "logging.h"
#include "balanced_btree.h"
#include "oldstructs.h"
#include "structs.h"
#include "protobuf_api.h"

/* CVS generated ID string (optional for h files) */
static char protos_h_ident[] _UNUSED_ = 
    "$Id$";

/*
 * Other prototypes to move later
 */
void LoginSendBanner( PlayerStruct_t *player );
void LoginStateMachine(PlayerStruct_t *player, char *arg);
void EditorStart( PlayerStruct_t *player, char **string, int maxlen );
void FlushQueue( QueueObject_t *queue, PlayerStruct_t *player );
PlayerStruct_t *FindCharacterNamed( char *name, PlayerStruct_t *oldPlayer );
int GetPlayerCount( void );

/* command_processor.c */
void JustLoggedIn( PlayerStruct_t *player );
void CommandParser( PlayerStruct_t *player, char *line );
void InitializeCommands( void );
void SetupCommands( CommandDef_t *commands, int count );
void AddCommand( CommandDef_t *cmd );
CommandDef_t *FindCommand( char *string );
void initializeFillWords( void );

/*
 * TODO: move these!
 */
int_func procGetFuncByName( char *name, proc_type type );
char *procGetNameByFunc( int_func func, proc_type type );
int procIsRegistered( int_func func, proc_type type );

/*
 * output.c
 */
void SendOutput( PlayerStruct_t *player, char *fmt, ... );
void SendOutputRaw( PlayerStruct_t *player, unsigned char *string, int len );
void SendToAll(char *messg);
void SendToAllAwake(char *messg);
void send_to_outdoor(char *messg);
void send_to_desert(char *messg);
void send_to_out_other(char *messg);
void send_to_arctic(char *messg);
void send_to_except(char *messg, struct char_data *ch);
void send_to_zone(char *messg, struct char_data *ch);
void send_to_room(char *messg, int room);
void send_to_room_except(char *messg, int room, struct char_data *ch);
void send_to_room_except_two(char *messg, int room, struct char_data *ch1, 
                             struct char_data *ch2);

/* 
 * ansi_output.c 
 */
int ParseAnsiColors(bool UsingAnsi, char *txt, char *buf);
void ScreenOff( PlayerStruct_t *player );

/*
 * text_process.c
 */
char *skip_spaces(char *string);
int search_block(char *arg, char **list, bool exact);
void remove_cr(char *output, char *input);
int get_number(char **name);
char *get_argument(char *line_in, char **arg_out);

/*
 * char_data.c
 */
int HasClass(struct char_data *ch, int clss);
int HowManyClasses( struct char_data *ch );
int pc_num_class(int clss);
char *AlignDesc(int value);


/*
 * dice.c
 */
int number(int from, int to);
int dice(PlayerStruct_t *player, int number, int size, int top);

/*
 * core_commands.c
 */
void            do_siteban(struct char_data *ch, char *argument, int cmd);
void            do_shutdow(struct char_data *ch, char *argument, int cmd);
void            do_shutdown(struct char_data *ch, char *argument, int cmd);
void            do_idea(struct char_data *ch, char *argument, int cmd);
void            do_typo(struct char_data *ch, char *argument, int cmd);
void            do_bug(struct char_data *ch, char *argument, int cmd);
void            do_viewfile(struct char_data *ch, char *argument, int cmd);

/*
 * mysql_handler.c
 */
void db_setup(void);
void db_initial_load(void);
void bind_numeric( MYSQL_BIND *data, long long int value, 
                   enum enum_field_types type );
void bind_string( MYSQL_BIND *data, char *value, enum enum_field_types type );
void bind_null_blob( MYSQL_BIND *data, void *value );
void db_queue_query( int queryId, QueryTable_t *queryTable,
                     MYSQL_BIND *queryData, int queryDataCount,
                     QueryResFunc_t queryCallback, void *queryCallbackArg,
                     pthread_mutex_t *queryMutex );

/*
 * db_api.c
 */

HavokResponse *db_get_setting( HavokRequest *req );
HavokResponse *db_set_setting( HavokRequest *req );
HavokResponse *db_load_account( HavokRequest *req );
HavokResponse *db_save_account( HavokRequest *req );
HavokResponse *db_get_pc_list( HavokRequest *req );
HavokResponse *db_load_pc( HavokRequest *req );
HavokResponse *db_save_pc( HavokRequest *req );
HavokResponse *db_find_pc( HavokRequest *req );


/*
 * protobuf_api.c
 */

char *pb_get_setting(char *name);
void pb_set_setting( char *name, char *format, ... );
PlayerAccount_t *pb_load_account( char *email );
void pb_save_account( PlayerAccount_t *account );
PlayerPC_t *pb_get_pc_list( int account_id );
PlayerPC_t *pb_load_pc( int account_id, int pc_id );
void pb_save_pc( PlayerPC_t *pc );
PlayerPC_t *pb_find_pc( char *name );




/*
 * object_data.c
 */
void initializeObjects( void );
struct index_data *objectIndex( int vnum );
void objectInsert(struct obj_data *obj, long vnum);
void objectClear(struct obj_data *obj);
struct obj_data *objectClone(struct obj_data *obj);
void objectCloneContainer(struct obj_data *to, struct obj_data *obj);
struct obj_data *objectRead(int nr);
void objectFree(struct obj_data *obj);
void objectExtract(struct obj_data *obj);
void objectExtractLocked(struct obj_data *obj, Locked_t locked );
void objectPutInObject(struct obj_data *obj, struct obj_data *obj_to);
void objectTakeFromObject(struct obj_data *obj, Locked_t locked);
void objectGiveToChar(struct obj_data *object, struct char_data *ch);
void objectTakeFromChar(struct obj_data *object);
void objectPutInRoom(struct obj_data *object, long room, Locked_t locked);
void objectTakeFromRoom(struct obj_data *object, Locked_t locked);
void objectSaveForChar(struct char_data *ch, int delete);
void load_char_objs(struct char_data *ch);
struct obj_data *objectGetInRoom( struct char_data *ch, char *name,
                                  struct room_data *rm );
struct obj_data *objectGetOnChar( struct char_data *ch, char *name,
                                  struct char_data *onch );
struct obj_data *objectGetInObject( struct char_data *ch, char *name,
                                    struct obj_data *obj );
struct obj_data *objectGetInEquip(struct char_data *ch, char *arg,
                                  struct obj_data *equipment[], int *j,
                                  bool visible );
struct obj_data *objectGetGlobal(struct char_data *ch, char *name, int *count);
struct obj_data *objectGetInCharOrRoom(struct char_data *ch, char *name);
struct obj_data *objectGetNumLastCreated(int num);
struct obj_data *objectGetOnCharNum(int num, struct char_data *ch);
struct obj_data *objectGetInRoomNum(int num, struct room_data *rm);

bool objectIsVisible(struct char_data *ch, struct obj_data *obj);

void objectKeywordTreeAdd( BalancedBTree_t *tree, struct obj_data *obj );
void objectKeywordTreeRemove( BalancedBTree_t *tree, struct obj_data *obj );
struct obj_data *objectKeywordFindFirst( BalancedBTree_t *tree, 
                                         Keywords_t *key );
struct obj_data *objectKeywordFindNext( BalancedBTree_t *tree, int offset,
                                        Keywords_t *key, 
                                        struct obj_data *lastobj );

void objectTypeTreeAdd( struct obj_data *obj );
void objectTypeTreeRemove( struct obj_data *obj );
struct obj_data *objectTypeFindFirst( ItemType_t type );
struct obj_data *objectTypeFindNext( ItemType_t type,
                                     struct obj_data *lastobj );

bool HasBitsEquipment(struct char_data *ch, int bits, int offset);
bool HasBitsInventory(struct char_data *ch, int bits, int offset);
bool HasBits(struct char_data *ch, int bits, int offset);
bool HasExtraBits(struct char_data *ch, int bits);

void PrintLimitedItems(void);

/*
 * mobile_data.c
 */
void initializeMobiles( void );
struct index_data *mobileIndex( int vnum );
void mobileInsert(struct char_data *obj, long vnum);
struct char_data *mobileRead(int nr);
void mobileWriteToFile(struct char_data *mob, void * mob_fi);
void mobileWrite(struct char_data *mob, void * mob_fi);
void mobileInitScripts(void);

/*
 * keywords.c
 */
char *KeywordsToString( Keywords_t *key, char *separator );
Keywords_t *StringToKeywords( char *string, Keywords_t *key );
void FreeKeywords( Keywords_t *key, bool freeRoot );
bool KeywordsMatch(Keywords_t *tofind, Keywords_t *keywords);
char *find_ex_description(char *word, Keywords_t *list, int count);

/*
 * weather.c
 */
void weather_and_time(int mode);
void another_hour(int mode);
void weather_change(void);
void GetMonth(int month);
void ChangeWeather(int change);
void switch_light(byte why);
int IsDarkOutside(struct room_data *rp);

/*
 * room_data.c
 */
void initializeRooms( void );
struct room_data *roomFindNum(int virtual);
void cleanout_room(struct room_data *rp);
void completely_cleanout_room(struct room_data *rp);
int roomCountObject(int nr, struct room_data *rp);

/*
 * From special_funcs.c
 */
void            assign_mobiles(void);
void            assign_objects(void);
void            assign_rooms(void);

/*
 * From mysql_upgrade.c
 */
void db_check_schema( char *setting, char *desc, int codeSupports, 
                      QueryTable_t *defSchema, int defSchemaCount, 
                      SchemaUpgrade_t *schemaUpgrade );

/*
 * From md5c.c
 */
void *opiemd5init(void);
void opiemd5update(void *ctx, unsigned char *input, unsigned int inputLen);
void opiemd5final(unsigned char *digest, void *ctx);

/*
 * From btoe.c
 */
char *opiebtoe (char *engout, char *c);
int opieetob ( char *out, char *e );

/*
 * From signals.c
 */
void signal_interrupt( int signum, void *info, void *secret );
void signal_everyone( int signum, void *info, void *secret );
void signal_death( int signum, void *info, void *secret );
void do_symbol( void *ptr );
void do_backtrace( int signum, void *ip );
void mainSighup( int signum, void *arg );

/*
 * From logging.c
 */
void logging_toggle_debug( int signum, void *info, void *secret );


/*
 * From json_attribs.c
 */
void AddJSONToTrees( JSONSource_t *js, PlayerPC_t *pc );
JSONSource_t *ExtractJSONFromTree( PlayerPC_t *pc );
char *CombineJSON( JSONSource_t *js );
JSONSource_t *SplitJSON( char *json );
void DestroyJSONSource( JSONSource_t *js );

void AddAttribute( char *json, char *source, PlayerPC_t *pc );
void DeleteAttribute( char *attrib, char *source, PlayerPC_t *pc );

char *GetAttributeString( PlayerPC_t *pc, char *attrib, char *source );
int GetAttributeInt( PlayerPC_t *pc, char *attrib, char *source );
double GetAttributeDouble( PlayerPC_t *pc, char *attrib, char *source );
bool GetAttributeBool( PlayerPC_t *pc, char *attrib, char *source );

void SetAttributeString( PlayerPC_t *pc, char *attrib, char *source, 
                         char *val );
void SetAttributeInt( PlayerPC_t *pc, char *attrib, char *source, int val );
void SetAttributeDouble( PlayerPC_t *pc, char *attrib, char *source, 
                         double val );
void SetAttributeBool( PlayerPC_t *pc, char *attrib, char *source, bool val );
void DestroyAttributes( PlayerPC_t *pc );



/*
 * From main.c
 */
void MainDelayExit( void );
void versionAdd( char *what, char *version );
void versionRemove( char *what );



/*************************************************************************
 * Support for different platforms
 *************************************************************************/
#include "config.h"

#ifndef HAVE_STRNLEN 
/* FreeBSD and Solaris seem to be missing strnlen */
size_t strnlen(const char *s, size_t maxlen);
#endif

#ifndef HAVE_STRSEP
/* Solaris seems to be missing strsep */
char *strsep(char **stringp, const char *delim);
#endif

#ifndef HAVE_STRDUP
char           *strdup(const char *str);
#endif

#ifndef HAVE_STRSTR
char           *strstr(register const char *s, register const char *find);
#endif

#ifndef HAVE_STRNDUP
/* OSX seems to be missing strndup */
char           *strndup(const char *s, size_t n);
#endif


#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
