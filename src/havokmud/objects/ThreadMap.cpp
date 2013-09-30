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

#include "thread/HavokThread.hpp"
#include "objects/ThreadMap.hpp"
#include "thread/LoggingThread.hpp"
#include "objects/ThreadColors.hpp"

havokmud::objects::ThreadMap g_threadMap;

namespace havokmud {
    namespace objects {
        int ThreadMap::addThread(havokmud::thread::HavokThread *thread)
        {
            if (!thread)
                return -1;

            int index = m_nextId++;
            m_idMap.insert(std::pair<int, boost::thread::id>(index,
                                                             thread->id()));
            m_map.insert(std::pair<int, havokmud::thread::HavokThread *>(index, thread));

            LogPrint(LG_INFO, "Added Thread %d as \"%s%s%s%s%s\" (%d/%d)",
                     index,
                     thread->background().c_str(), thread->foreground().c_str(),
                     thread->name().c_str(),
                     g_defaultColor.background().c_str(),
                     g_defaultColor.foreground().c_str(),
                     thread->backgroundNum(), thread->foregroundNum());

            return index;
        }

        void ThreadMap::removeThread(havokmud::thread::HavokThread *thread)
        {
            if (!thread)
                return;

            LogPrint(LG_INFO, "Removed thread  %d: %s", thread->index(),
                     thread->name().c_str());

            m_map.erase(thread->index());
            m_idMap.erase(thread->index());
        }

        havokmud::thread::HavokThread *ThreadMap::findThread(boost::thread::id threadId)
        {
            int index = -1;

            for (ThreadIdMapType::iterator it = m_idMap.begin();
                 it != m_idMap.end(); ++it) {
                if (it->second == threadId) {
                    index = it->first;
                    break;
                }
            }

            ThreadMapType::iterator it2;
            it2 = m_map.find(index);
            if (it2 == m_map.end())
                return NULL;

            return it2->second;
        }
    }
}

