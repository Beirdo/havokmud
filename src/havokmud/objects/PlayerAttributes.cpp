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
            boost::regex regex("([\"])");
            const char *format = "(?1\\\")";

            std::string outString = boost::regex_replace(inString, regex,
                                                         format);
            return outString;
        }

        std::string desanitize(const std::string &inString)
        {
            boost::regex regex("(\\\")");
            const char *format = "(?1\")";

            std::string outString = boost::regex_replace(inString, regex,
                                                         format);
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

            ptree root = pt.front().second;

            if (root.front().first != "") {
                // Single row - the front should be named "attribsrc"
                setFromJsonNode(root);
            } else {
                BOOST_FOREACH(ptree::value_type &row, root) {
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

            if (source.empty() || jsonAttribs.empty())
                return;

            std::stringstream ss;
            ss << desanitize(jsonAttribs);
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return;
            }

            BOOST_FOREACH(ptree::value_type &v, pt) {
                set<std::string>(source, v.first, v.second.data());
            }
        }

        boost::regex PlayerAttributes::s_saveRegex("(.*?)[.].*?");

        void PlayerAttributes::save()
        {
            // Save the player attributes to the database
            std::string jsonRequest = "{\"command\":\"expire attributes\", "
                                      "\"data\":{\"pc_id\":"
                                    + std::to_string(m_player->id()) + "}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);

            std::set<std::string> sourceSet;

            BOOST_FOREACH(ptree::value_type &v, m_attributeTree) {
                boost::smatch match;
                if (boost::regex_match(v.first, match, s_saveRegex)) {
                    std::string source(match[1].first, match[1].second);

                    sourceSet.insert(source);
                }
            }

            BOOST_FOREACH(const std::string &source, sourceSet) {
                std::stringstream ss;
                ptree attribTree = m_attributeSourceTree.get_child(source);
                boost::property_tree::write_json(ss, attribTree, false);
                std::string attribJson;
                ss >> attribJson;

                jsonRequest = "{\"command\":\"save attributes\", \"data\":{"
                              "\"pc_id\":" + std::to_string(m_player->id())
                            + ", \"attribsrc\":\"" + source + "\", "
                              "\"attribjson\":\"" + sanitize(attribJson)
                            + "\"}}";
                results = g_databaseThread->doRequest(jsonRequest);
            }

            jsonRequest = "{\"command\":\"purge attributes\", "
                          "\"data\":{\"pc_id\":"
                        + std::to_string(m_player->id()) + "}}";
            results = g_databaseThread->doRequest(jsonRequest);
        }
    }
}

