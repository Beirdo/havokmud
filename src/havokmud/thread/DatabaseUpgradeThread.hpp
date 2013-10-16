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

#ifndef __havokmud_thread_DatabaseUpgradeThread__
#define __havokmud_thread_DatabaseUpgradeThread__

#include <string>
#include <cppconn/driver.h>
#include <cppconn/connection.h>

#include "thread/HavokThread.hpp"
#include "objects/LockingQueue.hpp"
#include "objects/DatabaseResponse.hpp"
#include "objects/DatabaseRequest.hpp"

#if 0
namespace havokmud {
    namespace thread {

        using havokmud::thread::HavokThread;

        typedef boost::shared_ptr<DatabaseRequest>  RequestPointer;
        typedef boost::shared_ptr<DatabaseResponse> ResponsePointer;

        class DatabaseUpgradeThread : public HavokThread
        {
        public:
            DatabaseUpgradeThread(int supportedVersion);
            ~DatabaseUpgradeThread()  {};

            virtual void start();
            void handle_stop()  {};

        private:
            int m_supportedVersion;
        };
    }
}
#endif

#endif  // __havokmud_thread_DatabaseUpgradeThread__
