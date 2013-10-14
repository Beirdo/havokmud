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
 * @brief Account object
 */

#include <sstream>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include "objects/Settings.hpp"
#include "corefunc/Logging.hpp"
#include "thread/DatabaseThread.hpp"

namespace havokmud {
    namespace objects {

        Settings::Settings()
        {
        }

        bool Settings::load(const std::string &setting)
        {
            // Database load of setting
            std::string json = "{\"command\":\"get setting\", "
                               "\"data\":{\"name\":\"" + setting + "\"}}";
            std::string results = g_databaseThread->doRequest(json);
            //LogPrint(LG_INFO, "Results: %s", results.c_str());

            std::stringstream ss;
            ss << results;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return false;
            }

            std::string value(pt.get<std::string>("value", std::string()));
            if (!value.empty()) {
                set<std::string>(setting, value, false);
                return true;
            }
            return false;
        }

        void Settings::save(const std::string &setting)
        {
            // Database save of setting
            std::string json = "{\"command\":\"set setting\", "
                               "\"data\":{\"name\":\"" + setting + "\","
                               "\"value\":\"" + get<std::string>(setting)
                             + "\"}}";
            std::string results = g_databaseThread->doRequest(json);
        }
    }
}

havokmud::objects::Settings g_settings;

