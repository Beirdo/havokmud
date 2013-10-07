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

#include <string>
#include <stdarg.h>

#include "corefunc/Logging.hpp"
#include "thread/InputThread.hpp"
#include "corefunc/LoginStateMachine.hpp"
#include "objects/Connection.hpp"

namespace havokmud {
    namespace objects {
        class Connection;
    }

    namespace thread {

        using havokmud::objects::Connection;

        void InputThread::handle_stop()
        {
            m_abort = true;
        }

        void InputThread::enqueueInput(Connection::pointer connection,
                                       const std::string &line)
        {
            InputQueueItem *item = new InputQueueItem(connection,
                                                      std::string(line));
            m_inQueue.add(item);
        }
    }
}
