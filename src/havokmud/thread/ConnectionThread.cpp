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

#include <string>

#include "objects/Connection.hpp"
#include "thread/ConnectionThread.hpp"
#include "objects/ConnectionManager.hpp"

namespace havokmud {
    namespace thread {
        using boost::asio::ip::tcp;

        void ConnectionThread::prv_start()
        {
            tcp::resolver resolver(m_ioService);
            tcp::resolver::query query("0::0", m_port);
            tcp::endpoint endpoint = *resolver.resolve(query);

            m_acceptor.open(endpoint.protocol());
            m_acceptor.bind(endpoint);
            m_acceptor.listen();

            prv_start_accept();

            m_ioService.run();
        }

        void ConnectionThread::prv_start_accept()
        {
            m_newConnection.reset(new Connection(m_ioService));
            m_acceptor.async_accept(m_newConnection->socket(),
                    boost::bind(&ConnectionThread::prv_handle_accept, this,
                                boost::asio::placeholders::error));
        }

        void ConnectionThread::prv_handle_accept(const boost::system::error_code &e)
        {
            if (!m_acceptor.is_open()) {
                return;
            }

            if (!e) {
                havokmud::objects::g_connectionManager.start(m_newConnection);
            }

            prv_start_accept();
        }

        void ConnectionThread::handle_stop()
        {
            m_acceptor.close();
            havokmud::objects::g_connectionManager.stop_all();
        }
    }
}

