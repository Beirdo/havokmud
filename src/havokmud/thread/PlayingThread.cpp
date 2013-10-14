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
 * @brief Thread to handle playing
 */

#include <string>

#include "corefunc/Logging.hpp"
#include "thread/PlayingThread.hpp"

namespace havokmud {
    namespace thread {

        PlayingThread::PlayingThread(bool immortal) :
                InputThread(immortal ? "ImmortalPlaying" : "Playing"),
                m_immortal(immortal)
        {
            pro_initialize<PlayingThread>();
        }
        

        void PlayingThread::start()
        {
            m_startupMutex.unlock();
            while (!m_abort) {
                InputQueueItem *item = m_inQueue.get();
                if (!item) {
                    continue;
                }

                boost::shared_ptr<Connection> connection = item->first;
                std::string line = item->second;

                LogPrint(LG_INFO, "Recieved: connection %d - line: %s",
                         connection->id(), line.c_str());

#if 0
                ConnectionMap::iterator it = m_connectionMap.find(connection);
                if (it == m_connectionMap.end()) {
                    machine = new LoginStateMachine(*g_loginStateMachine);
                    machine->setConnection(connection);
                    m_connectionMap.insert(std::pair<Connection *,
                                    LoginStateMachine *>(connection, machine));
                } else {
                    machine = it->second;
                }

                if (!machine->handleLine(line)) {
                    removeConnection(connection);
                    connection->enterPlaying();
                }
#endif

                delete item;
            }
        }

        void PlayingThread::removeConnection(boost::shared_ptr<Connection> connection)
        {
            LogPrint(LG_INFO, "Disconnect in playing thread");
#if 0
            ConnectionMap::iterator it = m_connectionMap.find(connection);
            if (it == m_connectionMap.end())
                return;

            delete it->second;
            m_connectionMap.erase(it);
#endif
        }
    }
}
