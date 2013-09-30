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

#ifndef __havokmud_thread_ConnectionThread__
#define __havokmud_thread_ConnectionThread__

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

#include "thread/HavokThread.hpp"
#include "objects/Connection.hpp"

namespace havokmud {
    namespace thread {

        using boost::asio::ip::tcp;
        using havokmud::thread::HavokThread;
        using havokmud::objects::Connection;

        class ConnectionThread : public HavokThread
        {
        public:
            ConnectionThread(int port, int timeout);
            ~ConnectionThread()  {};

            void handle_stop();

        private:
            virtual void prv_start();
            void prv_start_accept();
            void prv_handle_accept(const boost::system::error_code &e);

            std::string             m_port;
            int                     m_timeout;

            boost::asio::io_service m_ioService;
            tcp::acceptor           m_acceptor;
            Connection::pointer     m_newConnection;
        };
    }
}

#endif  // __havokmud_thread_ConnectionThread__
