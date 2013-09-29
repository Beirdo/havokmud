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
 * @brief Logging Sink base class
 */

#ifndef __havokmud_thread_LoggingSink__
#define __havokmud_thread_LoggingSink__

#include <string>

namespace havokmud {
    namespace objects {

        typedef enum { Console, Syslog, File } LogSinkType;

        class LoggingSink
        {
        public:
            LoggingSink(LogSinkType type) : m_type(type) : m_open(false) {};
            virtual ~LoggingSink()  { if (m_open) close(); };

            LogSinkType type() const  { return m_type; };
            virtual bool operator==(const LoggingSink &b) = 0;
            virtual void outputItem(LoggingItem *item) = 0;
        private:
            virtual bool open() = 0;
            virtual void close() = 0;
            LogSinkType         m_type;
            bool                m_open;
        };

        class StdoutLoggingSink : public LoggingSink
        {
        public:
            StdoutLoggingSink() : LoggingSink(Console), m_fd(1) {};
            virtual ~StdoutLoggingSink()  {};

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open()  { m_open = true; };
            virtual void close()  { m_open = false; };

            int m_fd;
        };

        class FileLoggingSink : public LoggingSink
        {
        public:
            FileLoggingSink(std::string filename) : LoggingSink(File), m_fd(-1),
                    m_filename(filename);
            virtual ~FileLoggingSink()  {};

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open();
            virtual void close();

            int m_fd;
            std::string m_filename;
        };

        class SyslogLoggingSink : public LoggingSink
        {
        public:
            SyslogLoggingSink() : LoggingSink(Syslog);
            virtual ~SyslogLoggingSink()  {};

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open();
            virtual void close();

            int m_facility;
        };
    }
}

#endif  // __havokmud_thread_LoggingSink__
