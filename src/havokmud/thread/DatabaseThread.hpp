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
 * @brief Thread to handle database requests
 */

#ifndef __havokmud_thread_DatabaseThread__
#define __havokmud_thread_DatabaseThread__

#include <string>

#include "thread/HavokThread.hpp"
#include "objects/LockingQueue.hpp"

namespace havokmud {
    namespace thread {

        using havokmud::thread::HavokThread;
        using havokmud::objects::LockingQueue;

        class DatabaseThread : public HavokThread
        {
        public:
            DatabaseThread();
            ~DatabaseThread()  {};

            virtual void start();
            void handle_stop()  { m_abort = true; };

        private:
            bool                m_abort;
            LockingQueue<boost::shared_ptr<DatabaseRequest> > m_queue;
        };
    }
}

extern havokmud::thread::DatabaseThread *g_databaseThread;

#endif  // __havokmud_thread_DatabaseThread__
