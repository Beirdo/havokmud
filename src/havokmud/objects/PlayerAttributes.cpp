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
 * @brief Player Attributes object
 */

#include <sstream>
#include <set>
#include <string>

#include "objects/Player.hpp"
#include "objects/PlayerAttributes.hpp"
#include "corefunc/Logging.hpp"
#include "thread/DatabaseThread.hpp"

namespace havokmud {
    namespace objects {

        std::string sanitize(const std::string &inString)
        {
            // Change all " to \\" for the database, gets saved as \", then
            // on readback, it goes back to ".  All fricking odd.
            boost::regex regex("([\"])");
            const char *format = "(?1\\\\\\\\\\\\\\\\\\\\\")";

            //LogPrint(LG_INFO, "In string: %s", inString.c_str());
            std::string outString = boost::regex_replace(inString, regex,
                                                         format,
                                                         boost::format_all);
            //LogPrint(LG_INFO, "Out string: %s", outString.c_str());
            return outString;
        }

        void PlayerAttributes::load()
        {
            // Load the player attributes from the database
            std::string jsonRequest = "{\"command\":\"load attributes\", "
                                      "\"data\":{\"pc_id\":"
                                    + std::to_string(m_player->id()) + "}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);

            if (results.empty())
                return;

            //LogPrint(LG_INFO, "Results: %s", results.c_str());
            std::stringstream ss;
            ss << results;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return;
            }

            if (pt.empty()) {
                return;
            }

            //LogPrint(LG_INFO, "Root: %s -> %s", pt.front().first.c_str(),
            //         pt.front().second.data().c_str());

            if (pt.front().first != "") {
                // Single row - the front should be named "attribsrc"
                setFromJsonNode(pt);
            } else {
                BOOST_FOREACH(ptree::value_type &row, pt) {
                    setFromJsonNode(row.second);
                }
            }
        }

        void PlayerAttributes::setFromJsonNode(const ptree &node)
        {
            std::string source = node.get<std::string>("attribsrc",
                                                       std::string());
            std::string jsonAttribs = node.get<std::string>("attribjson",
                                                            std::string());

            //LogPrint(LG_INFO, "source: %s, json: %s", source.c_str(),
            //         jsonAttribs.c_str());

            if (source.empty() || jsonAttribs.empty())
                return;

            std::stringstream ss;
            ss << jsonAttribs;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return;
            }

            BOOST_FOREACH(ptree::value_type &v, pt) {
                set<std::string>(v.first, source, v.second.data());
            }
        }

        void PlayerAttributes::save()
        {
            // Save the player attributes to the database
            std::string jsonRequest = "{\"command\":\"expire attributes\", "
                                      "\"data\":{\"pc_id\":"
                                    + std::to_string(m_player->id()) + "}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);

            BOOST_FOREACH(ptree::value_type &v, m_attributeSourceTree) {
                std::stringstream ss;
                boost::property_tree::write_json(ss, v.second, false);
                std::string attribJson;
                ss >> attribJson;

                //LogPrint(LG_INFO, "Source: %s, JSON: %s", v.first.c_str(),
                //         attribJson.c_str());
                jsonRequest = "{\"command\":\"save attributes\", \"data\":{"
                              "\"pc_id\":" + std::to_string(m_player->id())
                            + ", \"attribsrc\":\"" + v.first + "\", "
                              "\"attribjson\":\"" + sanitize(attribJson)
                            + "\"}}";
                //LogPrint(LG_INFO, "JSON: %s", jsonRequest.c_str());
                results = g_databaseThread->doRequest(jsonRequest);
            }

            jsonRequest = "{\"command\":\"purge attributes\", "
                          "\"data\":{\"pc_id\":"
                        + std::to_string(m_player->id()) + "}}";
            results = g_databaseThread->doRequest(jsonRequest);
        }
    }
}

