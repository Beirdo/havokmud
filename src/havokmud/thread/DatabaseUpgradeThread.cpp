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
 * @brief Thread to handle database upgrades 
 */

#include <boost/foreach.hpp>
#include <string>

#include "thread/DatabaseUpgradeThread.hpp"
#include "thread/DatabaseThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"
#include "util/misc.hpp"

namespace havokmud {
    namespace thread {

        using boost::property_tree::ptree;

        DatabaseUpgradeThread::DatabaseUpgradeThread(const DatabaseSchema &schema_) :
                HavokThread("DatabaseUpgrade"),
                m_schema(schema_)
        {
            pro_initialize<DatabaseUpgradeThread>();
        }

        void DatabaseUpgradeThread::start()
        {
            m_setting = "dbSchema." + m_schema.name();
            int currentVersion = g_settings.get<int>(m_setting);
            LogPrint(LG_INFO, "Database %s schema version %d, supported %d",
                     m_schema.name().c_str(), currentVersion,
                     m_schema.supportedVersion());
            if (currentVersion <= 0) {
                LogPrint(LG_INFO, "Installing %s schema version %d",
                         m_schema.name().c_str(), m_schema.supportedVersion());
                BOOST_FOREACH(const std::string &query, m_schema.baseSchema()) {
                    RequestPointer request(new DatabaseRequest(query));
                    g_databaseThread->doRequest(request);
                }
                g_settings.set<int>(m_setting, m_schema.supportedVersion());
            } else {
                while (currentVersion < m_schema.supportedVersion())
                {
                    LogPrint(LG_INFO, "Upgrading %s schema to version %d",
                             m_schema.name().c_str(), currentVersion+1);
                    BOOST_FOREACH(const std::string &query,
                                  m_schema.upgradeItems()[currentVersion]) {
                        if (!query.empty()) {
                            RequestPointer request(new DatabaseRequest(query));
                            g_databaseThread->doRequest(request);
                        }
                    }
                    // Upgrade from current to current+1
                    currentVersion++;
                    g_settings.set<int>(m_setting, currentVersion);
                }
            }
            m_startupMutex.unlock();
        }
    }
}
