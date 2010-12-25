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
 * Handles MySQL database schema upgrades
 */

#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>
#include "environment.h"
#include "structs.h"
#include "protos.h"
#include "db_schema.h"
#include "logging.h"
#include "memory.h"

static char ident[] _UNUSED_ =
    "$Id$";


static SchemaUpgrade_t coreSchemaUpgrade[CURRENT_SCHEMA] = {
    /* 0 -> 1 */
    { { NULL, NULL, NULL, FALSE } },
    /* 1 -> 2 */
    { { "ALTER TABLE `accounts` ADD `confirmed` TINYINT DEFAULT 0 NOT NULL ,\n"
        "ADD `confcode` VARCHAR( 255 ) NOT NULL ,\n"
        "ADD KEY ( `confcode` )\n", NULL, NULL, FALSE },
      { NULL, NULL, NULL, FALSE }
    },
    /* 2 -> 3 */
    { { "CREATE TABLE `pcs` (\n"
        "  `id` INT NOT NULL AUTO_INCREMENT,\n"
        "  `account_id` INT NOT NULL,\n"
        "  `name` VARCHAR( 80 ) NOT NULL,\n"
        "  `complete` TINYINT NOT NULL DEFAULT 0,\n"
        "  `race_id` INT NOT NULL,\n"
        "  `align_moral` INT NOT NULL,\n"
        "  `align_ethical` INT NOT NULL,\n"
        "  `strength` INT NOT NULL,\n"
        "  `dexterity` INT NOT NULL,\n"
        "  `constitution` INT NOT NULL,\n"
        "  `intelligence` INT NOT NULL,\n"
        "  `wisdom` INT NOT NULL,\n"
        "  `charisma` INT NOT NULL,\n"
        "  `social_class` INT NOT NULL,\n"
        "  `birth_order` INT NOT NULL,\n"
        "  `siblings` INT NOT NULL,\n"
        "  `parents_married` TINYINT NOT NULL DEFAULT -1,\n"
        "  `max_hit_points` INT NOT NULL,\n"
        "  `hit_points` INT NOT NULL,\n"
        "  `height` INT NOT NULL,\n"
        "  `weight` INT NOT NULL,\n"
        "  `age` INT NOT NULL,\n"
        "  `hair_color` INT NOT NULL,\n"
        "  `eye_color` INT NOT NULL,\n"
        "  `hair_length` VARCHAR( 80 ) NOT NULL,\n"
        "  `skin_tone` INT NOT NULL,\n"
        "  `experience` INT NOT NULL,\n"
        "  PRIMARY KEY ( `id` ) ,\n"
        "  INDEX ( `name` )\n"
        ")\n", NULL, NULL, FALSE },
      { NULL, NULL, NULL, FALSE }
    },
    /* 3 -> 4 */
    { { "ALTER TABLE `pcs` DROP `complete`, DROP `race_id`, "
        "  DROP `align_moral`, DROP `align_ethical`, DROP `strength`, "
        "  DROP `dexterity`, DROP `constitution`, DROP `intelligence`, "
        "  DROP `wisdom`, DROP `charisma`, DROP `social_class`, "
        "  DROP `birth_order`, DROP `siblings`, DROP `parents_married`, "
        "  DROP `max_hit_points`, DROP `hit_points`, DROP `height`, "
        "  DROP `weight`, DROP `age`, DROP `hair_color`, DROP `eye_color`, "
        "  DROP `hair_length`, DROP `skin_tone`, DROP `experience`;",
        NULL, NULL, FALSE },
      { "CREATE TABLE `pcattribs` (\n"
        "  `pc_id` int(11) NOT NULL AUTO_INCREMENT,\n"
        "  `attribsrc` varchar(255) NOT NULL,\n"
        "  `attribjson` longtext NOT NULL,\n"
        "  PRIMARY KEY (`pc_id`),\n"
        "  KEY `attribsrc` (`attribsrc`)\n"
        ")\n", NULL, NULL, FALSE },
      { NULL, NULL, NULL, FALSE }
    },
    /* 4 -> 5 */
    { { "ALTER TABLE `accounts` CHANGE `passwd` `passwd` VARCHAR(32) NOT NULL",
        NULL, NULL, FALSE },
      { NULL, NULL, NULL, FALSE }
    }
    /* 5 -> 6 */
};

/* Internal protos */
extern void db_queue_query( int queryId, QueryTable_t *queryTable,
                            MYSQL_BIND *queryData, int queryDataCount,
                            QueryResFunc_t queryCallback, 
                            void *queryCallbackArg,
                            pthread_mutex_t *queryMutex );
static int db_upgrade_schema( char *setting, char *desc, 
                              QueryTable_t *defSchema, int defSchemaCount,
                              SchemaUpgrade_t *schemaUpgrade, int current, 
                              int codeSupports );

void db_check_schema_main( void )
{
    db_check_schema( "dbSchema", "core", CURRENT_SCHEMA, defCoreSchema,
                     defCoreSchemaCount, coreSchemaUpgrade );
}

void db_check_schema( char *setting, char *desc, int codeSupports, 
                      QueryTable_t *defSchema, int defSchemaCount, 
                      SchemaUpgrade_t *schemaUpgrade )
{
    char               *verString;
    int                 ver;
    int                 printed;

    ver = -1;
    printed = FALSE;
    do {
        verString = pb_get_setting( setting );
        if( !verString ) {
            ver = 0;
        } else {
            ver = atoi( verString );
            memfree( verString );
        }

        if( !printed ) {
            LogPrint( LOG_CRIT, "Current %s database schema version %d", desc,
                                ver );
            LogPrint( LOG_CRIT, "Code supports version %d", codeSupports );
            printed = TRUE;
        }

        if( ver < codeSupports ) {
            ver = db_upgrade_schema( setting, desc, defSchema, defSchemaCount,
                                     schemaUpgrade, ver, codeSupports );
        }
    } while( ver < codeSupports );
}

static int db_upgrade_schema( char *setting, char *desc, 
                              QueryTable_t *defSchema, int defSchemaCount,
                              SchemaUpgrade_t *schemaUpgrade, int current, 
                              int codeSupports )
{
    int                 i;

    if( current >= codeSupports ) {
        return( current );
    }

    if( current <= 0 ) {
        /* There is no dbSchema, assume that it is an empty database, populate
         * with the default schema
         */
        LogPrint( LOG_ERR, "Initializing %s database to schema version %d",
                  desc, codeSupports );
        for( i = 0; i < defSchemaCount; i++ ) {
            db_queue_query( i, defSchema, NULL, 0, NULL, NULL, NULL );
        }
        pb_set_setting( setting, "%d", codeSupports );
        return( codeSupports );
    }

    LogPrint( LOG_ERR, "Upgrading %s database from schema version %d to %d",
                       desc, current, current+1 );
    for( i = 0; schemaUpgrade[current][i].queryPattern; i++ ) {
        db_queue_query( i, schemaUpgrade[current], NULL, 0, NULL, NULL, NULL );
    }

    current++;

    pb_set_setting( setting, "%d", current );
    return( current );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
