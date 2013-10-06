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
 * @brief Thread to handle input - base class for Login/Playing
 */

#ifndef __havokmud_thread_InputThread__
#define __havokmud_thread_InputThread__

#include <string>

#include "thread/HavokThread.hpp"
#include "objects/LockingQueue.hpp"

namespace havokmud {
    namespace corefunc {
        class LoginStateMachine;
    }

    namespace objects {
        class Connection;
    }

    namespace thread {

        using havokmud::thread::HavokThread;
        using havokmud::objects::Connection;
        using havokmud::corefunc::LoginStateMachine;

        typedef std::pair<Connection *, const std::string &> InputQueueItem;
        typedef std::map<Connection *, LoginStateMachine *> ConnectionMap;

        class InputThread : public HavokThread
        {
        public:
            InputThread(const std::string &name) : HavokThread(name),
                    m_abort(false)  {};
            virtual ~InputThread()  {};

            virtual void start() = 0;
            void handle_stop();

            void enqueueInput(Connection *connection, const std::string &line);
            virtual void removeConnection(Connection *connection) = 0;

        protected:
            havokmud::objects::LockingQueue<InputQueueItem *> m_inQueue;
            bool m_abort;
        };
    }
}

#endif  // __havokmud_thread_InputThread__
