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
                        false, true, "" }
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

