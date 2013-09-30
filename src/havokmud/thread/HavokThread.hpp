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

#ifndef __havokmud_thread_HavokThread__
#define __havokmud_thread_HavokThread__

#include <boost/thread/thread.hpp>
#include <string>

#include "thread/ThreadColors.hpp"
#include "thread/ThreadMap.hpp"
#include "util/misc.hpp"

#define DEFAULT_STACK_SIZE 1 * 1024 * 1024  // 1MB

namespace havokmud {
    namespace thread {
        class HavokThread
        {
        public:
            HavokThread(std::string name) : m_name(name)  {};
            virtual ~HavokThread()
            {
                g_threadMap.removeThread(this);
            }

            const std::string &name() const { return m_name; };
            const boost::thread::id &id() const { return m_id; };
            const int index() const { return m_index; };

            const std::string &foreground() const
                { return m_color.foreground(); };
            const std::string &background() const
                { return m_color.background(); };
            const int foregroundNum() const
                { return m_color.foregroundNum(); };
            const int backgroundNum() const
                { return m_color.backgroundNum(); };

            bool joinable()  { return m_thread.joinable(); };
            void join()  { m_thread.join(); };
            virtual void start() = 0;

	    protected:
            template<class ThreadClass> void pro_initialize()
            {
                m_attrs.set_stack_size(DEFAULT_STACK_SIZE);
                m_thread = boost::thread(m_attrs,
                         boost::bind(&ThreadClass::start,
                                     dynamic_cast<ThreadClass *>(this)));
                //m_joiner = boost::thread_joiner(m_thread);
                m_id = m_thread.get_id();
                m_index = g_threadMap.addThread(this);
            };

            boost::thread::attributes   m_attrs;
            boost::thread               m_thread;
            // boost::thread_joiner        m_joiner;
            boost::thread::id           m_id;
            int                         m_index;

            std::string                 m_name;
            ThreadColors                m_color;
        };
    }
}

extern boost::thread::id g_mainThreadId;

#endif  // __havokmud_thread_HavokThread__
