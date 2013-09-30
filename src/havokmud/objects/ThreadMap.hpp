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
 * @brief Thread map
 */

#ifndef __havokmud_objects_ThreadMap__
#define __havokmud_objects_ThreadMap__

#include <boost/thread/thread.hpp>
#include <map>

namespace havokmud {
    namespace thread {
        class HavokThread;
    }
}

namespace havokmud {
    namespace objects {
        class ThreadMap {
            typedef std::map<int, havokmud::thread::HavokThread *> ThreadMapType;
            typedef std::map<int, boost::thread::id> ThreadIdMapType;
        public:
            ThreadMap() : m_nextId(0)  {};
            int addThread(havokmud::thread::HavokThread *thread);
            void removeThread(havokmud::thread::HavokThread *thread);
            havokmud::thread::HavokThread *findThread(boost::thread::id threadId);
        private:
            int             m_nextId;
            ThreadIdMapType m_idMap;
            ThreadMapType   m_map;
        };
    }
}

extern havokmud::objects::ThreadMap g_threadMap;

#endif  // __havokmud_objects_ThreadMap__
