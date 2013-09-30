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

#ifndef __havokmud_objects_LoggingSink__
#define __havokmud_objects_LoggingSink__

#include <string>

#include "objects/LoggingItem.hpp"

#define DEBUG_FILE "/var/log/havokmud.log"

extern bool g_debug;

namespace havokmud {
    namespace objects {

        typedef enum { Console, Syslog, File } LogSinkType;

        class LoggingSink
        {
        public:
            LoggingSink(LogSinkType type);
            virtual ~LoggingSink()  {};

            LogSinkType type() const  { return m_type; };
            virtual bool operator==(const LoggingSink &b) = 0;
            virtual void outputItem(LoggingItem *item) = 0;
        private:
            virtual bool open() = 0;
            virtual void close() = 0;
        protected:
            LogSinkType         m_type;
            bool                m_open;
            static std::string  s_bg;
            static std::string  s_fg;
        };

        class StdoutLoggingSink : public LoggingSink
        {
        public:
            StdoutLoggingSink() : LoggingSink(Console), m_fd(1) {};
            virtual ~StdoutLoggingSink()  { if (m_open) close(); };

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open()  { m_open = true; return true; };
            virtual void close()  { m_open = false; };

            int m_fd;
        };

        class FileLoggingSink : public LoggingSink
        {
        public:
            FileLoggingSink(std::string filename) : LoggingSink(File), m_fd(-1),
                    m_filename(filename)  {};
            virtual ~FileLoggingSink()  { if (m_open) close(); };

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open();
            virtual void close();

            int m_fd;
            std::string m_filename;
        };

#ifndef __CYGWIN__
        class SyslogLoggingSink : public LoggingSink
        {
        public:
            SyslogLoggingSink(int facility) : LoggingSink(Syslog),
                    m_facility(facility)  {};
            virtual ~SyslogLoggingSink()  { if (m_open) close(); };

            virtual bool operator==(const LoggingSink &b);
            virtual void outputItem(LoggingItem *item);
        private:
            virtual bool open();
            virtual void close();

            int m_facility;
        };
#endif  // __CYGWIN__
    }
}

#endif  // __havokmud_objects_LoggingSink__
