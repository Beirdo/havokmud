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
 * @brief Database protocol handlers (JSON->DB requests)
 */

#include <string>
#include <stdarg.h>

#include "objects/DatabaseRequest.hpp"
#include "corefunc/DatabaseHandler.hpp"
#include "corefunc/Logging.hpp"

typedef struct {
    std::string command;
    std::string query;
    std::vector<std::string> parameters;
    bool requiresResponse;
    bool requiresInsertId;
    std::string chainCommand;
} HandlerItem;

static const HandlerItem handlers[] = {
    { "get setting", "SELECT `value` FROM `settings` WHERE `name` = '%1%' LIMIT 1",
      { "name" }, true, false, "" },
    { "set setting", "SELECT `value` FROM `settings` WHERE `name` = '%1%' LIMIT 1",
      { "name" }, false, false, "set setting:1:2" },
    { "set setting:1", "UPDATE `settings` SET `value` = '%1%' WHERE `name` = '%2%'",
      { "value", "name" }, false, false, "" },
    { "set setting:2", "INSERT INTO `settings` (`name`, `value`) VALUES "
                       "('%1%', '%2%')", { "name", "value" }, false, false, "" },
    { "load account", "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, "
                      "`confcode` FROM `accounts` WHERE `email` = '%1%'",
                      { "email" }, true, false, "" },
    { "load account id", "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, "
                      "`confcode` FROM `accounts` WHERE `id` = %1%",
                      { "id" }, true, false, "" },
    { "save account", "SELECT `id`, `email`, `passwd`, `ansi`, `confirmed`, "
                      "`confcode` FROM `accounts` WHERE `email` = '%1%'",
                      { "email" }, true, false, "save account:1:2" },
    { "save account:1", "UPDATE `accounts` SET `email` = '%1%', "
                        "`passwd` = '%2%', `ansi` = '%3%', "
                        "`confirmed` = '%4%', `confcode` = '%5%' "
                        "WHERE `id` = %6%", { "email", "passwd", "ansi",
                                              "confirmed", "confcode", "id" },
                        false, false, "" },
    { "save account:2", "INSERT INTO `accounts` (`email`, `passwd`, `ansi`, "
                        "`confirmed`, `confcode`) VALUES ('%1%', '%2%', '%3%', "
                        "'%4%', '%5%')",
                        { "email", "passwd", "ansi", "confirmed", "confcode" },
                        false, true, "" },
    { "find player", "SELECT `id`, `account_id`, `name` "
                      "FROM `pcs` WHERE `name` = '%1%'",
                      { "name" }, true, false, "" },
    { "find player id", "SELECT `id`, `account_id`, `name` "
                      "FROM `pcs` WHERE `id` = %1%",
                      { "id" }, true, false, "" },
    { "save player", "SELECT `id`, `account_id`, `name` "
                      "FROM `pcs` WHERE `id` = %1%",
                      { "id" }, true, false, "save player:1:2" },
    { "save player:1", "UPDATE `pcs` SET `account_id` = %1%, "
                       "`name` = '%2%' WHERE `id` = %3%",
                       { "account_id", "name", "id" }, false, false, "" },
    { "save player:2", "INSERT INTO `pcs` (`account_id`, `name`) "
                       "VALUES (%1%, '%2%')",
                       { "account_id", "name" }, false, true, "" },
    { "load attributes", "SELECT `attribsrc`, `attribjson` FROM `pcattribs` "
                         "WHERE `pc_id` = %1%",
                       { "pc_id" }, true, false, "" },
    { "expire attributes", "UPDATE `pcattribs` SET `to_delete` = 1 "
                           "WHERE `pc_id` = %1%", { "pc_id" },
                           false, false, "" },
    { "save attributes", "SELECT `pc_id` FROM `pcattribs` WHERE `pc_id` = %1% "
                         "AND `attribsrc` = '%2%'", { "pc_id", "attribsrc" },
                         true, false, "save attributes:1:2" },
    { "save attributes:1", "UPDATE `pcattribs` SET `attribjson` = '%1%', "
                           "`to_delete` = 0 WHERE `pc_id` = %2% "
                           "AND `attribsrc` = '%3%'",
                           { "attribjson", "pc_id", "attribsrc" },
                           false, false, "" },
    { "save attributes:2", "INSERT INTO `pcattribs` (`pc_id`, `attribsrc`, "
                           "`attribjson`, `to_delete`) VALUES "
                           "(%1%, '%2%', '%3%', 0)",
                           { "pc_id", "attribsrc", "attribjson" },
                           false, false, "" },
    { "purge attributes", "DELETE FROM `pcattribs` WHERE `pc_id` = %1% "
                          "AND `to_delete` = 1", { "pc_id" },
                          false, false, "" },
};
static const int handlerCount = NELEMS(handlers);

namespace havokmud {
    namespace corefunc {
        std::map<std::string, DatabaseHandler *> DatabaseHandler::s_handlerMap;

        void DatabaseHandler::initialize()
        {
            for (int i = 0; i < handlerCount; i++)
            {
                const HandlerItem *item = &handlers[i];
                DatabaseHandler *handler = new DatabaseHandler(item->command,
                        item->query, item->parameters,
                        item->requiresResponse, item->requiresInsertId,
                        item->chainCommand);
            }
        }

        RequestPointer DatabaseHandler::getRequest(boost::shared_ptr<boost::property_tree::ptree> data)
        {
            boost::format q(query());
            std::vector<std::string> params(parameters());
            BOOST_FOREACH(std::string &param, params) {
                std::string value(data->get<std::string>(param));
                //LogPrint(LG_INFO, "Param %s = %s", param.c_str(),
                //         value.c_str());
                q % value;
            }

            //LogPrint(LG_INFO, "Query: %s", str(q).c_str());

            RequestPointer request(new DatabaseRequest(str(q), data,
                    requiresResponse(), requiresInsertId(), chainCommand()));

            return request;
        }
    }
}

