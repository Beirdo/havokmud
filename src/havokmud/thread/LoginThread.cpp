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
 * @brief Thread to handle logins
 */

#include <string>
#include <stdarg.h>

#include "corefunc/Logging.hpp"
#include "thread/LoginThread.hpp"
#include "corefunc/LoginStateMachine.hpp"

namespace havokmud {
    namespace thread {

        using havokmud::corefunc::LoginStateMachine;

        LoginThread::LoginThread() : InputThread("Login")
        {
            pro_initialize<LoginThread>();

            LoginStateMachine::initialize();
        }
        

        void LoginThread::start()
        {
            while (!m_abort) {
                LogPrint(LG_INFO, "Getting input item");
                InputQueueItem *item = m_inQueue.get();
                if (!item) {
                    continue;
                }

                Connection *connection = item->first;
                std::string line = std::string(item->second);
                // delete item;

                LogPrint(LG_INFO, "Input from connection %d - %s",
                         connection->id(), line.c_str());
                LoginStateMachine *machine = NULL;

                LoginConnectionMap::iterator it =
                        m_connectionMap.find(connection);
                if (it == m_connectionMap.end()) {
                    machine = new LoginStateMachine(g_loginStateMachine,
                                                    connection);
                    m_connectionMap.insert(std::pair<Connection *,
                                    LoginStateMachine *>(connection, machine));
                } else {
                    machine = it->second;
                }

                LogPrint(LG_INFO, "Connection %d, State Machine %p",
                         connection->id(), machine);

                if (machine->handleLine(line)) {
                    LogPrint(LG_INFO, "Entering play mode");
                    removeConnection(connection);
                    connection->enterPlaying();
                }
            }
        }

        void LoginThread::removeConnection(Connection *connection)
        {
            LogPrint(LG_INFO, "Removing connection %d", connection->id());
            LoginConnectionMap::iterator it = m_connectionMap.find(connection);
            if (it == m_connectionMap.end())
                return;

            delete it->second;
            m_connectionMap.erase(it);
        }
    }
}
