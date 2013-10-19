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
#include <cppconn/exception.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/thread.hpp>

#include "thread/DatabaseThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"
#include "objects/Email.hpp"
#include "corefunc/DatabaseHandler.hpp"
#include "util/sanitize.hpp"

namespace havokmud {
    namespace thread {

        using havokmud::corefunc::DatabaseHandler;
        using boost::property_tree::ptree;
        using havokmud::util::sanitize;

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

            LogPrint(LG_INFO, "Using database %s", m_database.c_str());
            m_connection->setSchema(m_database);

            DatabaseHandler::initialize();
            m_startupMutex.unlock();

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
            ResponsePointer response = ResponsePointer();

            //LogPrint(LG_INFO, "handle query: %s, %d, %d",
            //         request->query().c_str(), request->requiresResponse(),
            //         request->requiresInsertId());

            boost::shared_ptr<sql::Statement>
                    statement(m_connection->createStatement());

            int rowCount = 0;
            std::string jsonResult("");
            try {
                boost::shared_ptr<sql::ResultSet>
                        resultSet(statement->executeQuery(request->query()));

                if (request->requiresResponse()) {
                    sql::ResultSetMetaData *resultMetadata =
                            resultSet->getMetaData();

                    rowCount = resultSet->rowsCount();
                    int columnCount = resultMetadata->getColumnCount();

                    std::vector<std::string> columnNames;
                    for (int i = 1; i <= columnCount; i++) {
                        columnNames.push_back(resultMetadata->getColumnName(i));
                    }

                    if (rowCount != 1)
                        jsonResult += "[";
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
                    if (rowCount != 1)
                        jsonResult += "]";
                    //LogPrint(LG_INFO, "Result: %s", jsonResult.c_str());
                }
            } catch (sql::SQLException &e) {
                // This should happen on UPDATEs and INSERTs
                if (e.getErrorCode() != 0 || e.getSQLState() != "00000") {
                    /*
                    The MySQL Connector/C++ throws three different exceptions:

                    - sql::MethodNotImplementedException
                        (derived from sql::SQLException)
                    - sql::InvalidArgumentException
                        (derived from sql::SQLException)
                    - sql::SQLException (derived from std::runtime_error)
                    */
                    // Use what(), getErrorCode() and getSQLState()
                    LogPrint(LG_CRIT, "MySQL err: %s (MySQL error code: %d, "
                                      "SQLState: %s)", e.what(),
                            e.getErrorCode(), e.getSQLState().c_str());

                    response.reset(new DatabaseResponse(e.what(),
                            e.getErrorCode(), e.getSQLState()));
                    request->setResponse(response);
                    if (request->requiresResponse())
                        request->mutex().unlock();
                    return;
                }
            }

            int insertId = -1;
            if (request->requiresInsertId()) {
                boost::shared_ptr<sql::Statement>
                        statement2(m_connection->createStatement());
                boost::shared_ptr<sql::ResultSet>
                        resultSet2(statement2->executeQuery("SELECT LAST_INSERT_ID();"));

                resultSet2->next();
                insertId = resultSet2->getInt(1);
                //LogPrint(LG_INFO, "insertId: %d", insertId);
            }

            if (request->requiresResponse() || request->requiresInsertId()) {
                response.reset(new DatabaseResponse(jsonResult, insertId));
                request->setResponse(response);
            }

            if (!request->chainCommand().empty())
            {
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
                    //LogPrint(LG_INFO, "New command: %s", newCommand.c_str());

                    DatabaseHandler *handler =
                            DatabaseHandler::findCommand(newCommand);
                    if (handler) {
                        RequestPointer
                            chainRequest(handler->getRequest(request->data()));
                        handleRequest(chainRequest);
                        ResponsePointer chainResponse =
                            chainRequest->response();
                        request->setResponse(chainResponse);
                    }
                }
            }

            if (request->requiresResponse()) {
                request->mutex().unlock();
            }
        }

        boost::regex DatabaseThread::s_chainRegex("(.*?):(.*?):(.*?)");

        ResponsePointer DatabaseThread::doRequest(RequestPointer request)
        {
            ResponsePointer resp = ResponsePointer();

            if (request->requiresResponse()) {
                request->mutex().lock();
            }

            m_queue.add(request);

            if (request->requiresResponse()) {
                request->mutex().lock();
                resp = request->response();
            }

            return resp;
        }

        std::string DatabaseThread::doRequest(const std::string jsonRequest)
        {
            //LogPrint(LG_INFO, "JSON query: %s", jsonRequest.c_str());

            std::stringstream ss;
            ss << jsonRequest;
            boost::shared_ptr<ptree> pt(new ptree);
            boost::property_tree::read_json(ss, *pt);

            std::string command = pt->get<std::string>("command");
            DatabaseHandler *handler = DatabaseHandler::findCommand(command);
            if (!handler)
                return std::string();

            boost::shared_ptr<ptree> dataPtr(new ptree(pt->get_child("data")));
            RequestPointer req(handler->getRequest(dataPtr));
            ResponsePointer resp = doRequest(req);
            if (!resp)
                return std::string();

            std::string strResponse = resp->response();
            if (strResponse.empty() || strResponse == "[]") {
                int insertId = resp->insertId();
                //LogPrint(LG_INFO, "Final: insertId: %d", insertId);
                if (insertId != -1) {
                    strResponse = "{\"insertId\":" + std::to_string(insertId)
                                + "}";
                } else if (!resp->errMsg().empty()) {
                    strResponse = "{\"error\":{\"message\":\""
                                + sanitize(resp->errMsg()) + "\",\"code\":"
                                + std::to_string(resp->errCode()) + ","
                                + "\"state\":\"" + resp->sqlState() + "\"}}";
                }
            }
            return strResponse;
        }
    }
}

