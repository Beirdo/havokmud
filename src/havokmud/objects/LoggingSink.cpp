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
 * @brief Logging sinks
 */

#include <string>
#ifndef __CYGWIN__
#include <syslog.h>
#endif  // __CYGWIN__
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <boost/format.hpp>

#include "objects/LoggingSink.hpp"
#include "objects/ThreadMap.hpp"
#include "thread/LoggingThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/ThreadColors.hpp"

namespace havokmud {
    namespace objects {
        std::string LoggingSink::s_bg;
        std::string LoggingSink::s_fg;

        LoggingSink::LoggingSink(LogSinkType type) : m_type(type),
                m_open(false)
        {
            if (s_bg.empty() || s_fg.empty()) {
                s_bg = g_defaultColor.background();
                s_fg = g_defaultColor.foreground();
            }
        }

        bool StdoutLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type() != Console)
                return false;

            const StdoutLoggingSink *other =
                    dynamic_cast<const StdoutLoggingSink *>(&b);
            return (!(!other));
        }

        void StdoutLoggingSink::outputItem(LoggingItem *item)
        {
            if (!m_open && !open()) {
                g_loggingThread->remove(this);
                return;
            }

            std::string line = item->background() + item->foreground()
                    + item->timestamp() + "  " + item->message()
                    + s_bg + s_fg + "\n";

            int result = ::write(m_fd, line.c_str(), line.length());

            if (result == -1) {
                LogPrint(LG_UNKNOWN,
                         "Closed Log output on fd %d due to errors", m_fd);
                close();
                g_loggingThread->remove(this);
            }
        }


        bool FileLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type() != File)
                return false;

            const FileLoggingSink *other =
                    dynamic_cast<const FileLoggingSink *>(&b);
            if (!other)
                return false;

            return (other->m_filename == m_filename);
        }

        void FileLoggingSink::outputItem(LoggingItem *item)
        {
            if (!m_open && !open()) {
                g_loggingThread->remove(this);
                return;
            }

            std::string line = item->background() + item->foreground()
                 + item->timestamp()
                 + str(boost::format(".%06d") % item->timestamp_us())
                 + "  " + item->threadName() + " " + item->file() + ":"
                 + std::to_string(item->line()) + " (" + item->function()
                 + ") - " + item->message() + s_bg + s_fg + "\n";

            int result = ::write(m_fd, line.c_str(), line.length());

            if (result == -1) {
                LogPrint(LG_UNKNOWN,
                         "Closed Log output on fd %d due to errors", m_fd);
                close();
                g_loggingThread->remove(this);
            }
        }

        bool FileLoggingSink::open()
        {
            if (m_filename.empty())
                return false;

            m_fd = ::open(m_filename.c_str(),
                          O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK,
                          S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH );

            if (m_fd == -1)
            {
                /* Couldn't open the log file.  Gak! */
                LogPrint(LG_CRIT, "Couldn't open %s: %s", m_filename.c_str(),
                         strerror(errno));
                return false;
            }

            LogPrint(LG_INFO, "Added log file: %s", m_filename.c_str());

            m_open = true;
            return true;
        }

        void FileLoggingSink::close()
        {
            if (m_fd != -1)
                ::close(m_fd);
            m_open = false;
        }


#ifndef __CYGWIN__
        bool SyslogLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type() != Syslog)
                return false;

            const SyslogLoggingSink *other =
                    dynamic_cast<const SyslogLoggingSink *>(&b);
            if (!other)
                return false;

            return (other->m_facility == m_facility);
        }

        void SyslogLoggingSink::outputItem(LoggingItem *item)
        {
            if (!m_open && !open()) {
                g_loggingThread->remove(this);
                return;
            }

            ::syslog(item->level(), "%s", item->message().c_str());
        }

        bool SyslogLoggingSink::open()
        {
            ::openlog("havokmud", LOG_NDELAY | LOG_PID, m_facility);

            m_open = true;
            return true;
        }

        void SyslogLoggingSink::close()
        {
            ::closelog();

            m_open = false;
        }
#endif  // __CYGWIN__
    }
}

