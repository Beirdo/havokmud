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

#ifndef __havokmud_corefunc_DatabaseHandler__
#define __havokmud_corefunc_DatabaseHandler__

#include <string>
#include <map>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>

#include "objects/DatabaseRequest.hpp"
#include "util/misc.hpp"

namespace havokmud {
    namespace objects {
        class DatabaseRequest;
    }

    namespace corefunc {
        using havokmud::objects::DatabaseRequest;
        typedef boost::shared_ptr<DatabaseRequest> RequestPointer;

        class DatabaseHandler
        {
        public:
            DatabaseHandler(const std::string &command_,
                            const std::string &query_,
                            const std::vector<std::string> &parameters_,
                            bool requiresResponse_ = false,
                            bool requiresInsertId_ = false,
                            const std::string &chainCommand_ = std::string()) :
                    m_command(command_), m_query(query_),
                    m_requiresResponse(requiresResponse_),
                    m_requiresInsertId(requiresInsertId_),
                    m_chainCommand(chainCommand_)
            {
                s_handlerMap.insert(std::pair<std::string, DatabaseHandler *>
                        (m_command, this));
            };

            ~DatabaseHandler()
            {
                s_handlerMap.erase(m_command);
            };

            const std::string &command() const  { return m_command; };
            const std::string &query() const    { return m_query; };
            bool requiresResponse() const  { return m_requiresResponse; };
            bool requiresInsertId() const  { return m_requiresInsertId; };
            std::vector<std::string> parameters() const
                    { return m_parameters; };
            const std::string &chainCommand() const  { return m_chainCommand; };

            static void initialize();

            static DatabaseHandler *findCommand(const std::string &command_)
            {
                std::map<std::string, DatabaseHandler *>::iterator it =
                    s_handlerMap.find(command_);

                if (it == s_handlerMap.end())
                    return NULL;

                return it->second;
            };

            RequestPointer getRequest(const boost::property_tree::ptree &data);

        private:
            const std::string   m_command;
            const std::string   m_query;
            bool                m_requiresResponse;
            bool                m_requiresInsertId;
            std::vector<std::string> m_parameters;
            const std::string   m_chainCommand;

            static std::map<std::string, DatabaseHandler *> s_handlerMap;
        };
    }
}

#endif  // __havokmud_corefunc_DatabaseHandler__
