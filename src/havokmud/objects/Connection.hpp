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

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <string>

namespace havokmud {
    namespace objects {

        #define MAX_BUFSIZE 8192

        class Player;

        using boost::asio::ip::tcp;

        class Connection :
                public boost::enable_shared_from_this<Connection>
        {
        public:
            typedef boost::shared_ptr<Connection> pointer;

            Connection(boost::asio::io_service &io_service,
                       unsigned int inBufferSize = MAX_BUFSIZE);
            ~Connection();

            void set_player(Player *player_)  { m_player = player_; };

            void start();
            void stop();

            tcp::socket &socket()  { return m_socket; };
            Player *player() const  { return m_player; };

            void handle_read(const boost::system::error_code &e,
                             std::size_t bytes_transferred);
            void handle_write(const boost::system::error_code &e);

            void send(boost::asio::const_buffer buffer);

        private:

            void prv_setHostname(std::string hostname)
                    { m_hostname = hostname; };
            void prv_sendBuffers();

            unsigned char *prv_splitLines(boost::asio::mutable_buffer &inBuffer,
                                          boost::asio::const_buffer &remainBuf);

            tcp::socket                     m_socket;
            Player                         *m_player;
            boost::asio::const_buffer       m_inBufRemain;
            unsigned char                  *m_inBufRaw;
            boost::asio::mutable_buffer     m_inBuf;
            std::vector<boost::asio::const_buffer>  m_outBufVector;
            std::string                     m_hostname;

            bool                            m_writing;
        };
    }
}

#endif  // __havokmud_thread_Connection__
