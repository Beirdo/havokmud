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
 * @brief HavokMud Thread base class
 */

#include "HavokThread.hpp"

namespace {
    static const int DEFAULT_STACK_SIZE = 1 * 1024 * 1024;  // 1MB
}

namespace havokmud {
    namespace thread {
        static ThreadMap g_threadMap;

        HavokThread::HavokThread(std::string name) : m_name(name)
        {
            m_attrs.set_size(DEFAULT_STACK_SIZE);
            m_thread = boost::thread(m_attrs,
                                     boost::bind(&HavokThread::prv_start,
                                                 this));
            m_joiner = boost::thread_joiner(m_thread);
            m_id = boot::this_thread::get_id();
            
            m_color = ThreadColors();
            g_threadMap.addThread(this);
        }

        HavokThread::~HavokThread()
        {
            g_threadMap.removeThread(this);
        }

        const std::string ThreadColors::s_backgroundColors[] = 
                { BK_BLACK, BK_LT_GRAY, BK_RED, BK_GREEN,
                  BK_BROWN, BK_BLUE, BK_MAGENTA, BK_CYAN };
        const std::string ThreadColors::s_foregroundColors[] =
                { FG_BLACK, FG_LT_GRAY, FG_RED, FG_GREEN,
                  FG_BROWN, FG_BLUE, FG_MAGENTA, FG_CYAN,
                  FG_DK_GRAY, FG_LT_RED, FG_LT_GREEN, FG_YELLOW,
                  FG_LT_BLUE, FG_LT_MAGENTA, FG_LT_CYAN, FG_WHITE };

        const Color ThreadColors::s_badColors[] =
                { std::make_pair(1, 0), std::make_pair(1, 10),
                  std::make_pair(1, 11) };
        const int ThreadColors::s_badColorCount =
                NELEMS(ThreadColors::s_badColors);

        Color ThreadColors::s_lastColor = std::make_pair(0, 0);
        ThreadColors g_defaultColor(0, 1);

        explicit ThreadColors::ThreadColors()
        {
            Color color = s_lastColor;
            int bg = color.first();
            int fg = color.second();
            bool badColor = true;

            while (badColor) {
                fg++;
                if (fg == 16) {
                    bg++;
                    fg = 0;
                }

                badColor = ((bg == fg) || (fg == 15 && bg == 1) ||
                            ((fg <= 14 && fg >= 9) && (bg == fg - 7)));

                for (int i = 0; i < s_badColorCount && !badColor; i++) {
                    badColor |= (s_badColors[i].first()  == bg &&
                                 s_badColors[i].second() == fg);
                }
            }
            color = std::make_pair(bg, fg);
            m_color = color;
            s_lastColor = color;
        }

        void ThreadMap::addThread(HavokThread *thread)
        {
            if (!thread)
                return;

            LogPrint(LOG_INFO, "Added Thread as \"%s%s%s%s%s\" (%d/%d)",
                     thread->background(), thread->foreground(), name,
                     g_defaultColor.background(), g_defaultColor.foreground(),
                     thread->backgroundNum(), thread->foregroundNum());
            insert(thread->id(), thread);
        }

        void ThreadMap::removeThread(HavokThread *thread)
        {
            if (!thread)
                return;

            erase(thread->id());

            LogPrint(LOG_INFO, "Removed thread: %s", thread->name());
        }

        HavokThread *ThreadMap::findThread(boost::thread::id threadId)
        {
            ThreadMapType::iterator it;
            it = find(threadId);
            if (it == end())
                return NULL;

            return it->second();
        }
    }
}

