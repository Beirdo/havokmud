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
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace havokmud {
    namespace objects {
        class DatabaseRequest;

        typedef boost::shared_ptr<Databasequest> RequestPointer;

        class DatabaseHandler
        {
        public:
            DatabaseHandler(const std::string &command_,
                            const std::string &query_,
                            const std::string &parameters_[],
                            bool requiresResponse_ = false,
                            bool requiresInsertId_ = false) :
                    m_command(command_), m_query(query_),
                    m_requiresResponse(requiresResponse_),
                    m_requiresInsertId(requiresInsertId_), m_response()
            {
                int count = NELEMS(parameters_);
                for (int i = 0; i < count; i++) {
                    m_parameters.push_back(parameters_[i]);
                }
                s_handlerMap.insert(std::pair<std::string, DatabaseHandler *>
                        (m_command, this));
            };

            ~DatabaseHandler()
            {
                s_handlerMap.erase(m_command);
            };

            const std::string &command()   { return m_command; };
            const std::string &query()     { return m_query; };
            bool requiresResponse() const  { return m_requiresResponse; };
            bool requiresInsertId() const  { return m_requiresInsertId; };
            std::vector<std::string> parameters() const { m_parameters; };

            static void initialize()  {};

            static DatabaseHandler *findCommand(const std::string &command_)
            {
                std::map<std::string, DatabaseHandler *>::iterator it =
                    s_handlerMap.find(command_);

                if (it == s_handlerMap.end())
                    return NULL;

                return it->second;
            };

            RequestPointer getRequest(const boost::property_tree::ptree &data)
            {
                boost::format q(query());
                std::vector<std::string> params(parameters());
                BOOST_FOREACH(std::string &param, params) {
                    std::string value(data.get<std::string>(param));
                    q % value;
                }

                RequestPointer request(new DatabaseRequest(q,
                        requiresResponse(), requiresInsertId()));

                return request;
            };

        private:
            const std::string   m_command;
            const std::string   m_query;
            bool                m_requiresResponse;
            bool                m_requiresInsertId;
            std::vector<std::string> m_parameters;

            static std::map<std::string, DatabaseHandler *> s_handlerMap;
        };
    }
}

#endif  // __havokmud_corefunc_DatabaseHandler__
