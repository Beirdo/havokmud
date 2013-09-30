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

namespace havokmud {
    namespace objects {

        class LoggingItem
        {
        public:
            LoggingItem(int level, std::string &file, int line,
                        std::string &function, std::string message) :
                    m_level(level), m_file(file), m_function(function),
                    m_message(message),
                    m_threadId(boost::this_thread::get_id())
            {
                gettimeofday(&m_timestamp, NULL);
            };

            ~LoggingItem()  {};

            int level() const  { return m_level; };
            std::string file() const  { return m_file; };
            int line() const  { return m_line; };
            std::string function() const  { return m_function; };
            std::string message() const  { return m_message; };
            const boost::thread::id &threadId() const  { return m_threadId; };
            struct timeval timestamp() const  { return m_timestamp; };

        private:
            int                 m_level;
            std::string         m_file;
            int                 m_line;
            std::string         m_function;
            std::string         m_message;
            boost::thread::id   m_threadId;
            struct timeval      m_timestamp;
        };
    }
}

#endif  // __havokmud_thread_LoggingItem__
