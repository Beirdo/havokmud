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

#include "objects/LoggingSink.hpp"

namespace havokmud {
    namespace objects {
        bool StdoutLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type != Console)
                return false;

            StdoutLoggingSink *other = dynamic_cast<StdoutLoggingSink *>&b;
            return (!(!other));
        }

        void StdoutLoggingSink::outputItem(LoggingItem *item)
        {
            std::string bg;
            std::string fg;

            HavokThread *thread = g_threadMap.findThread(item->threadId);
            if (!thread) {
                bg = "";
                fg = "";
            } else {
                bg = thread->backgroundColor();
                fg = thread->foregroundColor();
            }

            char timestamp[TIMESTAMP_MAX];
            time_t sec = (time_t)item->timestamp().tv_sec;
            localtime_r((const time_t *)&sec, &ts);
            strftime(timestamp, TIMESTAMP_MAX-8, "%Y-%b-%d %H:%M:%S",
                     (const struct tm *)&ts);

            std::string line = bg + fg + timestamp + item->message()
                    + s_bg + s_fg;

            int result = ::write(m_fd, line.c_str(), line.length());

            if (result == -1) {
                LogPrint(LOG_UNKNOWN,
                         "Closed Log output on fd %d due to errors", m_fd);
                close();
                g_loggingThread.remove(this);
            }
        }


        bool FileLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type != File)
                return false;

            FileLoggingSink *other = dynamic_cast<FileLoggingSink *>&b;
            if (!other)
                return false;

            return (other->m_filename == m_filename);
        }

        void FileLoggingSink::outputItem(LoggingItem *item)
        {
            std::string threadName;
            std::string bg;
            std::string fg;

            HavokThread *thread = g_threadMap.findThread(item->threadId);
            if (!thread) {
                threadName = "unknown";
                bg = "";
                fg = "";
            } else {
                threadName = thread->name();
                bg = thread->backgroundColor();
                fg = thread->foregroundColor();
            }

            char timestamp[TIMESTAMP_MAX];
            time_t sec = (time_t)item->timestamp().tv_sec;
            localtime_r((const time_t *)&sec, &ts);
            strftime(timestamp, TIMESTAMP_MAX-8, "%Y-%b-%d %H:%M:%S",
                     (const struct tm *)&ts);

            std::string line = bg + fg + timestamp
                 + boost::format(".%06d") % item->timestamp().tv_usec
                 + "  " + threadName + " " + item->file() + ":"
                 + std::to_string(item->line()) + " (" + item->function()
                 + ") - " + item->message() + s_bg + s_fg;

            int result = ::write(m_fd, line.c_str(), line.length());

            if (result == -1) {
                LogPrint(LOG_UNKNOWN,
                         "Closed Log output on fd %d due to errors", m_fd);
                close();
                g_loggingThread.remove(this);
            }
        }

        void FileLoggingSink::open()
        {
            if (m_filename.empty())
                return;

            m_fd = ::open(filename, O_WRONLY | O_CREAT | O_APPEND | O_NONBLOCK,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |
                                    S_IROTH );

            if( m_fd == -1 )
            {
                /* Couldn't open the log file.  Gak! */
                //LogPrint( LOG_CRIT, "Couldn't open %s: %s", filename, strerror(errno) );
                return;
            }

            //LogPrint( LOG_INFO, "Added log file: %s", filename );

            m_open = true;
        }

        void FileLoggingSink::close()
        {
            if (m_fd != -1)
                ::close(m_fd);
            m_open = false;
        }


        bool SyslogLoggingSink::operator==(const LoggingSink &b)
        {
            if (b.type != File)
                return false;

            SyslogLoggingSink *other = dynamic_cast<SyslogLoggingSink *>&b;
            if (!other)
                return false;

            return (other->m_facility == m_facility);
        }

        void SyslogLoggingSink::outputItem(LoggingItem *item)
        {
            ::syslog(item->level(), "%s", item->message());
        }

        void SyslogLoggingSink::open()
        {
            ::openlog("havokmud", LOG_NDELAY | LOG_PID, m_facility);

            m_open = true;
        }

        void SyslogLoggingSink::close()
        {
            ::closelog();

            m_open = false;
        }
    }
}

