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
 * @brief Locking Queue
 */

#ifndef __havokmud_objects_LockingQueue__
#define __havokmud_objects_LockingQueue__

#include <boost/thread.hpp>
#include <boost/thread/locks.hpp>
#include <queue>
#include <iostream>

namespace havokmud {
    namespace objects {

        template <class QueueItem>
        class LockingQueue
        {
        public:
            void add(QueueItem item)
            {
                boost::mutex::scoped_lock lock(m_mutex);
                m_queue.push(item);
                m_cond.notify_all();
            };

            QueueItem get()
            {
                boost::mutex::scoped_lock lock(m_mutex);
                while (m_queue.empty())
                {
                    m_cond.wait(lock);
                }

                QueueItem item = m_queue.front();
                m_queue.pop();

                return item;
            };
        private:
            std::queue<QueueItem> m_queue;
            boost::condition_variable m_cond;
            boost::mutex m_mutex;
        };
    }
}

#endif  // __havokmud_objects_LockingQueue__
