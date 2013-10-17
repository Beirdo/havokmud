/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2013 Gavin Hurlbut
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

/**
 * @file
 * @brief Core database schema
 */

#include <string>
#include <vector>

#include "objects/DatabaseSchema.hpp"
#include "corefunc/Logging.hpp"

namespace havokmud {
    namespace corefunc {

        using havokmud::objects::DatabaseSchemaItem;
        using havokmud::objects::DatabaseSchema;

        static const int coreSupportedVersion = 6;

        static const DatabaseSchemaItem coreBaseSchema = {
            "CREATE TABLE `settings` (\n"
            "  `name` varchar(40) NOT NULL,\n"
            "  `value` varchar(40) NOT NULL default '',\n"
            "  PRIMARY KEY  (`name`)\n"
            ")\n",

            " CREATE TABLE `accounts` (\n"
            "  `id` INT NOT NULL AUTO_INCREMENT ,\n"
            "  `email` VARCHAR( 255 ) NOT NULL ,\n"
            "  `passwd` VARCHAR( 32 ) NOT NULL ,\n"
            "  `ansi` TINYINT NOT NULL DEFAULT 0,\n"
            "  `confirmed` TINYINT NOT NULL DEFAULT 0,\n"
            "  `confcode` VARCHAR( 255 ) NOT NULL , \n"
            "  PRIMARY KEY ( `id` ) ,\n"
            "  INDEX ( `email` ) ,\n"
            "  INDEX ( `confcode` ) \n"
            "  )\n",

            "CREATE TABLE `pcs` (\n"
            "  `id` INT NOT NULL AUTO_INCREMENT,\n"
            "  `account_id` INT NOT NULL,\n"
            "  `name` VARCHAR( 80 ) NOT NULL,\n"
            "  PRIMARY KEY ( `id` ) ,\n"
            "  INDEX ( `name` )\n"
            ")\n",

            "CREATE TABLE `pcattribs` (\n"
            "  `pc_id` int(11) NOT NULL,\n"
            "  `attribsrc` varchar(255) NOT NULL,\n"
            "  `attribjson` longtext NOT NULL,\n"
            "  `to_delete` tinyint not null default 0,\n"
            "  INDEX (`pc_id`),\n"
            "  INDEX (`attribsrc`)\n"
            ")\n"
        };

        static const std::vector<DatabaseSchemaItem> coreUpgradeItems = {
            // 0 -> 1
            {},
            // 1 -> 2
            { "ALTER TABLE `accounts` "
              "ADD `confirmed` TINYINT DEFAULT 0 NOT NULL,\n"
              "ADD `confcode` VARCHAR( 255 ) NOT NULL,\n"
              "ADD KEY ( `confcode` );"
            },
            // 2 -> 3
            { "CREATE TABLE `pcs` (\n"
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
              ");"
            },
            // 3 -> 4
            { "ALTER TABLE `pcs` DROP `complete`, DROP `race_id`,\n"
              "  DROP `align_moral`, DROP `align_ethical`, DROP `strength`,\n"
              "  DROP `dexterity`, DROP `constitution`, DROP `intelligence`,\n"
              "  DROP `wisdom`, DROP `charisma`, DROP `social_class`,\n"
              "  DROP `birth_order`, DROP `siblings`, DROP `parents_married`,\n"
              "  DROP `max_hit_points`, DROP `hit_points`, DROP `height`,\n"
              "  DROP `weight`, DROP `age`, DROP `hair_color`,\n"
              "  DROP `eye_color`, DROP `hair_length`, DROP `skin_tone`,\n"
              "  DROP `experience`;",

              "CREATE TABLE `pcattribs` (\n"
              "  `pc_id` int(11) NOT NULL AUTO_INCREMENT,\n"
              "  `attribsrc` varchar(255) NOT NULL,\n"
              "  `attribjson` longtext NOT NULL,\n"
              "  PRIMARY KEY (`pc_id`),\n"
              "  KEY `attribsrc` (`attribsrc`)\n"
              ")\n"
            },
            // 4 -> 5
            { "ALTER TABLE `accounts` "
              "CHANGE `passwd` `passwd` VARCHAR(32) NOT NULL" },
            // 5 -> 6
            { "ALTER TABLE `pcattribs` "
              "CHANGE `pc_id` `pc_id` int(11) NOT NULL, "
              "ADD `to_delete` TINYINT DEFAULT 0 NOT NULL, "
              "DROP PRIMARY KEY, "
              "ADD INDEX (`pc_id`)" },
        };

        DatabaseSchema coreDatabaseSchema("core", coreSupportedVersion,
                                          coreBaseSchema, coreUpgradeItems);
    }
}

