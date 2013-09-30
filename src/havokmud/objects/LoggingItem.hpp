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
 * @brief Logging Items
 */

#ifndef __havokmud_thread_LoggingItem__
#define __havokmud_thread_LoggingItem__

#include <string>
#include <boost/thread.hpp>

#include "thread/ThreadColors.hpp"
#include "thread/HavokThread.hpp"

namespace havokmud {
    namespace objects {

        using havokmud::thread::HavokThread;

        class LoggingItem
        {
        public:
            LoggingItem(int level, std::string &file, int line,
                        std::string &function, std::string message) :
                    m_level(level), m_file(file), m_line(line),
                    m_function(function), m_message(message)
            {
                gettimeofday(&m_epochtime, NULL);

                char timestamp[22];
                time_t sec = (time_t)m_epochtime.tv_sec;
                struct tm ts;
                localtime_r((const time_t *)&sec, &ts);
                strftime(timestamp, 21, "%Y-%b-%d %H:%M:%S",
                         (const struct tm *)&ts);
                m_timestamp = std::string(timestamp);
                m_timestamp_us = m_epochtime.tv_usec;

                boost::thread::id threadId = boost::this_thread::get_id();
                HavokThread *thread = g_threadMap.findThread(threadId);
                if (!thread) {
                    m_threadName = "unknown";
                    m_background = g_defaultColor.background();
                    m_foreground = g_defaultColor.foreground();
                } else {
                    m_threadName = thread->name();
                    m_background = thread->background();
                    m_foreground = thread->foreground();
                }
            };

            ~LoggingItem()  {};

            int level() const  { return m_level; };
            const std::string &file() const  { return m_file; };
            int line() const  { return m_line; };
            const std::string &function() const  { return m_function; };
            const std::string &message() const  { return m_message; };
            const std::string &timestamp() const  { return m_timestamp; };
            int timestamp_us() const { return m_timestamp_us; };
            const std::string &threadName() const  { return m_threadName; };
            const std::string &background() const  { return m_background; };
            const std::string &foreground() const  { return m_foreground; };

        private:
            int                 m_level;
            std::string         m_file;
            int                 m_line;
            std::string         m_function;
            std::string         m_message;
            struct timeval      m_epochtime;
            std::string         m_timestamp;
            int                 m_timestamp_us;
            std::string         m_threadName;
            std::string         m_background;
            std::string         m_foreground;
        };
    }
}

#endif  // __havokmud_thread_LoggingItem__
