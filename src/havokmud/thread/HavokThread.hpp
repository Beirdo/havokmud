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

#include "ThreadColors.hpp"

#define NELEMS(x)   (sizeof((x)) / sizeof((x)[0]))

namespace havokmud {
    namespace thread {
        class HavokThread
        {
        public:
            HavokThread(std::string name);
            virtual ~HavokThread();

            const std::string &name() const { return m_name; };
            const boost::thread::id id() const { return m_id; };

            const std::string &foreground() const
                { return m_color.foreground(); };
            const std::string &background() const
                { return m_color.foreground(); };
            const int foregroundNum() const
                { return m_color.foregroundNum(); };
            const int backgroundNum() const
                { return m_color.backgroundNum(); };

            bool joinable()  { return m_thread.joinable(); };
            void join()  { m_thread.join(); };

        private:
            virtual void prv_start() = 0;
            void prv_setColor(int threadNum);

            boost::thread::attributes   m_attrs;
            boost::thread               m_thread;
            // boost::thread_joiner        m_joiner;
            boost::thread::id           m_id;

            std::string                 m_name;
            ThreadColors                m_color;
        };

        typedef std::map<boost::thread::id, HavokThread *> ThreadMapType;
        class ThreadMap : public ThreadMapType {
        public:
            void addThread(HavokThread *thread);
            void removeThread(HavokThread *thread);
            HavokThread *findThread(boost::thread::id threadId);
        };
    }
}

#endif  // __havokmud_thread_HavokThread__
