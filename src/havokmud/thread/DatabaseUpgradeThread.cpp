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

#include <string>

#include "thread/DatabaseUpgradeThread.hpp"
#include "thread/DatabaseThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"

#if 0
namespace havokmud {
    namespace thread {

        DatabaseUpgradeThread::DatabaseUpgradeThread(int supportedVersion) :
                HavokThread("DatabaseUpgrade"),
                m_supportedVersion(supportedVersion)
        {
            pro_initialize<DatabaseUpgradeThread>();
        }

        void DatabaseThread::start()
        {
            int currentVersion = g_settings.get<int>("dbSchema");
            LogPrint(LG_INFO, "Database schema version %d, supported %d",
                     currentVersion, m_supportedVersion);
            while (currentVersion < m_supportedVersion)
            {
                // Upgrade from current to current+1
                currentVersion++;
                g_setting.set<int>("dbSchema", currentVersion);
            }
            m_startupMutex.unlock();
        }
    }
}
#endif
