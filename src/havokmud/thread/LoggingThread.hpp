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
 * @brief Thread to handle logging
 */

#ifndef __havokmud_thread_LoggingThread__
#define __havokmud_thread_LoggingThread__

#include <string>

#include "thread/HavokThread.hpp"


#define LogPrint(level, format, ...) \
    g_loggingThread.print(level, __FILE__, __LINE__, __FUNCTION__, \
                             format, ## __VA_ARGS__)

#define LogPrint(level, string) \
    g_loggingThread.print(level, __FILE__, __LINE__, __FUNCTION__, string)

namespace havokmud {
    namespace thread {

        using havokmud::thread::HavokThread;

        class LoggingSink;

        class LoggingThread : public HavokThread
        {
        public:
            LoggingThread() : HavokThread("Logging")  {};
            ~LoggingThread()  {};

            void print(int level, char *file, int line, const char *function,
                       char *format, ...);

            bool add(LoggingSink *sink) { m_sinks.insert(sink); };
            bool remove(LoggingSink *sink)  { m_sinks.remove(sink); };

            void handle_stop();

        private:
            virtual void prv_start();

            std::set<LoggingSink *> m_sinks;
            std::queue<LoggingItem> m_logQueue;
        };

        extern LoggingThread g_loggingThread;
    }
}

#endif  // __havokmud_thread_LoggingThread__
