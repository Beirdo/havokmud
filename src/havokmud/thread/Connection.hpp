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
 * @brief Thread to handle network connections.
 */

#ifndef __havokmud_thread_Connection__
#define __havokmud_thread_Connection__

#include <string>

namespace havokmud {
    namespace objects {

        class Player;

        class Connection
        {
        public:
            Connection(int fd, Player *player,
                       unsigned int inBufferSize = MAX_BUFSIZE,
                       unsigned int outBufferSize = MAX_BUFSIZE,
                       std::string ip) :
                m_fd(fd), m_player(player),
                m_inBuffer(boost::circular_buffer<char>(inBufferSize)),
                m_outBuffer(boost::circular_buffer<char>(outBufferSize)),
                m_hostname(ip)
            {
                // Force a power of two size
                assert(inBufferSize != 0 &&
                       !(inBufferSize & (inBufferSize - 1)));

                // Send off the hostname for resolution
                g_ResolveThread.resolve(m_hostname, 
                        boost::bind(&Connection::setHostname, this, _1));
            };
            ~Connection();

        private:
            void setHostname(std::string hostname) { m_hostname = hostname };

            int                             m_fd;
            Player                         *m_player;
            boost::circular_buffer<char>    m_inBuffer;
            boost::circular_buffer<char>    m_outBuffer;
            std::string                     m_hostName;
        };

        class ConnectionThread : public HavokThread
        {
        public:
            ConnectionThread(int port, struct timeval timeout) :
                    HavokThread("Connection"), m_port(port), m_count(0),
                    m_fdCount(0), m_timeout(timeout)  {};
            ~ConnectionThread();

        private:
            int                     m_port;
            int                     m_count;
            int                     m_fdCount;
            struct timeval          m_timeout;
        }
    }
}

#endif  // __havokmud_thread_Connection__
