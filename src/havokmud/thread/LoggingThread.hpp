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

#include "util/misc.hpp"
#include "thread/HavokThread.hpp"
#include "objects/LoggingItem.hpp"
#include "objects/LoggingSink.hpp"
#include "objects/LockingQueue.hpp"

#define LogPrint(level, format, ...) \
    g_loggingThread->print(level, __FILE__, __LINE__, __FUNCTION__, \
                           (char *)format, ## __VA_ARGS__)

#define LogPrintNoArg(level, string) \
    g_loggingThread->print(level, __FILE__, __LINE__, __FUNCTION__, \
                           (char *)string)

namespace havokmud {
    namespace thread {

        using havokmud::thread::HavokThread;
        using havokmud::objects::LoggingItem;
        using havokmud::objects::LoggingSink;

        class LoggingThread : public HavokThread
        {
        public:
            LoggingThread();
            ~LoggingThread()  {};

            void print(int level, std::string file, int line,
                       std::string function, std::string format, ...);

            void add(LoggingSink *sink) { m_sinks.insert(sink); };
            void remove(LoggingSink *sink);

            virtual void start();
            void handle_stop();

        private:
            void outputItem(LoggingItem *item);

            std::set<LoggingSink *> m_sinks;
            havokmud::objects::LockingQueue<LoggingItem *> m_logQueue;
            bool m_abort;
        };
    }
}

extern havokmud::thread::LoggingThread *g_loggingThread;

/* Define the log levels (lower number is higher priority) */

typedef enum
{
    LG_EMERG = 0,
    LG_ALERT,
    LG_CRIT,
    LG_ERR,
    LG_WARNING,
    LG_NOTICE,
    LG_INFO,
    LG_DEBUG,
    LG_UNKNOWN
} LogLevel;

#ifdef _LogLevelNames_
std::string g_LogLevelNames[] =
{
    "LG_EMERG",
    "LG_ALERT",
    "LG_CRIT",
    "LG_ERR",
    "LG_WARNING",
    "LG_NOTICE",
    "LG_INFO",
    "LG_DEBUG",
    "LG_UNKNOWN"
};
int g_LogLevelNameCount = NELEMS(g_LogLevelNames);
#else
extern std::string g_LogLevelNames[];
extern int g_LogLevelNameCount;
#endif
extern LogLevel g_LogLevel;

#endif  // __havokmud_thread_LoggingThread__
