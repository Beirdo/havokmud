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
 * @brief Thread to handle database requests 
 */

#include <sstream>
#include <string>
#include <stdarg.h>
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "thread/DatabaseThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"
#include "objects/Email.hpp"
#include "corefunc/DatabaseHandler.hpp"

namespace havokmud {
    namespace thread {

        using havokmud::corefunc::DatabaseHandler;

        DatabaseThread::DatabaseThread() : HavokThread("Database"),
                m_abort(false), m_server("tcp://localhost:3306"),
                m_user("havokmud"), m_password("havokmud"),
                m_database("havokdevel")
        {
            pro_initialize<DatabaseThread>();
        }

        void DatabaseThread::start()
        {
            m_driver = get_driver_instance();
            LogPrint(LG_INFO, "MySQL Connector/C++ version %d.%d.%d",
                     m_driver->getMajorVersion(), m_driver->getMinorVersion(),
                     m_driver->getPatchVersion());
            LogPrint(LG_INFO, "MySQL server: %s with user %s",
                     m_server.c_str(), m_user.c_str());
            m_connection.reset(m_driver->connect(m_server, m_user, m_password));

            DatabaseHandler::initialize();

            while (!m_abort)
            {
                boost::shared_ptr<DatabaseRequest> request = m_queue.get();
                if (!request)
                    continue;

                handleRequest(request);
            }
        }

        void DatabaseThread::handleRequest(RequestPointer request)
        {
            ResponsePointer response;

            // use request->m_query
            boost::shared_ptr<sql::Statement>
                    statement(m_connection->createStatement());
            boost::shared_ptr<sql::ResultSet>
                    resultSet(statement->executeQuery(request->query()));
            if (request->requiresResponse()) {
                sql::ResultSetMetaData *resultMetadata =
                        resultSet->getMetaData();

                int rowCount = resultSet->rowsCount();
                int columnCount = resultMetadata->getColumnCount();

                std::vector<std::string> columnNames;
                for (int i = 1; i <= columnCount; i++) {
                    columnNames.push_back(resultMetadata->getColumnName(i));
                }

                std::string jsonResult("[");
                for (int rowNum = 1; rowNum <= rowCount; rowNum++) {
                    resultSet->next();

                    jsonResult += "{";
                    for (int i = 1; i <= columnCount; i++) {
                        jsonResult += "\"" + columnNames[i-1] + "\":\"" 
                                   +  resultSet->getString(i) + "\"";
                        if (i != columnCount)
                            jsonResult += ", ";
                    }

                    jsonResult += "}";
                    if (rowNum != rowCount)
                        jsonResult += ", ";
                }
                jsonResult += "]";

                int insertId;
                if (request->requiresInsertId()) {
                    boost::shared_ptr<sql::Statement>
                            statement2(m_connection->createStatement());
                    boost::shared_ptr<sql::ResultSet>
                            resultSet2(statement2->executeQuery("SELECT LAST_INSERT_ID();"));

                    resultSet2->next();
                    insertId = resultSet2->getInt(1);
                }

                response.reset(new DatabaseResponse(jsonResult, insertId));

                request->setResponse(response);
                request->mutex().unlock();
            }

            if (!request->chainCommand().empty())
            {
                int rowCount = resultSet->rowsCount();
                std::string chainCommand = request->chainCommand();
                boost::smatch match;
                if (boost::regex_match(chainCommand, match, s_chainRegex)) {
                    std::string newCommand(match[1].first, match[1].second);
                    newCommand += ":";
                    if (rowCount) {
                        newCommand.append(match[2].first, match[2].second);
                    } else {
                        newCommand.append(match[3].first, match[3].second);
                    }

                    DatabaseHandler *handler =
                            DatabaseHandler::findCommand(newCommand);
                    if (handler) {
                        RequestPointer
                            chainRequest(handler->getRequest(request->data()));
                        ResponsePointer chainResponse(doRequest(chainRequest));
                        request->setResponse(chainResponse);
                    }
                }
            }
        }

        boost::regex DatabaseThread::s_chainRegex("(.*?):(.*?):(.*?)");

        ResponsePointer DatabaseThread::doRequest(RequestPointer request)
        {
            ResponsePointer response;

            if (request->requiresResponse()) {
                request->mutex().lock();
            }

            m_queue.add(request);

            if (request->requiresResponse()) {
                request->mutex().lock();
                response = request->response();
            }
        }

        std::string DatabaseThread::doRequest(const std::string &jsonRequest)
        {
            std::stringstream ss;
            ss << jsonRequest;
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(ss, pt);

            std::string command = pt.get<std::string>("command");
            DatabaseHandler *handler = DatabaseHandler::findCommand(command);
            if (!handler)
                return std::string();

            RequestPointer request(handler->getRequest(pt.get_child("data")));
            ResponsePointer response(doRequest(request));
            return response->response();
        }
    }
}

