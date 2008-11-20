/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
 * Copyright 2007 Gavin Hurlbut
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

#include "protos.h"
#include "externs.h"
#include "logging.h"
#include "interthread.h"

static char ident[] _UNUSED_ =
    "$Id$";


/**
 * @file
 * @brief Contains the API for the system to use to access the MySQL database.
 */


/* Externally accessible */
struct class_def *classes;
int             classCount;

struct skill_def *skills;
int             skillCount;

struct dir_data *direction;
int             directionCount;

struct clan    *clan_list;
int             clanCount;

struct sector_data *sectors;
int             sectorCount;

struct message_list *fightMessages;
int             fightMessageCount;

struct banned_user *bannedUsers = NULL;
int             bannedUserCount;

struct social_messg *socialMessages = NULL;
int                 socialMessageCount = 0;

struct message_list  *kickMessages;
int                   kickMessageCount;

struct pose_type   *poseMessages;
int                 poseMessageCount;

struct race_type   *races;
int                 raceCount;

struct lang_def    *languages;
int                 languageCount;


/* Internal protos */
char *db_quote(char *string);
void db_load_classes(void);
void db_load_skills(void);
void db_load_structures(void);
void db_load_messages(void);
void db_load_bannedUsers(void);
void db_load_socials(void);
void db_load_kick_messages(void);
void db_load_poses(void);
void db_load_races(void);
void db_load_languages(void);


void chain_load_classes( MYSQL_RES *res, QueryItem_t *item );
void chain_load_skills( MYSQL_RES *res, QueryItem_t *item );
void chain_load_messages( MYSQL_RES *res, QueryItem_t *item );
void chain_load_socials( MYSQL_RES *res, QueryItem_t *item );
void chain_load_poses( MYSQL_RES *res, QueryItem_t *item );
void chain_assign_specials( MYSQL_RES *res, QueryItem_t *item );
void chain_load_races( MYSQL_RES *res, QueryItem_t *item );
void chain_load_languages( MYSQL_RES *res, QueryItem_t *item );
void chain_load_textfiles( MYSQL_RES *res, QueryItem_t *item );
void chain_delete_board_message( MYSQL_RES *res, QueryItem_t *item );
void chain_load_object_tree( MYSQL_RES *res, QueryItem_t *item );
void chain_load_rooms( MYSQL_RES *res, QueryItem_t *item );

void result_get_report( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_classes_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_classes_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_classes_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_messages( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_structures_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_structures_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_structures_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_bannedUsers( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_races( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_lookup_board( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_get_board_message( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_get_board_replies( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_has_mail( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_get_mail_ids( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_get_mail_message( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_check_kill_file( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_lookup_help( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_lookup_help_similar( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_read_object_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_read_object_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_read_object_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_read_object_4( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_read_object_5( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_find_object_named( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_object_tree( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_char_extra_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_char_extra_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_get_char_rent( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_char_objects_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_room_objects_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_rooms_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_rooms_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_rooms_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg );
void result_load_rooms_4( MYSQL_RES *res, MYSQL_BIND *input, void *arg );


QueryTable_t    QueryTable[] = {
    /* 0 */
    { "INSERT INTO `userReports` (`reportId`, `reportTime`, `character`, "
      "`roomNum`, `report`) VALUES ( ?, NULL, ?, ?, ? )", NULL, NULL, FALSE },
    /* 1 */
    { "SELECT `reportTime`, `character`, `roomNum`, `report` FROM "
      "`userReports` WHERE `reportId` = ? ORDER BY `reportTime` ASC", NULL,
      NULL, FALSE },
    /* 2 */
    { "DELETE FROM `userReports` WHERE `reportId` = ?",  NULL, NULL, FALSE },
    /* 3 */
    { "SELECT classId, className, classAbbrev FROM classes ORDER BY classId "
      "ASC", chain_load_classes, NULL, FALSE },
    /* 4 */
    { "SELECT skills.skillName, skills.skillID, classSkills.minLevel, "
      "classSkills.maxTeach FROM skills, classSkills "
      "WHERE skills.skillId = classSkills.skillId AND classSkills.classId = ? "
      "AND classSkills.mainSkill = ? ORDER BY skills.skillName ASC", NULL, 
      NULL, FALSE },
    /* 5 */
    { "SELECT level, thaco, maleTitle, femaleTitle, minExp FROM classLevels "
      "WHERE classId = ? ORDER BY level", NULL, NULL, FALSE },
    /* 6 */
    { "SELECT skillId, skillName, skillType FROM skills ORDER BY skillId ASC",
      chain_load_skills, NULL, FALSE },
    /* 7 */
    { "SELECT text FROM skillMessages WHERE `skillId` = ? AND `msgId` = ? AND "
      "`index` = ?", NULL, NULL, FALSE },
    /* 8 */
    { "SELECT forward, reverse, trapBits, exit, listExit, direction, "
      "description FROM directions ORDER BY forward ASC", NULL, NULL, FALSE },
    /* 9 */
    { "SELECT clanId, clanName, shortName, description, homeRoom "
      "FROM playerClans ORDER BY clanId ASC", NULL, NULL, FALSE },
    /* 10 */
    { "SELECT sectorType, mapChar, moveLoss FROM sectorType ORDER BY "
      "sectorId ASC", NULL, NULL, FALSE },
    /* 11 */
    { "SELECT DISTINCT skillId FROM skillMessages WHERE msgId = ? ORDER BY "
      "skillId", chain_load_messages, NULL, FALSE },
    /* 12 */
    { "SELECT name FROM bannedName", NULL, NULL, FALSE },
    /* 13 */
    { "SELECT socialId, hide, minPosition FROM socials ORDER BY socialId",
      chain_load_socials, NULL, FALSE },
    /* 14 */
    { "SELECT DISTINCT skillId FROM skillMessages WHERE msgId = 5 OR "
      "msgId = 6 ORDER BY skillId", chain_load_poses, NULL, FALSE },
    /* 15 */
    { "SELECT `vnum`, `procedure` FROM procAssignments WHERE `procType` = ? "
      "ORDER BY `vnum`", chain_assign_specials, NULL, FALSE },
    /* 16 */
    { "SELECT raceId, raceName, description, raceSize, ageStart, ageYoung, "
      "ageMature, ageMiddle, ageOld, ageAncient, ageVenerable, nativeLanguage "
      "FROM races ORDER BY raceId", chain_load_races, NULL, FALSE },
    /* 17 */
    { "SELECT maxLevel FROM racialMax WHERE raceId = ? AND classId = ?", NULL,
      NULL, FALSE },
    /* 18 */
    { "SELECT `langId`, `skillId`, `name` FROM languages ORDER BY `langId`",
      chain_load_languages, NULL, FALSE },
    /* 19 */
    { "SELECT fileContents FROM textFiles WHERE fileId = ?", 
      chain_load_textfiles, NULL, FALSE },
    /* 20 */
    { "DELETE FROM textFiles WHERE `fileId` = ?", NULL, NULL, FALSE },
    /* 21 */
    { "INSERT INTO textFiles (`fileId`, `lastModified`, `lastModBy`, "
      "`fileContents`) VALUES (?, NULL, ?, ?)", NULL, NULL, FALSE },
    /* 22 */
    { "SELECT `messageNum` FROM `boardMessages` WHERE `boardId` = ? AND "
      "`replyToMessageNum` = ?", chain_delete_board_message, NULL, FALSE },
    /* 23 */
    { "DELETE FROM `boardMessages` where `messageNum` = ? AND `boardId` = ?",
      NULL, NULL, FALSE },
    /* 24 */
    { "UPDATE `boardMessages` SET `messageNum` = `messageNum` - 1 " 
      "WHERE `messageNum` > ? AND `boardId` = ?", NULL, NULL, FALSE },
    /* 25 */
    { "UPDATE `boards` SET `maxPostNum` = `maxPostNum` - 1 WHERE boardId = ?",
      NULL, NULL, FALSE },
    /* 26 */
    { "SELECT `boardId`, `maxPostNum`, `minLevel` FROM `boards` WHERE "
      "`vnum` = ?", NULL, NULL, FALSE },
    /* 27 */
    { "SELECT `poster`, `topic`, `post`, UNIX_TIMESTAMP(`postTime`), "
      "`messageNum`, `replyToMessageNum` FROM `boardMessages` "
      "WHERE `boardId` = ? AND messageNum = ?", NULL, NULL, FALSE },
    /* 28 */
    { "SELECT `poster`, `topic`, `post`, `messageNum`, " 
      "UNIX_TIMESTAMP(`postTime`), `replyToMessageNum` FROM `boardMessages` "
      "WHERE `boardId` = ? AND replyToMessageNum = ? ORDER BY `messageNum`", 
      NULL, NULL, FALSE },
    /* 29 */
    { "INSERT INTO `boardMessages` (`boardId`, `messageNum`, "
      "`replyToMessageNum`, `topic`, `poster`, `postTime`, `post`) VALUES "
      "(?, ?, ?, ?, ?, NULL, ?)", NULL, NULL, FALSE },
    /* 30 */
    { "UPDATE `boards` SET `maxPostNum` = `maxPostNum` + 1 WHERE `boardId` = ?",
      NULL, NULL, FALSE },
    /* 31 */
    { "INSERT INTO `mailMessages` (`mailFrom`, `mailTo`, `timestamp`, "
      "`message`) VALUES (?, ?, NULL, ?)", NULL, NULL, FALSE },
    /* 32 */
    { "SELECT HIGH_PRIORITY COUNT(*) as count FROM `mailMessages` "
      "WHERE LOWER(`mailTo`) = LOWER(?)", NULL, NULL, FALSE },
    /* 33 */
    { "SELECT `messageNum` FROM `mailMessages` WHERE LOWER(`mailTo`) = "
      "LOWER(?) ORDER BY `timestamp` LIMIT ?", NULL, NULL, FALSE },
    /* 34 */
    { "SELECT `mailFrom`, `mailTo`, "
      "DATE_FORMAT(`timestamp`, '%a %b %c %Y  %H:%i:%S'), `message` "
      "FROM `mailMessages` WHERE `messageNum` = ?", NULL, NULL, FALSE },
    /* 35 */
    { "DELETE FROM `mailMessages` WHERE `messageNum` = ?", NULL, NULL, FALSE },
    /* 36 */
    { "SELECT `vnum` FROM `mobKillfile` WHERE `vnum` = ?", NULL, NULL, FALSE },
    /* 37 */
    { "SELECT `keywords`, `helpText` FROM `helpTopics` WHERE `helpType` = ? "
      "AND UPPER(`keywords`) = UPPER(?)", NULL, NULL, FALSE },
    /* 38 */
    { "SELECT UPPER(`keywords`), "
      "MATCH(`keywords`, `helpText`) AGAINST(?) AS score "
      "FROM  `helpTopics` WHERE `helpType` = ? AND "
      "MATCH(keywords, helpText) AGAINST(?) ORDER BY score DESC LIMIT 10",
      NULL, NULL, FALSE },
    /* 39 */
    { "REPLACE INTO `objects` (`vnum`, `ownerId`, `roomId`, `ownedItemId`, "
      "`shortDescription`, `description`, `actionDescription`, `modBy`, "
      "`itemType`, `value0`, `value1`, `value2`, `value3`, `weight`, `cost`, "
      "`costPerDay`, `level`, `max`, `modified`, `speed`, `weaponType`, "
      "`tweak`, `wearPos`, `inOwnedItemId`) VALUES ( ?, ?, ?, ?, ?, ?, ?, ?, "
      "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, FROM_UNIXTIME(?), ?, ?, ?, ?, ?)", NULL, 
      NULL, FALSE },
    /* 40 */
    { "REPLACE INTO `objectKeywords` (`vnum`, `ownerId`, `roomId`, "
      "`ownedItemId`, `seqNum`, `keyword`) VALUES ( ?, ?, ?, ?, ?, ?)", NULL, 
      NULL, FALSE },
    /* 41 */
    { "DELETE FROM `objectKeywords` WHERE `vnum` = ? AND `ownerId` = ? "
      "AND `roomId` = ? AND `ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL, 
      FALSE },
    /* 42 */
    { "REPLACE INTO `objectFlags` (`vnum`, `ownerId`, `roomId`, `ownedItemId`, "
      "`seqNum`, `flagTypeId`, `index`, `value`) VALUES "
      "(?, ?, ?, ?, ?, ?, ?, ?)", NULL, NULL, FALSE },
    /* 43 */
    { "REPLACE INTO `objectExtraDesc` (`vnum`, `ownerId`, `roomId`, "
      "`ownedItemId`, `seqNum`, `keyword`, `description`) VALUES "
      "( ?, ?, ?, ?, ?, ?, ? )", NULL, NULL, FALSE },
    /* 44 */
    { "DELETE FROM `objectExtraDesc` WHERE `vnum` = ? AND `ownerId` = ? AND "
      "`roomId` = ? AND `ownedItemId` = ? AND `seqNum` > ?", NULL, NULL, 
      FALSE },
    /* 45 */
    { "REPLACE INTO `objectAffects` (`vnum`, `ownerId`, `roomId`, "
      "`ownedItemId`, `seqNum`, `location`, `modifier`) VALUES "
      "( ?, ?, ?, ?, ?, ?, ? )", NULL, NULL, FALSE },
    /* 46 */
    { "DELETE FROM `objectAffects` WHERE `vnum` = ? AND `ownerId` = ? AND "
      "`roomId` = ? AND `ownedItemId` = ? AND `seqNum` > ?", NULL, NULL, 
      FALSE },
    /* 47 */
    { "SELECT `shortDescription`, `description`, `actionDescription`, "
      "`modBy`, `itemType`, `value0`, `value1`, `value2`, `value3`, `weight`, "
      "`cost`, " "`costPerDay`, `level`, `max`, UNIX_TIMESTAMP(`modified`), "
      "`speed`, `weaponType`, `tweak`, `wearPos` FROM `objects` "
      "WHERE `vnum` = ? AND `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ?", NULL, NULL, FALSE },
    /* 48 */
    { "SELECT `keyword` FROM `objectKeywords` WHERE `vnum` = ? AND "
      "`ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 49 */
    { "SELECT `flagTypeId`, `index`, `value` FROM `objectFlags` " 
      "WHERE `vnum` = ? AND `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ? ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 50 */
    { "SELECT `keyword`, `description` FROM `objectExtraDesc` WHERE "
      "`vnum` = ? AND `ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 51 */
    { "SELECT `location`, `modifier` FROM `objectAffects` WHERE `vnum` = ? "
      "AND `ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 52 */
    { "SELECT `vnum` FROM `objects` WHERE `ownerId` = 0 AND `roomId` = -1 AND "
      "ownedItemId = -1 ORDER BY `vnum`", chain_load_object_tree, NULL,
      FALSE },
    /* 53 */
    { "SELECT `keyword` FROM `objectKeywords` WHERE `vnum` = ? AND "
      "`ownerId` = -1 AND `ownedItemId` = -1 ORDER BY `seqNum`", NULL, NULL,
      FALSE },
    /* 54 */
    { "DELETE FROM `objectFlags` WHERE `vnum` = ? AND `ownerId` = ? AND "
      "`roomId` = ? AND `ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL,
      FALSE },
    /* 55 */
    { "UPDATE `players` SET `gold` = ?, `totalCost` = ?, `minStay` = ?, "
      "`lastUpdate` = ? WHERE `playerId` = ?", NULL, NULL, FALSE },
    /* 56 */
    { "DELETE FROM `objects` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` >= ?", NULL, NULL, FALSE },
    /* 57 */
    { "UPDATE `players` SET `bamfin` = ?, `bamfout` = ?, `setsev` = ?, "
      "`invisLevel` = ?, `editZone` = ?, `prompt` = ?, `battlePrompt` = ?, "
      "`email` = ?, `clan` = ?, `hostIp` = ?, `rumor` = ? WHERE `playerId` = ?",
      NULL, NULL, FALSE },
    /* 58 */
    { "REPLACE INTO `playerAliases` (`playerId`, `seqNum`, `index`, "
      "`alias`) VALUES ( ?, ?, ?, ? )", NULL, NULL, FALSE },
    /* 59 */
    { "DELETE FROM `playerAliases` WHERE `playerId` = ? AND `seqNum` >= ?",
      NULL, NULL, FALSE },
    /* 60 */
    { "SELECT `bamfout`, `bamfin`, `editZone`, `prompt`, `battlePrompt`, "
      "`email`, `clan`, `hostIp`, `rumor`, `setsev`, `invisLevel` FROM "
      "`players` WHERE `playerId` = ?", NULL, NULL, FALSE },
    /* 61 */
    { "SELECT `index`, `alias` FROM `playerAliases` WHERE `playerId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 62 */
    { "SELECT `gold`, `totalCost`, `minStay`, `lastUpdate` FROM `players` "
      "WHERE `playerId` = ?", NULL, NULL, FALSE },
    /* 63 */
    { "DELETE FROM `objectAffects` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL, FALSE },
    /* 64 */
    { "DELETE FROM `objectExtraDesc` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL, FALSE },
    /* 65 */
    { "DELETE FROM `objectFlags` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL, FALSE },
    /* 66 */
    { "DELETE FROM `objectKeywords` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`ownedItemId` = ? AND `seqNum` >= ?", NULL, NULL, FALSE },
    /* 67 */
    { "SELECT `vnum, `ownedItemId`, `shortDescription`, `description`, "
      "`actionDescription`, `modBy`, `itemType`, `value0`, `value1`, "
      "`value2`, `value3`, `weight`, `cost`, " "`costPerDay`, `level`, "
      "`max`, UNIX_TIMESTAMP(`modified`), `speed`, `weaponType`, `tweak`, "
      "`wearPos` FROM `objects` WHERE `ownerId` = ? AND `roomId` = ? AND "
      "`inOwnedItemId` = ?", NULL, NULL, FALSE },
    /* 68 */
    { "SELECT `keyword` FROM `objectKeywords` WHERE `ownerId` = ? AND "
      "`roomId` = ? AND `ownedItemId` = ? ORDER BY `seqNum`", NULL, NULL, 
      FALSE },
    /* 69 */
    { "SELECT `flagTypeId`, `index`, `value` FROM `objectFlags` " 
      "WHERE `ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 70 */
    { "SELECT `keyword`, `description` FROM `objectExtraDesc` WHERE "
      "`ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 71 */
    { "SELECT `location`, `modifier` FROM `objectAffects` WHERE "
      "`ownerId` = ? AND `roomId` = ? AND `ownedItemId` = ? ORDER BY `seqNum`",
      NULL, NULL, FALSE },
    /* 72 */
    { "SELECT `roomId`, `zoneId`, `name`, `description`, `sectorType`, "
      "`teleportTime`, `teleportTarget`, `teleportCount`, `riverSpeed`, "
      "`riverDir`, `mobLimit`, `specialFunc`, `light` FROM `rooms`", 
      chain_load_rooms, NULL, FALSE },
    /* 73 */
    { "SELECT `flagTypeId`, `index`, `value` FROM `roomFlags` WHERE "
      "`roomId` = ?", NULL, NULL, FALSE },
    /* 74 */
    { "SELECT `keyword`, `description` FROM `roomExtraDesc` WHERE `roomId` = ? "
      "ORDER BY `seqNum`", NULL, NULL, FALSE },
    /* 75 */
    { "SELECT `direction`, `description`, `keyword`, `key`, `toRoom`, "
      "`openCommand` FROM `roomExits` WHERE `roomId` = ? AND `direction` = ?", 
      NULL, NULL, FALSE },
    /* 76 */
    { "SELECT `direction`, `flagTypeId`, `index`, `value` FROM `roomExitFlags` "
      "WHERE `roomId` = ? AND `direction` = ?", NULL, NULL, FALSE },
    /* END */
    { NULL, NULL, NULL, FALSE }
};

struct obj_flag_bits {
    unsigned int set;
    unsigned int clear;
};


static struct obj_flag_bits obj_wear_flags[] = {
    { ITEM_TAKE, 0 },
    { ITEM_WEAR_FINGER, 0 },
    { ITEM_WEAR_NECK, 0 },
    { ITEM_WEAR_BODY, 0 },
    { ITEM_WEAR_HEAD, 0 },
    { ITEM_WEAR_LEGS, 0 },
    { ITEM_WEAR_FEET, 0 },
    { ITEM_WEAR_HANDS, 0 },
    { ITEM_WEAR_ARMS, 0 },
    { ITEM_WEAR_SHIELD, 0 },
    { ITEM_WEAR_ABOUT, 0 },
    { ITEM_WEAR_WAIST, 0 },
    { ITEM_WEAR_WRIST, 0 },
    { ITEM_WEAR_BACK, 0 },
    { ITEM_WEAR_EAR, 0 },
    { ITEM_WEAR_EYE, 0 },
    { ITEM_LIGHT_SOURCE, 0 },
    { ITEM_HOLD, 0 },
    { ITEM_WIELD, 0 },
    { ITEM_THROW, 0 }
};

static struct obj_flag_bits obj_extra_flags[] = {
    { ITEM_GLOW, 0 },
    { ITEM_HUM, 0 },
    { ITEM_METAL, 0 },
    { ITEM_MINERAL, 0 },
    { ITEM_ORGANIC, 0 },
    { ITEM_INVISIBLE, 0 },
    { ITEM_MAGIC, 0 },
    { ITEM_NODROP, 0 },
    { ITEM_BRITTLE, 0 },
    { ITEM_RESISTANT, 0 },
    { ITEM_IMMUNE, 0 },
    { ITEM_RARE, 0 },
    { ITEM_UBERRARE, 0 },
    { ITEM_QUEST, 0 }
};

static struct obj_flag_bits obj_anti_flags[] = {
    { ITEM_ANTI_SUN, 0 },
    { ITEM_ANTI_GOOD, 0 },
    { ITEM_ANTI_EVIL, 0 },
    { ITEM_ANTI_NEUTRAL, 0 },
    { ITEM_ANTI_MEN, 0 },
    { ITEM_ANTI_WOMEN, 0 }
};

static struct obj_flag_bits obj_only_class_flags[] = {
    { ITEM_ANTI_MAGE | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_CLERIC | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_FIGHTER | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_THIEF | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_DRUID | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_MONK | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_BARBARIAN | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_SORCERER | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_PALADIN | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_RANGER | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_PSI | ITEM_ONLY_CLASS, 0 },
    { ITEM_ANTI_NECROMANCER | ITEM_ONLY_CLASS, 0 }
};

static struct obj_flag_bits obj_anti_class_flags[] = {
    { ITEM_ANTI_MAGE, ITEM_ONLY_CLASS },
    { ITEM_ANTI_CLERIC, ITEM_ONLY_CLASS },
    { ITEM_ANTI_FIGHTER, ITEM_ONLY_CLASS },
    { ITEM_ANTI_THIEF, ITEM_ONLY_CLASS },
    { ITEM_ANTI_DRUID, ITEM_ONLY_CLASS },
    { ITEM_ANTI_MONK, ITEM_ONLY_CLASS },
    { ITEM_ANTI_BARBARIAN, ITEM_ONLY_CLASS },
    { ITEM_ANTI_SORCERER, ITEM_ONLY_CLASS },
    { ITEM_ANTI_PALADIN, ITEM_ONLY_CLASS },
    { ITEM_ANTI_RANGER, ITEM_ONLY_CLASS },
    { ITEM_ANTI_PSI, ITEM_ONLY_CLASS },
    { ITEM_ANTI_NECROMANCER, ITEM_ONLY_CLASS }
};

struct obj_flags_t {
    struct obj_flag_bits   *bits;
    int                     count;
    int                     offset;
};

static struct obj_flags_t obj_flags[] = {
    { NULL, 0, 0 },
    { obj_wear_flags,       NELEMENTS(obj_wear_flags), 
      OFFSETOF(wear_flags, struct obj_data) },
    { obj_extra_flags,      NELEMENTS(obj_extra_flags),
      OFFSETOF(extra_flags, struct obj_data) },
    { obj_anti_flags,       NELEMENTS(obj_anti_flags),
      OFFSETOF(anti_flags, struct obj_data) },
    { obj_only_class_flags, NELEMENTS(obj_only_class_flags),
      OFFSETOF(anti_class, struct obj_data) },
    { obj_anti_class_flags, NELEMENTS(obj_anti_class_flags),
      OFFSETOF(anti_class, struct obj_data) }
};
static int obj_flag_count = NELEMENTS(obj_flags);

static struct obj_flag_bits room_flags_flags[] = {
    { DARK, 0 },
    { DEATH, 0 },
    { NO_MOB, 0 },
    { INDOORS, 0 },
    { PEACEFUL, 0 },
    { NOSTEAL, 0 },
    { NO_SUM, 0 },
    { NO_MAGIC, 0 },
    { TUNNEL, 0 },
    { PRIVATE, 0 },
    { SILENCE, 0 },
    { LARGE, 0 },
    { NO_DEATH, 0 },
    { SAVE_ROOM, 0 },
    { ARENA_ROOM, 0 },
    { NO_FLY, 0 },
    { REGEN_ROOM, 0 },
    { FIRE_ROOM, 0 },
    { ICE_ROOM, 0 },
    { WIND_ROOM, 0 },
    { EARTH_ROOM, 0 },
    { ELECTRIC_ROOM, 0 },
    { WATER_ROOM, 0 },
    { MOVE_ROOM, 0 },
    { MANA_ROOM, 0 },
    { NO_FLEE, 0 },
    { NO_SPY, 0 },
    { EVER_LIGHT, 0 },
    { ROOM_WILDERNESS, 0 }
};

static struct obj_flag_bits room_tele_mask_flags[] = {
    { TELE_LOOK, 0 },
    { TELE_COUNT, 0 },
    { TELE_RANDOM, 0 },
    { TELE_SPIN, 0 }
};

static struct obj_flags_t room_flags[] = {
    { room_flags_flags,     NELEMENTS(room_flags_flags), 
      OFFSETOF(room_flags, struct room_data) },
    { room_tele_mask_flags, NELEMENTS(room_tele_mask_flags),
      OFFSETOF(tele_mask, struct room_data) }
};
static int room_flag_count = NELEMENTS(room_flags);

static struct obj_flag_bits room_exit_flags[] = {
    { EX_ISDOOR, 0 },
    { EX_CLOSED, 0 },
    { EX_LOCKED, 0 },
    { EX_SECRET, 0 },
    { EX_RSLOCKED, 0 },
    { EX_PICKPROOF, 0 },
    { EX_CLIMB, 0 }
};

static struct obj_flags_t exit_flags[] = {
    { room_exit_flags,     NELEMENTS(room_exit_flags), 
      OFFSETOF(exit_info, struct room_direction_data) }
};
static int exit_flag_count = NELEMENTS(exit_flags);

void db_report_entry(int reportId, struct char_data *ch, char *report)
{
    MYSQL_BIND     *data;

    if( !report ) {
        return;
    }

    data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
    memset( data, 0, 4 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], reportId, MYSQL_TYPE_LONG );
    bind_string( &data[1], GET_NAME(ch), MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[2], ch->in_room, MYSQL_TYPE_LONG );
    bind_string( &data[3], report, MYSQL_TYPE_VAR_STRING );

    db_queue_query( 0, QueryTable, data, 4, NULL, NULL, NULL );
}

struct user_report *db_get_report(int reportId)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    struct user_report *report;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], reportId, MYSQL_TYPE_LONG );
    db_queue_query( 1, QueryTable, data, 1, result_get_report, 
                    (void *)&report, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( report );
}

void db_clean_report(int reportId)
{
    MYSQL_BIND         *data;

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], reportId, MYSQL_TYPE_LONG );
    db_queue_query( 2, QueryTable, data, 1, NULL, NULL, NULL );
}

void db_initial_load(void)
{
    db_load_classes();
    db_load_skills();
    db_load_structures();
    db_load_messages();
    db_load_bannedUsers();
    db_load_socials();
    db_load_kick_messages();
    db_load_poses();
    db_load_races();
    db_load_languages();
}

void db_load_classes(void)
{
    LogPrintNoArg(LOG_CRIT, "Loading classes[] from SQL");

    db_queue_query( 3, QueryTable, NULL, 0, NULL, NULL, NULL );
}


void db_load_skills(void)
{
    LogPrintNoArg(LOG_CRIT, "Loading skills[] from SQL");

    db_queue_query( 6, QueryTable, NULL, 0, NULL, NULL, NULL );
}

void db_load_structures(void)
{
    pthread_mutex_t    *mutex;

    LogPrintNoArg(LOG_CRIT, "Loading misc structures from SQL");

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    /* direction[] */
    db_queue_query( 8, QueryTable, NULL, 0, result_load_structures_1, NULL, 
                    mutex );
    pthread_mutex_unlock( mutex );

    /* clan_list[] */
    db_queue_query( 9, QueryTable, NULL, 0, result_load_structures_2, NULL, 
                    mutex );
    pthread_mutex_unlock( mutex );

    /* sectors[] */
    db_queue_query( 10, QueryTable, NULL, 0, result_load_structures_3, NULL, 
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finished loading misc structures from SQL");
}


void db_load_messages(void)
{
    MYSQL_BIND         *data;

    LogPrintNoArg(LOG_CRIT, "Loading fight messages from SQL");

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], 2, MYSQL_TYPE_LONG );

    db_queue_query( 11, QueryTable, data, 1, NULL, NULL, NULL );
}

void db_load_bannedUsers(void)
{
    LogPrintNoArg(LOG_CRIT, "Loading banned usernames from SQL");
    db_queue_query( 12, QueryTable, NULL, 0, result_load_bannedUsers, NULL,
                    NULL );
}

void db_load_socials(void)
{
    LogPrintNoArg(LOG_CRIT, "Loading social messages from SQL");

    db_queue_query( 13, QueryTable, NULL, 0, NULL, NULL, NULL );
}

void db_load_kick_messages(void)
{
    MYSQL_BIND         *data;

    LogPrintNoArg(LOG_CRIT, "Loading kick messages from SQL");

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], 4, MYSQL_TYPE_LONG );

    db_queue_query( 11, QueryTable, data, 1, NULL, NULL, NULL );
}

void db_load_poses(void)
{
    LogPrintNoArg(LOG_CRIT, "Loading pose messages from SQL");

    db_queue_query( 14, QueryTable, NULL, 0, NULL, NULL, NULL );
}

/*
 * assign special procedures to mobiles 
 */
void assign_mobiles( void )
{
    MYSQL_BIND         *data;

    LogPrintNoArg(LOG_CRIT, "Loading mobile procs from SQL");

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], PROC_MOBILE, MYSQL_TYPE_LONG );

    db_queue_query( 15, QueryTable, data, 1, NULL, NULL, NULL );
}

/*
 * assign special procedures to objects 
 */
void assign_objects( void )
{
    MYSQL_BIND         *data;

    LogPrintNoArg(LOG_CRIT, "Loading object procs from SQL");

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], PROC_OBJECT, MYSQL_TYPE_LONG );

    db_queue_query( 15, QueryTable, data, 1, NULL, NULL, NULL );
}

/*
 * assign special procedures to rooms 
 */
void assign_rooms( void )
{
    MYSQL_BIND         *data;

    LogPrintNoArg(LOG_CRIT, "Loading room procs from SQL");

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], PROC_ROOM, MYSQL_TYPE_LONG );

    db_queue_query( 15, QueryTable, data, 1, NULL, NULL, NULL );
}


void db_load_races(void) 
{
    LogPrintNoArg(LOG_CRIT, "Loading races[] from SQL");

    db_queue_query( 16, QueryTable, NULL, 0, NULL, NULL, NULL );
}


void db_load_languages(void) 
{
    LogPrintNoArg(LOG_CRIT, "Loading languages[] from SQL");

    db_queue_query( 18, QueryTable, NULL, 0, NULL, NULL, NULL );
}

typedef struct {
    int         id;
    char      **buffer;
    char       *descr;
} TextFiles_t;

static TextFiles_t textfile[] = {
    { 1, &credits,  "credits" },
    { 2, &info,     "info" },
    { 3, &login,    "login" },
    { 4, &motd,     "motd" },
    { 5, &news,     "news" },
    { 6, &wmotd,    "wmotd" }
};
static int textfileCount = NELEMENTS(textfile);

void db_load_textfiles(void)
{
    int                 i;
    TextFiles_t        *file;

    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    LogPrintNoArg(LOG_CRIT, "Loading Essential Text Files.");

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    for( i = 0; i < textfileCount; i++ ) {
        file = &textfile[i];

        /* Load up the defined file */
        LogPrint( LOG_CRIT, "Loading '%s' file", file->descr );

        data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
        memset( data, 0, 2 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], file->id, MYSQL_TYPE_LONG );
        bind_null_blob( &data[1], (void *)file->buffer );

        db_queue_query( 19, QueryTable, data, 2, NULL, NULL, mutex );
        pthread_mutex_unlock( mutex );
    }

    pthread_mutex_destroy( mutex );
    free( mutex );
}

int db_save_textfile(struct char_data *ch)
{
    char                   *outbuf;
    struct edit_txt_msg    *tfd;
    int                     fileId;
    PlayerStruct_t         *player;
    MYSQL_BIND             *data;
    pthread_mutex_t        *mutex;


    if (!ch) {
        return (FALSE);
    }
    tfd = ch->specials.txtedit;

    if (!tfd) {
        return (FALSE);
    }

    player = (PlayerStruct_t *)ch->playerDesc;

    switch (tfd->file) {
    case 1:
        /* News */
        fileId = 5;
        break;
    case 2:
        /* MOTD */
        fileId = 4;
        break;
    case 3:
        /* WMOTD */
        fileId = 6;
        break;
    default:
        SendOutput( player, "Cannot save this file type.\n\r" );
        return (FALSE);
        break;
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    outbuf = (char *)malloc(MAX_STRING_LENGTH+2);
    memset( outbuf, 0, MAX_STRING_LENGTH+2 );

    strcpy(outbuf, "\n");
    if (tfd->date) {
        strcat(outbuf, tfd->date);
        
        while(outbuf[strlen(outbuf) - 1] == '~') {
            outbuf[strlen(outbuf) - 1] = '\0';
        }
        strcat(outbuf, "\n");
    }

    if (tfd->body) {
        strcat(outbuf, "\n");
        remove_cr(&outbuf[strlen(outbuf)], tfd->body);
        while (outbuf[strlen(outbuf) - 1] == '~') {
            outbuf[strlen(outbuf) - 1] = '\0';
        }
    }

    if (tfd->author) {
        strcat(outbuf, "\nLast edited by ");
        strcat(outbuf, tfd->author);
        strcat(outbuf, ".");
    }

    strcat(outbuf, "\n");

    /* Delete old file */
    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], fileId, MYSQL_TYPE_LONG );
    db_queue_query( 20, QueryTable, data, 1, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    /* Insert new file */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], fileId, MYSQL_TYPE_LONG );
    bind_string( &data[1], tfd->author, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[2], outbuf, MYSQL_TYPE_STRING );
    db_queue_query( 21, QueryTable, data, 3, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
    free( outbuf );

    return (TRUE);
}


/*
 *  Recursively delete a post and all it's replies.
 */
void db_delete_board_message(struct board_def *board, short message_id)
{
    MYSQL_BIND             *data;
    pthread_mutex_t        *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );


    /*
     * First check to see if we have a reply to this message -- if so
     * axe it (done in the chain routine)
     */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], board->boardId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], message_id, MYSQL_TYPE_LONG );
    bind_null_blob( &data[2], board );
    db_queue_query( 22, QueryTable, data, 3, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    /* delete the message */
    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], message_id, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], board->boardId, MYSQL_TYPE_LONG );
    db_queue_query( 23, QueryTable, data, 2, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    /* Renumber any following messages */
    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], message_id, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], board->boardId, MYSQL_TYPE_LONG );
    db_queue_query( 24, QueryTable, data, 2, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    /* Renumber max posts for the board */
    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], board->boardId, MYSQL_TYPE_LONG );
    db_queue_query( 25, QueryTable, data, 1, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    pthread_mutex_destroy( mutex );
    free( mutex );
}

struct board_def *db_lookup_board(int vnum)
{
    struct board_def   *board;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    db_queue_query( 26, QueryTable, data, 1, result_lookup_board, 
                    (void *)&board, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( board );
}

struct bulletin_board_message *db_get_board_message(int boardId, int msgNum)
{
    struct bulletin_board_message *msg;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], boardId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], msgNum, MYSQL_TYPE_LONG );
    db_queue_query( 27, QueryTable, data, 2, result_get_board_message, 
                    (void *)&msg, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( msg );
}

void db_free_board_message(struct bulletin_board_message *msg)
{
    if( !msg ) {
        return;
    }

    if( msg->author ) {
        free( msg->author );
    }

    if( msg->title ) {
        free( msg->title );
    }

    if( msg->text ) {
        free( msg->text );
    }

    free( msg );
}

typedef struct {
    struct bulletin_board_message **msg;
    int                             count;
} BoardRepliesArgs_t;

int db_get_board_replies(struct board_def *board, int msgId, 
                         struct bulletin_board_message **msg)
{
    BoardRepliesArgs_t  args;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    args.msg = msg;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], board->boardId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], msgId, MYSQL_TYPE_LONG );
    db_queue_query( 28, QueryTable, data, 2, result_get_board_message, 
                    (void *)&args, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( args.count );
}

void db_free_board_replies(struct bulletin_board_message *msg, int count)
{
    int             i;

    if( !msg ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        if( msg[i].author ) {
            free( msg[i].author );
        }

        if( msg[i].title ) {
            free( msg[i].title );
        }

        if( msg[i].text ) {
            free( msg[i].text );
        }
    }

    free( msg );
}

void db_post_message(struct board_def *board, 
                     struct bulletin_board_message *msg)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    /* Write the post */
    data = (MYSQL_BIND *)malloc(6 * sizeof(MYSQL_BIND));
    memset( data, 0, 6 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], board->boardId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], board->messageCount + 1, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], msg->reply_to, MYSQL_TYPE_LONG );
    bind_string( &data[3], msg->title, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[4], msg->author, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[5], msg->text, MYSQL_TYPE_STRING );
    db_queue_query( 29, QueryTable, data, 6, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    /* update the board's max post */
    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], board->boardId, MYSQL_TYPE_LONG );
    db_queue_query( 30, QueryTable, data, 1, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    pthread_mutex_destroy( mutex );
    free( mutex );
}

void db_store_mail(char *to, char *from, char *message_pointer)
{
    MYSQL_BIND         *data;

    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_string( &data[0], from, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[1], to, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[2], message_pointer, MYSQL_TYPE_STRING );
    db_queue_query( 31, QueryTable, data, 3, NULL, NULL, NULL );
}

int             db_has_mail(char *recipient)
{
    int                 count;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_string( &data[0], recipient, MYSQL_TYPE_VAR_STRING );
    db_queue_query( 32, QueryTable, data, 1, result_has_mail, (void *)&count, 
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( count );
}

typedef struct {
    int         *msgNum;
    int         count;
} MailIdsArgs_t;

int db_get_mail_ids(char *recipient, int *messageNum, int count)
{
    MailIdsArgs_t       args;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    args.msgNum = messageNum;
    args.count  = count;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_string( &data[0], recipient, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[1], count, MYSQL_TYPE_LONG );
    db_queue_query( 33, QueryTable, data, 2, result_get_mail_ids, (void *)&args,
                    mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( args.count );
}


char *db_get_mail_message(int messageId)
{
    char               *msg;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], messageId, MYSQL_TYPE_LONG );
    db_queue_query( 34, QueryTable, data, 1, result_get_mail_message, 
                    (void *)&msg, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( msg );
}

void db_delete_mail_message(int messageId)
{
    MYSQL_BIND         *data;

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], messageId, MYSQL_TYPE_LONG );
    db_queue_query( 35, QueryTable, data, 1, NULL, NULL, NULL );
}

int CheckKillFile(long virtual)
{
    int                 count;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], virtual, MYSQL_TYPE_LONG );
    db_queue_query( 36, QueryTable, data, 1, result_check_kill_file, 
                    (void *)&count, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( count );
}


char *db_lookup_help( int type, char *keywords )
{
    char               *msg;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], type, MYSQL_TYPE_LONG );
    bind_string( &data[1], keywords, MYSQL_TYPE_VAR_STRING );
    db_queue_query( 37, QueryTable, data, 2, result_lookup_help, 
                    (void *)&msg, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( msg );
}

typedef struct {
    char           *msg;
    char           *keywords;
} LookupHelpArgs_t;

char *db_lookup_help_similar( int type, char *keywords )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    LookupHelpArgs_t    args;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_string( &data[0], keywords, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[1], type, MYSQL_TYPE_LONG );
    bind_string( &data[2], keywords, MYSQL_TYPE_VAR_STRING );
    db_queue_query( 38, QueryTable, data, 3, result_lookup_help_similar, 
                    (void *)&args, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    return( args.msg );
}

void db_save_object(struct obj_data *obj, int owner, int room, int ownerItem, 
                    int parentItem)
{
    char           *actDesc;
    int             i,
                    j;
    Keywords_t     *descr;
    int             vnum;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    char               *temp;
    unsigned int       *var;
    int                 value;
    int                 seq;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );


    vnum      = obj->item_number;
    actDesc   = obj->action_description;

    /* Strip trailing \n\r from the action description */
    i = strlen(actDesc) - 1;
    while( i > 0 &&
           (actDesc[i] == '\n' || actDesc[i] == '\r') &&
           (actDesc[i-1] == '\n' || actDesc[i-1] == '\r') ) {
        actDesc[i--] = '\0';
    }

    if( i == 0 && (actDesc[0] == '\n' || actDesc[0] == '\r') ) {
        actDesc[0] = '\0';
    }

    /* Replace the object */
    data = (MYSQL_BIND *)malloc(24 * sizeof(MYSQL_BIND));
    memset( data, 0, 24 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    bind_string( &data[4], obj->short_description, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[5], obj->description, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[6], obj->action_description, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[7], obj->modBy, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[8], obj->type_flag, MYSQL_TYPE_LONG );
    bind_numeric( &data[9], obj->value[0], MYSQL_TYPE_LONG );
    bind_numeric( &data[10], obj->value[1], MYSQL_TYPE_LONG );
    bind_numeric( &data[11], obj->value[2], MYSQL_TYPE_LONG );
    bind_numeric( &data[12], obj->value[3], MYSQL_TYPE_LONG );
    bind_numeric( &data[13], obj->weight, MYSQL_TYPE_LONG );
    bind_numeric( &data[14], obj->cost, MYSQL_TYPE_LONG );
    bind_numeric( &data[15], obj->cost_per_day, MYSQL_TYPE_LONG );
    bind_numeric( &data[16], obj->level, MYSQL_TYPE_LONG );
    bind_numeric( &data[17], obj->max, MYSQL_TYPE_LONG );
    bind_numeric( &data[18], (long)obj->modified, MYSQL_TYPE_LONG );
    bind_numeric( &data[19], (IS_WEAPON(obj)? obj->speed : 0), 
                  MYSQL_TYPE_LONG );
    bind_numeric( &data[20], (IS_WEAPON(obj) ? obj->weapontype + 1 : 0),
                  MYSQL_TYPE_LONG );
    bind_numeric( &data[21], obj->tweak, MYSQL_TYPE_LONG );
    bind_numeric( &data[22], obj->eq_pos, MYSQL_TYPE_LONG );
    bind_numeric( &data[23], parentItem, MYSQL_TYPE_LONG );
    db_queue_query( 39, QueryTable, data, 24, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    for( i = 0; i < obj->keywords.count; i++ ) {
        /* Update the keywords */
        data = (MYSQL_BIND *)malloc(6 * sizeof(MYSQL_BIND));
        memset( data, 0, 6 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        bind_numeric( &data[4], i, MYSQL_TYPE_LONG );
        bind_string( &data[5], obj->keywords.words[i], MYSQL_TYPE_VAR_STRING );
        db_queue_query( 40, QueryTable, data, 6, NULL, NULL, mutex );
        pthread_mutex_unlock( mutex );
    }

    /* Remove any unused keywords */
    data = (MYSQL_BIND *)malloc(5 * sizeof(MYSQL_BIND));
    memset( data, 0, 5 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    bind_numeric( &data[4], i, MYSQL_TYPE_LONG );
    db_queue_query( 41, QueryTable, data, 5, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );


    seq = 1;
    for( i = 0; i < obj_flag_count; i++ ) {
        if( obj_flags[i].count == 0 ) {
            continue;
        }

        var = (unsigned int *)PTR_AT_OFFSET(obj_flags[i].offset, obj);

        for( j = 0; j < obj_flags[i].count; j++ ) {
            value = (IS_SET(*var, obj_flags[i].bits[j].set) && 
                     !IS_SET(*var, obj_flags[i].bits[j].clear)) ? 1 : 0;

            if( value ) {
                /* Update this flag */
                data = (MYSQL_BIND *)malloc(8 * sizeof(MYSQL_BIND));
                memset( data, 0, 8 * sizeof(MYSQL_BIND) );

                bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
                bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
                bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
                bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
                bind_numeric( &data[4], seq, MYSQL_TYPE_LONG );
                bind_numeric( &data[5], i, MYSQL_TYPE_LONG );
                bind_numeric( &data[6], j, MYSQL_TYPE_LONG );
                bind_numeric( &data[7], value, MYSQL_TYPE_LONG );

                db_queue_query( 42, QueryTable, data, 8, NULL, NULL, mutex );
                pthread_mutex_unlock( mutex );
                seq++;
            }
        }
    }

    /* Remove any unused flags */
    data = (MYSQL_BIND *)malloc(5 * sizeof(MYSQL_BIND));
    memset( data, 0, 5 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    bind_numeric( &data[4], seq, MYSQL_TYPE_LONG );
    db_queue_query( 54, QueryTable, data, 5, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );


    for (descr = obj->ex_description, i = 0; 
         i < obj->ex_description_count; descr++, i++) {
        /* Update extra descriptions */
        data = (MYSQL_BIND *)malloc(7 * sizeof(MYSQL_BIND));
        memset( data, 0, 7 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        bind_numeric( &data[4], i, MYSQL_TYPE_LONG );
        temp = KeywordsToString( descr, " " );
        bind_string( &data[5], temp, MYSQL_TYPE_VAR_STRING );
        bind_string( &data[6], descr->description, MYSQL_TYPE_VAR_STRING );
        db_queue_query( 43, QueryTable, data, 7, NULL, NULL, mutex );
        free( temp );
        pthread_mutex_unlock( mutex );
    }

    /* remove unused extra descriptions */
    data = (MYSQL_BIND *)malloc(5 * sizeof(MYSQL_BIND));
    memset( data, 0, 5 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    bind_numeric( &data[4], i, MYSQL_TYPE_LONG );
    db_queue_query( 44, QueryTable, data, 5, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    for (i = 0, j = 0; i < MAX_OBJ_AFFECT; i++) {
        if (obj->affected[i].location != APPLY_NONE) {
            /* update affects */
            data = (MYSQL_BIND *)malloc(7 * sizeof(MYSQL_BIND));
            memset( data, 0, 7 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
            bind_numeric( &data[4], j, MYSQL_TYPE_LONG );
            bind_numeric( &data[5], obj->affected[i].location, 
                          MYSQL_TYPE_LONG );
            bind_numeric( &data[6], obj->affected[i].modifier, 
                          MYSQL_TYPE_LONG );
            db_queue_query( 45, QueryTable, data, 7, NULL, NULL, mutex );
            pthread_mutex_unlock( mutex );
            j++;
        }
    }

    /* remove unused affects */
    data = (MYSQL_BIND *)malloc(5 * sizeof(MYSQL_BIND));
    memset( data, 0, 5 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    bind_numeric( &data[4], j, MYSQL_TYPE_LONG );
    db_queue_query( 46, QueryTable, data, 5, NULL, NULL, mutex );
    pthread_mutex_unlock( mutex );

    pthread_mutex_destroy( mutex );
    free( mutex );
}


struct obj_data *db_read_object(struct obj_data *obj, int vnum, int owner,
                                int room, int ownerItem )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    obj->index = objectIndex( vnum );

    /* load the object */
    data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
    memset( data, 0, 4 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
    db_queue_query( 47, QueryTable, data, 4, result_read_object_1, 
                    (void *)&obj, mutex );
    pthread_mutex_unlock( mutex );

    if( obj ) {
        /* load the keywords */
        data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
        memset( data, 0, 4 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        db_queue_query( 48, QueryTable, data, 4, result_read_object_2, 
                        (void *)&obj, mutex );
        pthread_mutex_unlock( mutex );
    }

    if( obj ) {
        /* load the flags */
        data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
        memset( data, 0, 4 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        db_queue_query( 49, QueryTable, data, 4, result_read_object_3, 
                        (void *)&obj, mutex );
        pthread_mutex_unlock( mutex );
    }
    
    if( obj ) {
        /* load extra descriptions */
        data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
        memset( data, 0, 4 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        db_queue_query( 50, QueryTable, data, 4, result_read_object_4, 
                        (void *)&obj, mutex );
        pthread_mutex_unlock( mutex );
    }

    if( obj ) {
        /* load affects */
        data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
        memset( data, 0, 4 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], owner, MYSQL_TYPE_LONG );
        bind_numeric( &data[2], room, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ownerItem, MYSQL_TYPE_LONG );
        db_queue_query( 51, QueryTable, data, 4, result_read_object_5, 
                        (void *)&obj, mutex );
        pthread_mutex_unlock( mutex );
    }

    pthread_mutex_destroy( mutex );
    free( mutex );

    return(obj);
}

struct keyword_list;
struct keyword_list {
    struct keyword_list *next;
    char *keyword;
};

QueryTable_t    QueryKeywords[] = {
    /* 0 */
    { "SELECT a1.vnum "
      "FROM `objectKeywords` as a1 "
      "WHERE a1.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE },
    /* 1 */
    { "SELECT a1.vnum FROM `objectKeywords` as a1, `objectKeywords` as a2 "
      "WHERE a2.vnum = a1.vnum AND "
      "a1.keyword REGEXP ? AND a2.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE },
    /* 2 */
    { "SELECT a1.vnum from `objectKeywords` as a1, `objectKeywords` as a2, "
      "`objectKeywords` as a3 "
      "WHERE a2.vnum = a1.vnum AND a3.vnum = a1.vnum AND "
      "a1.keyword REGEXP ? AND a2.keyword REGEXP ? AND a3.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE },
    /* 3 */
    { "SELECT a1.vnum from `objectKeywords` as a1, `objectKeywords` as a2, "
      "`objectKeywords` as a3, `objectKeywords` as a4 "
      "WHERE a2.vnum = a1.vnum AND a3.vnum = a1.vnum AND a4.vnum = a1.vnum AND "
      "a1.keyword REGEXP ? AND a2.keyword REGEXP ? AND a3.keyword REGEXP ? AND "
      "a4.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE },
    /* 4 */
    { "SELECT a1.vnum from `objectKeywords` as a1, `objectKeywords` as a2, "
      "`objectKeywords` as a3, `objectKeywords` as a4, `objectKeywords` as a5 "
      "WHERE a2.vnum = a1.vnum AND a3.vnum = a1.vnum AND a4.vnum = a1.vnum AND "
      "a5.vnum = a1.vnum AND "
      "a1.keyword REGEXP ? AND a2.keyword REGEXP ? AND a3.keyword REGEXP ? AND "
      "a4.keyword REGEXP ? AND a5.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE },
    /* 5 */
    { "SELECT a1.vnum from `objectKeywords` as a1, `objectKeywords` as a2, "
      "`objectKeywords` as a3, `objectKeywords` as a4, `objectKeywords` as a5, "
      "`objectKeywords` as a6 "
      "WHERE a2.vnum = a1.vnum AND a3.vnum = a1.vnum AND a4.vnum = a1.vnum AND "
      "a5.vnum = a1.vnum AND a6.vnum = a1.vnum AND "
      "a1.keyword REGEXP ? AND a2.keyword REGEXP ? AND a3.keyword REGEXP ? AND "
      "a4.keyword REGEXP ? AND a5.keyword REGEXP ? AND a6.keyword REGEXP ? "
      "LIMIT 1", NULL, NULL, FALSE }
};
static int maxKeywords = NELEMENTS( QueryKeywords );

int db_find_object_named(char *string, int owner, int ownerItem)
{
    int             count;
    int             i;
    char           *keyword;
    struct keyword_list *keywordList;
    struct keyword_list *prev;
    struct keyword_list *curr;
    int             vnum;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );


    keywordList = NULL;
    prev = NULL;
    count = 0;

    while( ( keyword = strsep( &string, "- \t\n\r" ) ) ) {
        curr = (struct keyword_list *)malloc(sizeof(struct keyword_list));
        curr->next = NULL;
        curr->keyword = (char *)malloc(strlen(keyword)+2);
        strcpy( curr->keyword, "^" );
        strcat( curr->keyword, keyword );

        if( !prev ) {
            keywordList = curr;
        } else {
            prev->next = curr;
        }
        prev = curr;
        count++;
    }

    /* search for the keywords, up to a maximum number of them */
    if( count > maxKeywords ) {
        LogPrint( LOG_CRIT, "User entered %d keywords, searching on only "
                            "first %d keywords", count, maxKeywords );
        count = maxKeywords;
    }
    vnum = -1;

    data = (MYSQL_BIND *)malloc(count * sizeof(MYSQL_BIND));
    memset( data, 0, count * sizeof(MYSQL_BIND) );

    for( i = 0, curr = keywordList; i < count; i++, curr = curr->next ) {
        bind_string( &data[i], curr->keyword, MYSQL_TYPE_VAR_STRING );
    }
    db_queue_query( count - 1, QueryKeywords, data, count, 
                    result_find_object_named, (void *)&vnum, mutex );
    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );

    for( curr = keywordList; curr; curr = prev ) {
        prev = curr->next;
        free( curr->keyword );
        free( curr );
    }

    return( vnum );
}


void db_load_object_tree( BalancedBTree_t *tree )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_null_blob( &data[0], (void *)tree );
    db_queue_query( 52, QueryTable, data, 1, NULL, NULL, mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
}

void db_update_char_rent( int ownerId, int gold, int rentCost, int minStay )
{
    MYSQL_BIND         *data;

    data = (MYSQL_BIND *)malloc(5 * sizeof(MYSQL_BIND));
    memset( data, 0, 5 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], gold, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], rentCost, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], minStay, MYSQL_TYPE_LONG );
    bind_numeric( &data[3], time(0), MYSQL_TYPE_LONG );
    bind_numeric( &data[4], ownerId, MYSQL_TYPE_LONG );
    db_queue_query( 55, QueryTable, data, 5, NULL, NULL, NULL );
}

void db_clear_objects( int ownerId, int room, int itemNum )
{
    MYSQL_BIND         *data;

    /* Clear the objects */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], itemNum, MYSQL_TYPE_LONG );
    db_queue_query( 56, QueryTable, data, 3, NULL, NULL, NULL );
    
    /* Clear the affects */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], itemNum, MYSQL_TYPE_LONG );
    db_queue_query( 63, QueryTable, data, 3, NULL, NULL, NULL );
    
    /* Clear the extra descriptions */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], itemNum, MYSQL_TYPE_LONG );
    db_queue_query( 64, QueryTable, data, 3, NULL, NULL, NULL );
    
    /* Clear the flags */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], itemNum, MYSQL_TYPE_LONG );
    db_queue_query( 65, QueryTable, data, 3, NULL, NULL, NULL );
    
    /* Clear the keywords */
    data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
    memset( data, 0, 3 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
    bind_numeric( &data[2], itemNum, MYSQL_TYPE_LONG );
    db_queue_query( 66, QueryTable, data, 3, NULL, NULL, NULL );
}

void db_write_char_extra(struct char_data *ch)
{
    MYSQL_BIND     *data;
    int             i;
    int             seq;

    data = (MYSQL_BIND *)malloc(12 * sizeof(MYSQL_BIND));
    memset( data, 0, 12 * sizeof(MYSQL_BIND) );

    if (IS_IMMORTAL(ch)) {
        bind_string( &data[0], ch->specials.poofin, MYSQL_TYPE_VAR_STRING );
        bind_string( &data[1], ch->specials.poofout, MYSQL_TYPE_VAR_STRING );
        bind_numeric( &data[2], ch->specials.sev, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], ch->invis_level, MYSQL_TYPE_LONG );
        bind_numeric( &data[4], GET_ZONE(ch), MYSQL_TYPE_LONG );
    } else {
        bind_string( &data[0], NULL, MYSQL_TYPE_VAR_STRING );
        bind_string( &data[1], NULL, MYSQL_TYPE_VAR_STRING );
        bind_numeric( &data[2], 0, MYSQL_TYPE_LONG );
        bind_numeric( &data[3], 0, MYSQL_TYPE_LONG );
        bind_numeric( &data[4], 0, MYSQL_TYPE_LONG );
    }

    bind_string( &data[5], ch->specials.prompt, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[6], ch->specials.bprompt, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[7], ch->specials.email, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[8], ch->specials.clan, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[9], ch->specials.hostip, MYSQL_TYPE_VAR_STRING );
    bind_string( &data[10], ch->specials.rumor, MYSQL_TYPE_VAR_STRING );
    bind_numeric( &data[11], ch->playerId, MYSQL_TYPE_LONG );
    db_queue_query( 57, QueryTable, data, 12, NULL, NULL, NULL );

    seq = 1;
    if (ch->specials.A_list) {
        for (i = 0; i < 10; i++) {
            if (GET_ALIAS(ch, i)) {
                data = (MYSQL_BIND *)malloc(4 * sizeof(MYSQL_BIND));
                memset( data, 0, 4 * sizeof(MYSQL_BIND) );

                bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
                bind_numeric( &data[1], seq, MYSQL_TYPE_LONG );
                bind_numeric( &data[2], i, MYSQL_TYPE_LONG );
                bind_string( &data[3], GET_ALIAS(ch, i), MYSQL_TYPE_VAR_STRING);
                db_queue_query( 58, QueryTable, data, 4, NULL, NULL, NULL );
                seq++;
            }
        }
    }

    /* Clean up remaining aliases */
    data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
    memset( data, 0, 2 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
    bind_numeric( &data[1], seq, MYSQL_TYPE_LONG );
    db_queue_query( 59, QueryTable, data, 2, NULL, NULL, NULL );
}

void db_load_char_extra(struct char_data *ch)
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );
    bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
    db_queue_query( 60, QueryTable, data, 1, result_load_char_extra_1, ch, 
                    mutex );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );
    bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
    db_queue_query( 61, QueryTable, data, 1, result_load_char_extra_2, ch, 
                    mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
}

typedef struct {
    int        *gold;
    int        *rentCost;
    int        *minStay;
    int        *lastUpdate;
} GetCharRentArgs_t;

void db_get_char_rent( int ownerId, int *gold, int *rentCost, int *minStay,
                       int *lastUpdate )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    GetCharRentArgs_t   args;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_numeric( &data[0], ownerId, MYSQL_TYPE_LONG );

    args.gold       = gold;
    args.rentCost   = rentCost;
    args.minStay    = minStay;
    args.lastUpdate = lastUpdate;

    db_queue_query( 62, QueryTable, data, 1, result_get_char_rent, &args, 
                    mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
}

struct _LoadCharObjsArgs_t;
typedef struct _LoadCharObjsArgs_t {
    int    *parentNum;
    struct obj_data **parentObj;
    int     count;
    int     i;
    struct _LoadCharObjsArgs_t *prev;
    struct char_data *ch;
    int                 room;
} LoadCharObjsArgs_t;

void db_load_char_objects( struct char_data *ch )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    volatile LoadCharObjsArgs_t *args;
    LoadCharObjsArgs_t *keep;
    LoadCharObjsArgs_t *prev;
    struct obj_data    *obj;
    int                 num;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    CREATE(args, LoadCharObjsArgs_t, 1);
    CREATE(args->parentNum, int, 1);
    CREATE(args->parentObj, struct obj_data *, 1);
    args->count  = 1;
    args->i = 0;
    args->parentNum[0] = -1;
    args->parentObj[0] = NULL;
    args->prev = NULL;
    args->ch = ch;

    while( args ) {
        for( ; args->i < args->count; args->i++ ) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], -1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], args->parentNum[args->i], MYSQL_TYPE_LONG );

            /*
             * Note, this will actually modify args by creating a new one, and
             * pushing it onto the stack, keep the orig args so we can finish
             * loading it before "recursing" into items
             */
            keep = (LoadCharObjsArgs_t *)args;
            db_queue_query( 67, QueryTable, data, 3, 
                            result_load_char_objects_1, &args, mutex );
            pthread_mutex_unlock( mutex );

            num = keep->parentNum[keep->i];
            obj = keep->parentObj[keep->i];

            if( num == -1 ) {
                /* 
                 * There is no "null" object to load the rest of the data for 
                 */
                continue;
            }

            /* Get the Keywords */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], -1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 68, QueryTable, data, 3, result_read_object_2, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Flags */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], -1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 69, QueryTable, data, 3, result_read_object_3, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Extra Descriptions */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], -1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 70, QueryTable, data, 3, result_read_object_4, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Affects */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], ch->playerId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], -1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 71, QueryTable, data, 3, result_read_object_5, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            if( IS_RARE(obj) ) {
                obj->index->number--;
            }
        }

        /*
         * We finished that bunch, pop the stack and delete this one
         */
        prev = args->prev;
        free( args->parentNum );
        free( args->parentObj );
        free( (void *)args );
        args = prev;
    }

    pthread_mutex_destroy( mutex );
    free( mutex );
}

void db_load_room_objects( int room )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    volatile LoadCharObjsArgs_t *args;
    LoadCharObjsArgs_t *keep;
    LoadCharObjsArgs_t *prev;
    struct obj_data    *obj;
    int                 num;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    CREATE(args, LoadCharObjsArgs_t, 1);
    CREATE(args->parentNum, int, 1);
    CREATE(args->parentObj, struct obj_data *, 1);
    args->count  = 1;
    args->i = 0;
    args->parentNum[0] = -1;
    args->parentObj[0] = NULL;
    args->prev = NULL;
    args->room = room;

    while( args ) {
        for( ; args->i < args->count; args->i++ ) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], 0, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], args->parentNum[args->i], MYSQL_TYPE_LONG );

            /*
             * Note, this will actually modify args by creating a new one, and
             * pushing it onto the stack, keep the orig args so we can finish
             * loading it before "recursing" into items
             */
            keep = (LoadCharObjsArgs_t *)args;
            db_queue_query( 67, QueryTable, data, 3, 
                            result_load_room_objects_1, &args, mutex );
            pthread_mutex_unlock( mutex );

            num = keep->parentNum[keep->i];
            obj = keep->parentObj[keep->i];

            if( num == -1 ) {
                /* 
                 * There is no "null" object to load the rest of the data for 
                 */
                continue;
            }

            /* Get the Keywords */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], 0, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 68, QueryTable, data, 3, result_read_object_2, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Flags */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], 0, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 69, QueryTable, data, 3, result_read_object_3, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Extra Descriptions */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], 0, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 70, QueryTable, data, 3, result_read_object_4, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            /* Get the Affects */
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], 0, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], room, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], num, MYSQL_TYPE_LONG );
            db_queue_query( 71, QueryTable, data, 3, result_read_object_5, 
                            &obj, mutex );
            pthread_mutex_unlock( mutex );

            if( IS_RARE(obj) ) {
                obj->index->number--;
            }
        }

        /*
         * We finished that bunch, pop the stack and delete this one
         */
        prev = args->prev;
        free( args->parentNum );
        free( args->parentObj );
        free( (void *)args );
        args = prev;
    }

    pthread_mutex_destroy( mutex );
    free( mutex );
}

void db_load_rooms( BalancedBTree_t *tree )
{
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
    memset( data, 0, 1 * sizeof(MYSQL_BIND) );

    bind_null_blob( &data[0], (void *)tree );
    db_queue_query( 72, QueryTable, data, 1, NULL, NULL, mutex );

    pthread_mutex_unlock( mutex );
    pthread_mutex_destroy( mutex );
    free( mutex );
}

/*
 * Query chaining functions
 */

void chain_update_nick( MYSQL_RES *res, QueryItem_t *item )
{
    int             count;
    MYSQL_BIND     *data;
    MYSQL_BIND      temp[2];

    data = item->queryData;

    if( !res || !(count = mysql_num_rows(res)) ) {
        count = 0;
    }

    memcpy( temp, &data[2], 2 * sizeof(MYSQL_BIND) );
    memcpy( &data[2], &data[0], 2 * sizeof(MYSQL_BIND) );
    memcpy( &data[0], temp, 2 * sizeof(MYSQL_BIND) );

    if( count ) {
        /* update */
        db_queue_query( 4, QueryTable, data, 4, NULL, NULL, NULL );
    } else {
        /* insert */
        db_queue_query( 5, QueryTable, data, 4, NULL, NULL, NULL );
    }
}


void chain_load_classes( MYSQL_RES *res, QueryItem_t *item )
{
    int                 count;
    int                 classId;
    int                 i;
    MYSQL_ROW           row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(count = mysql_num_rows(res))) {
        /* No classes!? */
        LogPrintNoArg( LOG_CRIT, "No classes defined in the database!" );
        exit(1);
    }

    classCount = count;
    classes = (struct class_def *)malloc(classCount * sizeof(struct class_def));
    if( !classes ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating classes[]" );
        exit(1);
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        classId = atoi(row[0]);
        classes[i].name = strdup(row[1]);
        classes[i].abbrev = strdup(row[2]);

        /* 4, classId, 0 */
        data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
        memset( data, 0, 2 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], classId, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], 0, MYSQL_TYPE_LONG );
        db_queue_query( 4, QueryTable, data, 2, result_load_classes_1, 
                        (void *)&classes[i], mutex );

        pthread_mutex_unlock( mutex );

        /* 4, classId, 1 */
        data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
        memset( data, 0, 2 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], classId, MYSQL_TYPE_LONG );
        bind_numeric( &data[1], 1, MYSQL_TYPE_LONG );
        db_queue_query( 4, QueryTable, data, 2, result_load_classes_2, 
                        (void *)&classes[i], mutex );

        pthread_mutex_unlock( mutex );

        /* 5 */
        data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
        memset( data, 0, 1 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], classId, MYSQL_TYPE_LONG );
        db_queue_query( 4, QueryTable, data, 1, result_load_classes_3, 
                        (void *)&i, mutex );

        pthread_mutex_unlock( mutex );
    }
    
    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finished loading classes[] from SQL");
}

void chain_load_skills( MYSQL_RES *res, QueryItem_t *item )
{
    int                 count;
    int                 i,
                        j;
    int                 skillId;
    MYSQL_ROW           row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(count = mysql_num_rows(res)) ) {
        /* No skills!? */
        LogPrintNoArg( LOG_CRIT, "No skills defined in the database!" );
        exit(1);
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    skills = (struct skill_def *)malloc((count + 1) * sizeof(struct skill_def));
    if( !skills ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating skills[]" );
        exit(1);
    }

    /* Leave skill 0 emtpy */
    memset( skills, 0, (count + 1) * sizeof(struct skill_def) );

    for( i = 1; i <= count; i++ ) {
        row = mysql_fetch_row(res);
        skillId = atoi(row[0]);
        skills[i].skillId   = skillId;
        skills[i].name      = strdup(row[1]);
        skills[i].skillType = atoi(row[2]);

        for(j = 0; j < SKILL_MSG_COUNT; j++) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], skillId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], 1, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 7, QueryTable, data, 3, result_load_messages,
                            (void *)skills[i].message[j], mutex );
            pthread_mutex_unlock( mutex );
        }
    }
    
    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finished loading skills[] from SQL");
}

void chain_load_messages( MYSQL_RES *res, QueryItem_t *item )
{
    int                 i,
                        j,
                        tmp;
    int                 count;
    int                 count2;
    int                 type;

    MYSQL_ROW           row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;
    struct message_list *msgs;

    static char        *types[] = { "", "", "fight", "", "kick" };

    type = *(int *)item->queryData[0].buffer;

    if( !res || !(count = mysql_num_rows(res)) ) {
        LogPrint( LOG_CRIT, "No %s messages defined in the database!",
                  types[type] );
        exit(1);
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    msgs = (struct message_list *)malloc(count * sizeof(struct message_list));
    if( !msgs ) {
        LogPrint( LOG_CRIT, "Out of memory allocating %sMessages[]", 
                            types[type] );
        exit(1);
    }

    if( type == 2 ) {
        fightMessages = msgs;
        fightMessageCount = count;
        count2 = FIGHT_MSG_COUNT;
    } else {
        kickMessages = msgs;
        kickMessageCount = count;
        count2 = KICK_MSG_COUNT;
    }

    memset( msgs, 0, count * sizeof(struct message_list) );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);
        if( type == 2 ) {
            msgs[i].a_type = tmp;
        }

        for( j = 0; j < count2; j++ ) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], tmp, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], type, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 7, QueryTable, data, 3, result_load_messages,
                            (void *)msgs[i].msg[j], mutex );
            pthread_mutex_unlock( mutex );
        }
    }
    
    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrint(LOG_CRIT, "Finished loading %s messages from SQL", types[type] );
}

void chain_load_socials( MYSQL_RES *res, QueryItem_t *item )
{
    int             i,
                    j;
    int             tmp;

    MYSQL_ROW       row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(socialMessageCount = mysql_num_rows(res)) ) {
        LogPrintNoArg( LOG_CRIT, "No social messages defined in the "
                                 "database!" );
        exit(1);
    }

    socialMessages = (struct social_messg *)malloc(socialMessageCount * 
                                                   sizeof(struct social_messg));
    if( !socialMessages ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating socialMessages[]" );
        exit(1);
    }

    memset( socialMessages, 0, 
            socialMessageCount * sizeof(struct social_messg) );

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );


    for (i = 0; i < socialMessageCount; i++) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);
        socialMessages[i].act_nr = tmp;
        socialMessages[i].hide = atoi(row[1]);
        socialMessages[i].min_victim_position = atoi(row[2]);

        for( j = 0; j < SOCIAL_MSG_COUNT; j++ ) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], tmp, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], 3, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 7, QueryTable, data, 3, result_load_messages,
                            (void *)socialMessages[i].msg[j], mutex );
            pthread_mutex_unlock( mutex );
        }
    }
    
    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finishing loading social messages from SQL");
}

void chain_load_poses( MYSQL_RES *res, QueryItem_t *item )
{
    int             i,
                    j;
    int             tmp;

    MYSQL_ROW       row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(poseMessageCount = mysql_num_rows(res)) ) {
        LogPrintNoArg(LOG_CRIT, "No pose messages defined in the database!" );
        exit(1);
    }

    poseMessages = (struct pose_type *)malloc(poseMessageCount * 
                                              sizeof(struct pose_type));
    if( !poseMessages ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating poseMessages[]" );
        exit(1);
    }

    memset( poseMessages, 0, poseMessageCount * sizeof(struct pose_type) );

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    for (i = 0; i < poseMessageCount; i++) {
        row = mysql_fetch_row(res);
        tmp = atoi(row[0]);
        poseMessages[i].level = tmp;

        poseMessages[i].poser_msg = (char **)malloc(classCount * sizeof(char*));
        poseMessages[i].room_msg  = (char **)malloc(classCount * sizeof(char*));
        if( !poseMessages[i].poser_msg || !poseMessages[i].room_msg ) {
            LogPrintNoArg(LOG_CRIT, "Out of memory allocating poses" );
            exit(1);
        }

        for( j = 0; j < classCount; j++ ) {
            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], tmp, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], 5, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 7, QueryTable, data, 3, result_load_messages,
                            (void *)poseMessages[i].poser_msg[j], mutex );
            pthread_mutex_unlock( mutex );


            data = (MYSQL_BIND *)malloc(3 * sizeof(MYSQL_BIND));
            memset( data, 0, 3 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], tmp, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], 6, MYSQL_TYPE_LONG );
            bind_numeric( &data[2], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 7, QueryTable, data, 3, result_load_messages,
                            (void *)poseMessages[i].room_msg[j], mutex );
            pthread_mutex_unlock( mutex );
        }
    }

    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finishing loading pose messages from SQL");
}

void chain_assign_specials( MYSQL_RES *res, QueryItem_t *item )
{
    static char        *desc[] = { "", "mobile", "object", "room" };
    int                 type;
    int                 i,
                        rnum,
                        vnum,
                        count;
    struct room_data   *rp = NULL;
    struct index_data  *index = NULL;
    int_func            func;

    MYSQL_ROW       row;

    type = *(int *)item->queryData[0].buffer;

    if( !res || !(count = mysql_num_rows(res)) ) {
        LogPrint(LOG_CRIT, "No %s procedures defined in the database!",
                           desc[type]);
        return;
    }

    for (i = 0; i < count; i++) {
        row = mysql_fetch_row(res);

        if( !(func = procGetFuncByName( row[1], type )) ) {
            LogPrint(LOG_CRIT, "assign_procs: proc for %s %s (%s) not "
                               "registered.", desc[type], row[0], row[1] );
            continue;
        }
        
        vnum = atoi(row[0]);
        switch( type ) {
        case PROC_MOBILE:
            index = mobileIndex(vnum);
            rnum = ( index ? 1 : -1 );
            break;
        case PROC_OBJECT:
            index = objectIndex(vnum);
            rnum = ( index ? 1 : -1 );
            break;
        case PROC_ROOM:
            rp = roomFindNum(vnum);
            rnum = ( rp ? 1 : -1 );
            break;
        default:
            LogPrint(LOG_CRIT, "Unknown special proc type %d!", type );
            return;
        }

        if (rnum < 0) {
            LogPrint(LOG_CRIT, "assign_procs: %s %d not found in database.", 
                               desc[type], vnum);
            continue;
        }

        switch( type ) {
        case PROC_MOBILE:
        case PROC_OBJECT:
            index->func = func;
            break;
        case PROC_ROOM:
            rp->func = func;
            break;
        default:
            return;
        }
    }

    LogPrint(LOG_CRIT, "Finished %s procs from SQL", desc[type]);
}


void chain_load_races( MYSQL_RES *res, QueryItem_t *item )
{
    int                 i,
                        j;

    MYSQL_ROW           row;
    MYSQL_BIND         *data;
    pthread_mutex_t    *mutex;

    if( !res || !(raceCount = mysql_num_rows(res)) ) {
        LogPrintNoArg(LOG_CRIT, "No races defined in the database!" );
        return;
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    races = (struct race_type *)malloc(raceCount * sizeof(struct race_type));
    if( !races ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating races[]" );
        exit(1);
    }

    memset( races, 0, raceCount * sizeof(struct race_type) );

    for (i = 0; i < raceCount; i++) {
        row = mysql_fetch_row(res);

        races[i].race = atoi(row[0]) - 1;
        races[i].racename = strdup(row[1]);
        if( row[2] ) {
            races[i].desc = strdup(row[2]);
        } else {
            races[i].desc = NULL;
        }
        races[i].size = atoi(row[4]);
        races[i].start = atoi(row[5]);
        races[i].young = atoi(row[6]);
        races[i].mature = atoi(row[7]);
        races[i].middle = atoi(row[8]);
        races[i].old = atoi(row[9]);
        races[i].venerable = atoi(row[10]);
        races[i].nativeLanguage = atoi(row[11]) - 1;
        if( races[i].nativeLanguage < 0 ) {
            races[i].nativeLanguage = 0;
        }

        races[i].racialMax = (int *)malloc(classCount * sizeof(int));
        if( !races[i].racialMax ) {
            LogPrintNoArg(LOG_CRIT, "Out of memory allocating "
                                    "races[i].racialMax" );
            exit(1);
        }

        memset( races[i].racialMax, 0, classCount * sizeof(int) );

        for( j = 0; j < classCount; j++ ) {
            data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
            memset( data, 0, 2 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], races[i].race + 1, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], j+1, MYSQL_TYPE_LONG );
            db_queue_query( 17, QueryTable, data, 2, result_load_races,
                            (void *)&races[i].racialMax[j], mutex );
            pthread_mutex_unlock( mutex );
        }
    }

    pthread_mutex_destroy( mutex );
    free( mutex );

    LogPrintNoArg(LOG_CRIT, "Finished loading races[] from SQL");
}

void chain_load_languages( MYSQL_RES *res, QueryItem_t *item )
{
    int             i;

    MYSQL_ROW       row;

    if( !res || !(languageCount = mysql_num_rows(res)) ) {
        LogPrintNoArg(LOG_CRIT, "No languages defined in the database!" );
        return;
    }

    languages = (struct lang_def *)malloc(languageCount * 
                                          sizeof(struct lang_def));
    if( !languages ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating languages[]" );
        exit(1);
    }


    for (i = 0; i < languageCount; i++) {
        row = mysql_fetch_row(res);

        languages[i].langSpeaks = atoi(row[0]);
        languages[i].langSkill  = atoi(row[1]);
        languages[i].name       = strdup(row[2]);
    }

    LogPrintNoArg(LOG_CRIT, "Finished loading languages[] from SQL");
}

void chain_load_textfiles( MYSQL_RES *res, QueryItem_t *item )
{
    char          **buffer;
    MYSQL_ROW       row;

    buffer = (char **)item->queryData[1].buffer;
    if( *buffer ) {
        free( *buffer );
        *buffer = NULL;
    }

    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        *buffer = strdup(row[0]);
    }
}

void chain_delete_board_message( MYSQL_RES *res, QueryItem_t *item )
{
    int                 i;
    int                 count;
    int                 msgId;
    MYSQL_ROW           row;
    struct board_def   *board;

    board = (struct board_def *)item->queryData[2].buffer;

    if( res && (count = mysql_num_rows(res)) ) {
        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);
            msgId = atoi(row[0]);
            db_delete_board_message(board, msgId);
        }
    }
}

void chain_load_object_tree( MYSQL_RES *res, QueryItem_t *item )
{
    BalancedBTree_t        *tree;
    BalancedBTreeItem_t    *bitem;
    struct index_data      *index;
    int                     i,
                            vnum;
    int                     count;

    MYSQL_ROW               row;
    MYSQL_BIND             *data;
    pthread_mutex_t        *mutex;

    tree = (BalancedBTree_t *)item->queryData[0].buffer;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    BalancedBTreeLock( tree );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        index = (struct index_data *)malloc(sizeof(struct index_data));
        if( !index ) {
            continue;
        }

        vnum = atoi(row[0]);
        index->vnum = vnum;
        index->number = 0;
        index->data = NULL;
        index->func = NULL;
        index->list = LinkedListCreate(NULL);

        data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
        memset( data, 0, 1 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], vnum, MYSQL_TYPE_LONG );
        db_queue_query( 53, QueryTable, data, 1, result_load_object_tree,
                        (void *)index, mutex );
        pthread_mutex_unlock( mutex );

        bitem = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
        bitem->key  = &index->vnum;
        bitem->item = (void *)index;
        BalancedBTreeAdd( tree, bitem, LOCKED, FALSE );
    }

    pthread_mutex_destroy( mutex );
    free( mutex );

    BalancedBTreeAdd( tree, NULL, LOCKED, TRUE );
    BalancedBTreeUnlock( tree );
}

void chain_load_rooms( MYSQL_RES *res, QueryItem_t *item )
{
    BalancedBTree_t        *tree;
    BalancedBTreeItem_t    *bitem;
    struct room_data       *room;
    int                     i,
                            roomId;
    int                     count;

    MYSQL_ROW               row;
    MYSQL_BIND             *data;
    pthread_mutex_t        *mutex;

    tree = (BalancedBTree_t *)item->queryData[0].buffer;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init( mutex, NULL );

    BalancedBTreeLock( tree );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        room = (struct room_data *)malloc(sizeof(struct room_data));
        if( !room ) {
            continue;
        }
        memset(room, 0, sizeof(struct room_data));

        roomId = atoi(row[0]);

        room->number = roomId;
        room->zone = atoi(row[1]);
        room->name = strdup(row[2]);
        room->description = strdup(row[3]);
        room->sector_type = atoi(row[4]);
        room->tele_time = atoi(row[5]);
        room->tele_targ = atoi(row[6]);
        room->tele_cnt = atoi(row[7]);
        room->river_speed = atoi(row[8]);
        room->river_dir = atoi(row[9]);
        room->moblim = atoi(row[10]);
        room->func = procGetFuncByName( row[11], PROC_ROOM );
        room->light = atoi(row[12]);

        room->contentList = LinkedListCreate(NULL);
        room->contentKeywordTree = BalancedBTreeCreate( NULL, 
                                                        BTREE_KEY_STRING );

        /* Load the room_flags and tele_mask */
        data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
        memset( data, 0, 1 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], roomId, MYSQL_TYPE_LONG );
        db_queue_query( 73, QueryTable, data, 1, result_load_rooms_1,
                        (void *)room, mutex );
        pthread_mutex_unlock( mutex );

        /* Load the extra descriptions */
        data = (MYSQL_BIND *)malloc(1 * sizeof(MYSQL_BIND));
        memset( data, 0, 1 * sizeof(MYSQL_BIND) );

        bind_numeric( &data[0], roomId, MYSQL_TYPE_LONG );
        db_queue_query( 74, QueryTable, data, 1, result_load_rooms_2,
                        (void *)room, mutex );
        pthread_mutex_unlock( mutex );

        /* Load the exits */
        for( i = 0; i < 6; i++ ) {
            data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
            memset( data, 0, 2 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], roomId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], i, MYSQL_TYPE_LONG );
            db_queue_query( 75, QueryTable, data, 1, result_load_rooms_3,
                            (void *)room, mutex );
            pthread_mutex_unlock( mutex );

            if( !room->dir_option[i] ) {
                continue;
            }

            /* Load the exit flags */
            data = (MYSQL_BIND *)malloc(2 * sizeof(MYSQL_BIND));
            memset( data, 0, 2 * sizeof(MYSQL_BIND) );

            bind_numeric( &data[0], roomId, MYSQL_TYPE_LONG );
            bind_numeric( &data[1], i, MYSQL_TYPE_LONG );
            db_queue_query( 76, QueryTable, data, 1, result_load_rooms_4,
                            (void *)room, mutex );
            pthread_mutex_unlock( mutex );
        }

        bitem = (BalancedBTreeItem_t *)malloc(sizeof(BalancedBTreeItem_t));
        bitem->key  = &room->number;
        bitem->item = (void *)room;
        BalancedBTreeAdd( tree, bitem, LOCKED, FALSE );
    }

    pthread_mutex_destroy( mutex );
    free( mutex );

    BalancedBTreeAdd( tree, NULL, LOCKED, TRUE );
    BalancedBTreeUnlock( tree );
}

/*
 * Query result callbacks
 */

void result_get_report( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct user_report **rept;
    struct user_report *report;
    int             count;
    int             mcount;
    int             i;
    MYSQL_ROW       row;

    rept = (struct user_report **)arg;

    count = mysql_num_rows(res);
    mcount = ( count ? count : 1 );

    report = (struct user_report *)malloc(mcount * sizeof(struct user_report));
    *rept = report;
    if( !report ) {
        LogPrintNoArg( LOG_CRIT, "Can't allocate a user report buffer!" );
        return;
    }

    if( !count ) {
        /* No reports */
        report->resCount  = 0;
        report->timestamp = NULL;
        report->character = NULL;
        report->report    = strdup("No reports at this time");
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        report[i].resCount  = count;
        report[i].timestamp = strdup(row[0]);
        report[i].character = strdup(row[1]);
        report[i].roomNum   = atoi(row[2]);
        report[i].report    = strdup(row[3]);
    }
}

void result_load_classes_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int                 count;
    struct class_def   *cls;
    int                 i;
    MYSQL_ROW           row;

    cls = (struct class_def *)arg;
    
    if( !res || !(count = mysql_num_rows(res)) ) {
        /* No result for this query */
        cls->skillCount = 0;
        cls->skills = NULL;
        return;
    } 

    cls->skillCount = count;
    cls->skills = (struct skillset *)malloc(count * sizeof(struct skillset));
    if( !cls->skills ) {
        cls->skillCount = 0;
        LogPrintNoArg( LOG_CRIT, "Dumping skills due to lack of memory" );
        exit(1);
    }
    
    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        cls->skills[i].name     = strdup(row[0]);
        cls->skills[i].skillnum = atoi(row[1]);
        cls->skills[i].level    = atoi(row[2]);
        cls->skills[i].maxlearn = atoi(row[3]);
    }
}

void result_load_classes_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int                 count;
    struct class_def   *cls;
    int                 i;
    MYSQL_ROW           row;

    cls = (struct class_def *)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        /* No result for this query */
        cls->mainskillCount = 0;
        cls->mainskills = NULL;
        return;
    }
    
    cls->mainskillCount = count;
    cls->mainskills = (struct skillset *)malloc(count* sizeof(struct skillset));

    if( !cls->mainskills ) {
        cls->mainskillCount = 0;
        LogPrintNoArg( LOG_CRIT, "Dumping mainskills due to lack of memory" );
        exit( 1 );
    } 
    
    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        cls->mainskills[i].name     = strdup(row[0]);
        cls->mainskills[i].skillnum = atoi(row[1]);
        cls->mainskills[i].level    = atoi(row[2]);
        cls->mainskills[i].maxlearn = atoi(row[3]);
    }
}


void result_load_classes_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int                 count;
    struct class_def   *cls;
    int                 i;
    int                 level;
    MYSQL_ROW           row;

    cls = (struct class_def *)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        /* No result for this query */
        cls->levelCount = 0;
        cls->levels = NULL;
        return;
    } 
    
    cls->levelCount = count;
    cls->levels = (struct class_level_t *)
                      malloc(count * sizeof(struct class_level_t));

    if( !cls->levels ) {
        cls->levelCount = 0;
        LogPrintNoArg( LOG_CRIT, "Dumping levels due to lack of memory" );
        exit( 1 );
    } 
    
    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);
        level = atoi(row[0]);
        cls->levels[level].thaco   = atoi(row[1]);
        cls->levels[level].title_m = strdup(row[2]);
        cls->levels[level].title_f = strdup(row[3]);
        cls->levels[level].exp     = atol(row[4]);
    }
}

void result_load_messages( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    char               *msg;
    MYSQL_ROW           row;

    msg = (char *)arg;

    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        msg = strdup(row[0]);
    }
}

void result_load_structures_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int             i;

    MYSQL_ROW       row;

    if( !res || !(directionCount = mysql_num_rows(res)) ) {
        LogPrintNoArg( LOG_CRIT, "No directions defined in the database!" );
        exit(1);
    }

    direction = (struct dir_data *)malloc(directionCount * 
                                          sizeof(struct dir_data));
    if( !direction ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating direction[]" );
        exit(1);
    }

    for( i = 0; i < directionCount; i++ ) {
        row = mysql_fetch_row(res);
        direction[i].forward  = atoi(row[0]);
        direction[i].rev      = atoi(row[1]);
        direction[i].trap     = atoi(row[2]);
        direction[i].exit     = strdup(row[3]);
        direction[i].listexit = strdup(row[4]);
        direction[i].dir      = strdup(row[5]);
        direction[i].desc     = strdup(row[6]);
    }
}

void result_load_structures_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int             i;

    MYSQL_ROW       row;

    if( !res || !(clanCount = mysql_num_rows(res)) ) {
        LogPrintNoArg( LOG_CRIT, "No clans defined in the database!" );
        exit(1);
    }

    clan_list = (struct clan *)malloc(clanCount * sizeof(struct clan));
    if( !direction ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating clan_list[]" );
        exit(1);
    }

    for( i = 0; i < clanCount; i++ ) {
        row = mysql_fetch_row(res);
        clan_list[i].number    = atoi(row[0]);
        clan_list[i].name      = strdup(row[1]);
        clan_list[i].shortname = strdup(row[2]);
        clan_list[i].desc      = strdup(row[3]);
        clan_list[i].home      = atoi(row[4]);
    }
}

void result_load_structures_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int             i;
    MYSQL_ROW       row;

    if( !res || !(sectorCount = mysql_num_rows(res)) ) {
        LogPrintNoArg( LOG_CRIT, "No sector types defined in the database!" );
        exit(1);
    }

    sectors = (struct sector_data *)malloc(sectorCount * 
                                           sizeof(struct sector_data));
    if( !sectors ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating sectors[]" );
        exit(1);
    }

    for( i = 0; i < sectorCount; i++ ) {
        row = mysql_fetch_row(res);
        sectors[i].type     = strdup(row[0]);
        sectors[i].mapChar  = strdup(row[1]);
        sectors[i].moveLoss = atoi(row[2]);
    }
}

void result_load_bannedUsers( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    /*
     * 0 - full match
     * 1 - from start of string
     * 2 - from end of string
     * 3 - somewhere in string
     */
    int             i;
    char           *tmp;

    MYSQL_ROW       row;

    if( !res || !(bannedUserCount = mysql_num_rows(res)) ) {
        LogPrintNoArg( LOG_CRIT, "No banned usernames defined in the "
                                 "database!" );
        exit(1);
    }

    bannedUsers = (struct banned_user *)malloc(bannedUserCount * 
                                               sizeof(struct banned_user));
    if( !bannedUsers ) {
        LogPrintNoArg( LOG_CRIT, "Out of memory allocating bannedUsers[]" );
        exit(1);
    }

    for (i = 0; i < bannedUserCount; i++) {
        row = mysql_fetch_row(res);
        tmp = row[0];
        if (*tmp == '*') {
            if (tmp[strlen(tmp) - 1] == '*') {
                bannedUsers[i].how = 3;
                tmp[strlen(tmp) - 1] = '\0';
            } else {
                bannedUsers[i].how = 2;
            }
            tmp++;
        } else if (tmp[strlen(tmp) - 1] == '*') {
                bannedUsers[i].how = 1;
                tmp[strlen(tmp) - 1] = '\0';
        } else {
            bannedUsers[i].how = 0;
        }
        bannedUsers[i].name = strdup(tmp);
        bannedUsers[i].len  = strlen(bannedUsers[i].name);
    }

    LogPrintNoArg(LOG_CRIT, "Finished loading banned usernames from SQL");
}

void result_load_races( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int            *value;
    MYSQL_ROW       row;

    value = (int *)arg;

    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);
        *value = atoi(row[0]);
    }
}

void result_lookup_board( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct board_def  **retboard;
    struct board_def   *board;
    MYSQL_ROW           row;

    retboard = (struct board_def **)arg;

    /* result_lookup_board */
    if( !res || !mysql_num_rows(res) ) {
        *retboard = NULL;
        return;
    }

    row = mysql_fetch_row(res);
    
    board = (struct board_def *)malloc(sizeof(struct board_def));
    *retboard = board;
    if( !board ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating a board structure!" );
        return;
    }

    board->boardId = atoi(row[0]);
    board->messageCount = atoi(row[1]);
    board->minLevel = atoi(row[2]);
}

void result_get_board_message( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct bulletin_board_message **retmsg;
    struct bulletin_board_message *msg;

    MYSQL_ROW       row;

    retmsg = (struct bulletin_board_message **)arg;

    if( !res || !mysql_num_rows(res) ) {
        *retmsg = NULL;
        return;
    }

    row = mysql_fetch_row(res);
    
    msg = (struct bulletin_board_message *)
              malloc(sizeof(struct bulletin_board_message));
    *retmsg = msg;
    if( !msg ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating a message "
                                "structure!" );
        return;
    }

    msg->author = strdup(row[0]);
    msg->title = strdup(row[1]);
    msg->text = strdup(row[2]);
    msg->date = atoi(row[3]);
    msg->message_id = atoi(row[4]);
    msg->reply_to = atoi(row[5]);
}

void result_get_board_replies( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    BoardRepliesArgs_t *args;
    struct bulletin_board_message *msg;

    int             i;

    MYSQL_ROW       row;

    args = (BoardRepliesArgs_t *)arg;
    args->count = 0;
    
    if( !res || !(args->count = mysql_num_rows(res)) ) {
        *args->msg = NULL;
        return;
    }

    msg = (struct bulletin_board_message *)
              malloc(sizeof(struct bulletin_board_message) * args->count);
    *args->msg = msg;
    if( !msg ) {
        LogPrintNoArg(LOG_CRIT, "Out of memory allocating a message "
                                "structure!" );
        return;
    }

    for(i = 0; i < args->count; i++ ) {
        row = mysql_fetch_row(res);

        msg[i].author     = strdup(row[0]);
        msg[i].title      = strdup(row[1]);
        msg[i].text       = strdup(row[2]);
        msg[i].message_id = atoi(row[3]);
        msg[i].date       = atoi(row[4]);
        msg[i].reply_to   = atoi(row[5]);
    }
}

void result_has_mail( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int          *retval;
    MYSQL_ROW     row;

    retval = (int *)arg;

    /* result_has_mail */
    if( !res || !mysql_num_rows(res) ) {
        *retval = 0;
        return;
    }

    row = mysql_fetch_row(res);
    *retval = atoi(row[0]);
}


void result_get_mail_ids( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MailIdsArgs_t  *args;
    int             i;
    MYSQL_ROW       row;

    args = (MailIdsArgs_t *)arg;
    args->count = 0;

    if( !res || !(args->count = mysql_num_rows(res)) ) {
        return;
    }

    for( i = 0; i < args->count; i++ ) {
        row = mysql_fetch_row(res);
        args->msgNum[i] = atoi(row[0]);
    }
}

void result_get_mail_message( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    char          **retmsg;
    char           *msg;
    MYSQL_ROW       row;

    retmsg = (char **)arg;

    if( !res || !mysql_num_rows(res) ) {
        *retmsg = NULL;
        return;
    }

    row = mysql_fetch_row(res);

    msg = (char *)malloc( 64 + strlen(row[0]) + strlen(row[1]) + 
                          strlen(row[2]) + strlen(row[3]) + 2 );
    *retmsg = msg;
    sprintf(msg, " * * * * Havok Mail System * * * *\n\r"
                 "Date: %s\n\r"
                 "  To: %s\n\r"
                 "From: %s\n\r\n\r%s\n\r", row[2], row[1], row[0], row[3]);
}

void result_check_kill_file( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int            *count;

    count = (int *)arg;

    *count = (res ? mysql_num_rows(res) : 0 );
}


void result_lookup_help( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    char          **retmsg;
    char           *msg;
    MYSQL_ROW       row;

    retmsg = (char **)arg;

    if( !res || !mysql_num_rows(res) ) {
        *retmsg = NULL;
        return;
    }

    row = mysql_fetch_row(res);

    msg = (char *)malloc( 8 + strlen(row[0]) + strlen(row[1]) + 2);
    *retmsg = msg;

    sprintf(msg, "\"%s\"\n\r\n\r%s\n\r", row[0], row[1] );
}


void result_lookup_help_similar( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    LookupHelpArgs_t   *args;
    char               *msg;
    MYSQL_ROW           row;

    char                line[256];
    int                 count,
                        len,
                        i;

    args = (LookupHelpArgs_t *)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        args->msg = NULL;
        return;
    }

    len = 200 + (count * 82) + 50 + strlen(args->keywords);

    msg = (char *)malloc(len);
    args->msg = msg;

    sprintf(msg, "No exact matches found for \"%s\".  Top %d relevant "
                 "topics are:\n\r  %-68s Score\n\r", args->keywords, count,
                 "Keywords" );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        snprintf(line, 255, "    %-66s %6.3f\n\r", row[0],
                                                   strtod(row[1], NULL));
        strcat(msg, line);
    }

    strcat(msg, "Please retry help using one of these keywords.\n\r");
}

void result_read_object_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct obj_data           **retobj;
    struct obj_data            *obj;

    MYSQL_ROW                   row;

    retobj = (struct obj_data **)arg;
    obj = *retobj;

    if( !res || !mysql_num_rows(res) ) {
        *retobj = NULL;
        return;
    }

    row = mysql_fetch_row(res);

    obj->short_description      = strdup(row[0]);
    obj->description            = strdup(row[1]);
    obj->action_description     = strdup(row[2]);
    obj->modBy                  = strdup(row[3]);
    obj->type_flag              = atoi(row[4]);
    obj->value[0]               = atoi(row[5]);
    obj->value[1]               = atoi(row[6]);
    obj->value[2]               = atoi(row[7]);
    obj->value[3]               = atoi(row[8]);
    obj->weight                 = atoi(row[9]);
    obj->cost                   = atoi(row[10]);
    obj->cost_per_day           = atoi(row[11]);
    obj->level                  = atoi(row[12]);
    obj->max                    = atoi(row[13]);
    obj->modified               = atol(row[14]); 
    obj->speed                  = atoi(row[15]);
    obj->weapontype             = atoi(row[16]) - 1;
    obj->tweak                  = atoi(row[17]);
    obj->eq_pos                 = atoi(row[18]);
}

void result_read_object_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct obj_data           **retobj;
    struct obj_data            *obj;
    int                         count;
    int                         i;

    MYSQL_ROW                   row;

    retobj = (struct obj_data **)arg;
    obj = *retobj;

    if( !res || !(count = mysql_num_rows(res)) ) {
        *retobj = NULL;
        objectFree(obj);
        return;
    }

    obj->keywords.count = count;
    obj->keywords.words = (char **)malloc(count * sizeof(char *));
    obj->keywords.length = (int *)malloc(count * sizeof(int));
    obj->keywords.found = (int *)malloc(count * sizeof(int));

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        obj->keywords.words[i] = strdup(row[0]);
        obj->keywords.length[i] = strlen(row[0]);
    }
}

void result_read_object_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct obj_data   **retobj;
    struct obj_data    *obj;
    unsigned int       *var;
    int                 i;
    int                 count;
    int                 type;
    int                 index;
    int                 value;
    struct obj_flags_t *flags;

    MYSQL_ROW           row;

    retobj = (struct obj_data **)arg;
    obj = *retobj;

    obj->wear_flags = 0;
    obj->extra_flags = 0;
    obj->anti_flags = 0;
    obj->anti_class = 0;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        type = atoi(row[0]);
        index = atoi(row[1]);
        value = atoi(row[2]);

        if( !value || type < 0 || type >= obj_flag_count ) {
            continue;
        }

        flags = &obj_flags[type];

        if( index < 0 || index >= flags->count ) {
            continue;
        }

        var = (unsigned int *)PTR_AT_OFFSET(flags->offset, obj);

        SET_BIT(*var, flags->bits[index].set);
        REMOVE_BIT(*var, flags->bits[index].clear);
    }
}

void result_read_object_4( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct obj_data           **retobj;
    struct obj_data            *obj;
    Keywords_t                 *descr;
    int                         count;
    int                         i;

    MYSQL_ROW                   row;

    retobj = (struct obj_data **)arg;
    obj = *retobj;

    obj->ex_description = NULL;
    obj->ex_description_count = 0;

    if( res && (count = mysql_num_rows(res)) ) {
        CREATE(obj->ex_description, Keywords_t, count);
        obj->ex_description_count = count;

        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);
            descr = &obj->ex_description[i];

            StringToKeywords( row[0], descr );
            descr->description = strdup(row[1]);
        }
    }
}

void result_read_object_5( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct obj_data           **retobj;
    struct obj_data            *obj;
    int                         count;
    int                         i;

    MYSQL_ROW                   row;

    retobj = (struct obj_data **)arg;
    obj = *retobj;

    if( res && (count = mysql_num_rows(res)) ) {
        for( i = 0; i < count && i < MAX_OBJ_AFFECT; i++ ) {
            row = mysql_fetch_row(res);

            obj->affected[i].location = atoi(row[0]);
            obj->affected[i].modifier = atoi(row[1]);
        }
    }
}


void result_find_object_named( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int                *retvnum;
    int                 vnum;
    MYSQL_ROW           row;

    retvnum = (int *)arg;

    if( res && mysql_num_rows(res) ) {
        row = mysql_fetch_row(res);

        vnum = atoi(row[0]);
        *retvnum = vnum;
    }
}

void result_load_object_tree( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    int                 count;
    int                 i;
    struct index_data  *index;
    MYSQL_ROW           row;

    index = (struct index_data *)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    index->keywords.count  = count;
    index->keywords.words  = (char **)malloc( count * sizeof(char *) );
    index->keywords.length = (int *)malloc( count * sizeof(int) );
    index->keywords.found  = (int *)malloc( count * sizeof(int) );

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        index->keywords.words[i] = strdup(row[0]);
        index->keywords.length[i] = strlen(row[0]);
    }
}


void result_load_char_extra_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MYSQL_ROW           row;
    struct char_data   *ch;
    char                temp[400];

    ch = (struct char_data *)arg;
    if( !ch || !res || !mysql_num_rows(res) ) {
        return;
    }

    row = mysql_fetch_row(res);

    do_bamfout(ch, row[0], 0);
    do_bamfin(ch, row[1], 0);
    GET_ZONE(ch) = atoi(row[2]);
    do_set_prompt(ch, row[3], 0);
    do_set_bprompt(ch, row[4], 0);
    if( row[5] ) {
        sprintf(temp, "email %s", row[5]);
        do_set_flags(ch, temp, 0);
    }

    if( row[6] ) {
        sprintf(temp, "clan %s", row[6]);
        do_set_flags(ch, temp, 0);
    }

    ch->specials.hostip = strdup(row[7]);

    if( row[8] ) {
        ch->specials.rumor = strdup(row[8]);
    }

    do_setsev(ch, row[9], 0);
    do_invis(ch, row[10], 242);       
}

void result_load_char_extra_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MYSQL_ROW           row;
    struct char_data   *ch;
    int                 count;
    int                 i;
    char                temp[400];

    ch = (struct char_data *)arg;
    if( !ch || !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        sprintf(temp, "%s %s", row[0], row[1] );
        do_alias(ch, temp, 260);
    }
}

void result_get_char_rent( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MYSQL_ROW           row;
    GetCharRentArgs_t  *args;

    args = (GetCharRentArgs_t *)arg;
    if( !args || !res || !mysql_num_rows(res) ) {
        return;
    }

    row = mysql_fetch_row(res);
    *args->gold       = atoi(row[0]);
    *args->rentCost   = atoi(row[1]);
    *args->minStay    = atoi(row[2]);
    *args->lastUpdate = atoi(row[3]);
}

void result_load_char_objects_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MYSQL_ROW           row;
    LoadCharObjsArgs_t **argPoint;
    LoadCharObjsArgs_t *args;
    LoadCharObjsArgs_t *newargs;
    struct obj_data    *obj;
    int                 count;
    struct char_data   *ch;
    int                 i;

    argPoint = (LoadCharObjsArgs_t **)arg;
    args = *argPoint;

    if( !args || !args->ch || !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    ch = args->ch;

    CREATE(newargs, LoadCharObjsArgs_t, 1);
    CREATE(newargs->parentNum, int, count);
    CREATE(newargs->parentObj, struct obj_data *, count);
    newargs->count = count;
    newargs->i = 0;
    newargs->prev = args;
    newargs->ch = ch;

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        CREATE(obj, struct obj_data, 1);

        obj->item_number            = atoi(row[0]);
        obj->index                  = objectIndex( obj->item_number );
        newargs->parentNum[i]       = atoi(row[1]);
        obj->short_description      = strdup(row[2]);
        obj->description            = strdup(row[3]);
        obj->action_description     = strdup(row[4]);
        obj->modBy                  = strdup(row[5]);
        obj->type_flag              = atoi(row[6]);
        obj->value[0]               = atoi(row[7]);
        obj->value[1]               = atoi(row[8]);
        obj->value[2]               = atoi(row[9]);
        obj->value[3]               = atoi(row[10]);
        obj->weight                 = atoi(row[11]);
        obj->cost                   = atoi(row[12]);
        obj->cost_per_day           = atoi(row[13]);
        obj->level                  = atoi(row[14]);
        obj->max                    = atoi(row[15]);
        obj->modified               = atol(row[16]); 
        obj->speed                  = atoi(row[17]);
        obj->weapontype             = atoi(row[18]) - 1;
        obj->tweak                  = atoi(row[19]);
        obj->eq_pos                 = atoi(row[20]);
        newargs->parentObj[i] = obj;

        if( obj->eq_pos != -1 ) {
            equip_char(ch, obj, obj->eq_pos );
        } else if( args->parentObj[args->i] ) {
            objectPutInObject( obj, args->parentObj[args->i] );
        } else {
            objectGiveToChar( obj, ch );
        }
    }

    *argPoint = newargs;
}

void result_load_room_objects_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    MYSQL_ROW           row;
    LoadCharObjsArgs_t **argPoint;
    LoadCharObjsArgs_t *args;
    LoadCharObjsArgs_t *newargs;
    struct obj_data    *obj;
    int                 count;
    int                 room;
    int                 i;

    argPoint = (LoadCharObjsArgs_t **)arg;
    args = *argPoint;

    if( !args || !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    room = args->room;

    CREATE(newargs, LoadCharObjsArgs_t, 1);
    CREATE(newargs->parentNum, int, count);
    CREATE(newargs->parentObj, struct obj_data *, count);
    newargs->count = count;
    newargs->i = 0;
    newargs->prev = args;
    newargs->room = room;

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        CREATE(obj, struct obj_data, 1);

        obj->item_number            = atoi(row[0]);
        obj->index                  = objectIndex( obj->item_number );
        newargs->parentNum[i]       = atoi(row[1]);
        obj->short_description      = strdup(row[2]);
        obj->description            = strdup(row[3]);
        obj->action_description     = strdup(row[4]);
        obj->modBy                  = strdup(row[5]);
        obj->type_flag              = atoi(row[6]);
        obj->value[0]               = atoi(row[7]);
        obj->value[1]               = atoi(row[8]);
        obj->value[2]               = atoi(row[9]);
        obj->value[3]               = atoi(row[10]);
        obj->weight                 = atoi(row[11]);
        obj->cost                   = atoi(row[12]);
        obj->cost_per_day           = atoi(row[13]);
        obj->level                  = atoi(row[14]);
        obj->max                    = atoi(row[15]);
        obj->modified               = atol(row[16]); 
        obj->speed                  = atoi(row[17]);
        obj->weapontype             = atoi(row[18]) - 1;
        obj->tweak                  = atoi(row[19]);
        obj->eq_pos                 = atoi(row[20]);
        newargs->parentObj[i] = obj;

        if( args->parentObj[args->i] ) {
            objectPutInObject( obj, args->parentObj[args->i] );
        } else {
            objectPutInRoom( obj, room, UNLOCKED );
        }
    }

    *argPoint = newargs;
}

void result_load_rooms_1( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct room_data   *room;
    unsigned int       *var;
    int                 i;
    int                 count;
    int                 type;
    int                 index;
    int                 value;
    struct obj_flags_t *flags;

    MYSQL_ROW           row;

    room = (struct room_data *)arg;

    room->room_flags = 0;
    room->tele_mask  = 0;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        type = atoi(row[0]) - 12;  /* room flags = 12, teleport mask = 13 */
        index = atoi(row[1]);
        value = atoi(row[2]);

        if( !value || type < 0 || type >= room_flag_count ) {
            continue;
        }

        flags = &room_flags[type];

        if( index < 0 || index >= flags->count ) {
            continue;
        }

        var = (unsigned int *)PTR_AT_OFFSET(flags->offset, room);

        SET_BIT(*var, flags->bits[index].set);
        REMOVE_BIT(*var, flags->bits[index].clear);
    }
}

void result_load_rooms_2( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct room_data           *room;
    Keywords_t                 *descr;
    int                         count;
    int                         i;

    MYSQL_ROW                   row;

    room = (struct room_data *)arg;

    room->ex_description = NULL;
    room->ex_description_count = 0;

    if( res && (count = mysql_num_rows(res)) ) {
        CREATE(room->ex_description, Keywords_t, count);
        room->ex_description_count = count;

        for( i = 0; i < count; i++ ) {
            row = mysql_fetch_row(res);
            descr = &room->ex_description[i];

            StringToKeywords( row[0], descr );
            descr->description = strdup(row[1]);
        }
    }
}

void result_load_rooms_3( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct room_data           *room;
    int                         i;
    struct room_direction_data *dir;
    CommandDef_t               *command;

    MYSQL_ROW                   row;

    room = (struct room_data *)arg;

    if( !res || !mysql_num_rows(res) ) {
        return;
    }

    row = mysql_fetch_row(res);
    i = atoi(row[0]) - 1;

    CREATE(dir, struct room_direction_data, 1);
    room->dir_option[i] = dir;

    dir->general_description = strdup(row[1]);
    if( row[2][0] == '\0' ) {
        dir->keywords = NULL;
    } else {
        dir->keywords = StringToKeywords( row[2], NULL );
    }
    dir->key = atoi(row[3]);
    dir->to_room = atoi(row[4]);
    command = FindCommand( row[5] );
    dir->open_cmd = (command ? command->number : 0);
    dir->exit_info = 0;
}

void result_load_rooms_4( MYSQL_RES *res, MYSQL_BIND *input, void *arg )
{
    struct room_data   *room;
    struct room_direction_data *dir;
    unsigned int       *var;
    int                 i;
    int                 count;
    int                 type;
    int                 index;
    int                 value;
    struct obj_flags_t *flags;

    MYSQL_ROW           row;

    room = (struct room_data *)arg;

    if( !res || !(count = mysql_num_rows(res)) ) {
        return;
    }

    for( i = 0; i < count; i++ ) {
        row = mysql_fetch_row(res);

        dir = room->dir_option[atoi(row[0])];

        type = atoi(row[1]) - 14;  /* exit flags = 14 */
        index = atoi(row[2]);
        value = atoi(row[3]);

        if( !value || type < 0 || type >= exit_flag_count ) {
            continue;
        }

        flags = &exit_flags[type];

        if( index < 0 || index >= flags->count ) {
            continue;
        }

        var = (unsigned int *)PTR_AT_OFFSET(flags->offset, dir);

        SET_BIT(*var, flags->bits[index].set);
        REMOVE_BIT(*var, flags->bits[index].clear);
    }
}



/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
