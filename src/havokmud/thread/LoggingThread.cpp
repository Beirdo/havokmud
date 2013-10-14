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

#include <string>
#include <stdarg.h>
#include <boost/thread.hpp>

#include "corefunc/Logging.hpp"
#include "thread/LoggingThread.hpp"
#include "objects/LoggingSink.hpp"
#include "objects/LoggingItem.hpp"

#ifndef __CYGWIN__
#include <syslog.h>
#endif // __CYGWIN__

using havokmud::objects::LoggingItem;

namespace havokmud {
    namespace thread {
        using havokmud::objects::StdoutLoggingSink;
        using havokmud::objects::FileLoggingSink;

#ifndef __CYGWIN__
        using havokmud::objects::SyslogLoggingSink;
#endif  // __CYGWIN__

        LoggingThread::LoggingThread() : HavokThread("Logging"), m_abort(false)
        {
            pro_initialize<LoggingThread>();
        }
        

        void LoggingThread::start()
        {
            add(new StdoutLoggingSink());
#ifndef __CYGWIN__
            add(new SyslogLoggingSink(LOG_LOCAL7));
#endif  // __CYGWIN__

            if (g_debug) {
                add(new FileLoggingSink(DEBUG_FILE));
            }

            m_startupMutex.unlock();

            while (!m_abort) {
                LoggingItem *item = g_logQueue.get();
                if (!item) {
                    continue;
                }

                outputItem(item);
            }

            std::for_each(m_sinks.begin(), m_sinks.end(),
                    boost::bind(&LoggingThread::remove, this, _1));
        }

        void LoggingThread::outputItem(LoggingItem *item)
        {
            if (item->level() <= g_LogLevel) {
                std::for_each(m_sinks.begin(), m_sinks.end(),
                        boost::bind(&LoggingSink::outputItem, _1, item));
            }
            delete item;
        }

        void LoggingThread::handle_stop()
        {
            m_abort = true;
        }

        void LoggingThread::remove(LoggingSink *sink)
        {
            std::set<LoggingSink *>::iterator it;
            for (it = m_sinks.begin(); it != m_sinks.end(); ++it) {
                if (**it == *sink) {
                    m_sinks.erase(*it);
                    delete *it;
                    break;
                }
            }
        }
    }
}
