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
 * @brief Connection handling
 */

#ifndef __havokmud_objects_Connection__
#define __havokmud_objects_Connection__

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/regex.hpp>
#include <string>
#include <queue>

#include "objects/Account.hpp"
#include "objects/Player.hpp"

namespace havokmud {
    namespace corefunc {
        class LoginStateMachine;
    }

    namespace thread {
        class InputThread;
    }

    namespace objects {

        #define MAX_BUFSIZE 8192

        class Account;
        class Player;

        using boost::asio::ip::tcp;

        class Connection :
                public boost::enable_shared_from_this<Connection>
        {
        public:
            typedef boost::shared_ptr<Connection> pointer;

            static unsigned char echo_on[];
            static unsigned char echo_off[];
            static int s_nextId;

            Connection(boost::asio::io_service &io_service,
                       unsigned int inBufferSize = MAX_BUFSIZE);
            ~Connection()  {};

            void setPlayer(Player *player_)  { m_player = player_; };
            void setAccount(Account *account_)  { m_account = account_; };
            void setLoginStateMachine(havokmud::corefunc::LoginStateMachine *sm)
                    { m_loginStateMachine = sm; };

            void start();
            void stop();

            bool isOpen()  { return m_socket.is_open(); };
            bool isSiteLocked()  { return false; };

            int id()  { return m_id; };
            tcp::socket &socket()  { return m_socket; };
            Player *player() const  { return m_player; };
            Account *account() const  { return m_account; };
            havokmud::corefunc::LoginStateMachine *loginStateMachine() const
                    { return m_loginStateMachine; };

            void handle_read(const boost::system::error_code &e,
                             std::size_t bytes_transferred);
            void handle_write(const boost::system::error_code &e);

            void send(std::string format, ...);
            void send(boost::asio::mutable_buffer buffer);
            void sendRaw(const unsigned char *data, int length);

            const std::string &hostname() const { return m_hostname; };

            void enterPlaying();
            std::string colorize(boost::smatch match, bool ansi);
        private:
            void prv_set_ip(std::string ip)  { m_ip = ip; };
            void prv_handle_resolve(std::string hostname)
                    { m_hostname = hostname; };
            void prv_sendBuffer();

            bool prv_splitLines(boost::asio::mutable_buffer &inBuffer,
                                std::string &line);

            int                             m_id;
            tcp::socket                     m_socket;
            Player                         *m_player;
            Account                        *m_account;
            havokmud::corefunc::LoginStateMachine *m_loginStateMachine;
            havokmud::thread::InputThread  *m_inputThread;

            boost::asio::const_buffer       m_inBufRemain;
            int                             m_inBufSize;
            unsigned char                  *m_inBufRaw;
            boost::asio::mutable_buffer     m_inBuf;
            std::queue<boost::asio::const_buffer *>  m_outBufQueue;
            boost::asio::const_buffer      *m_outBuf;
            const char                     *m_outBufRaw;
            std::string                     m_ip;
            std::string                     m_hostname;

            bool                            m_writing;
            static boost::regex             s_lineRegex;
            static boost::regex             s_colorRegex;
            bool                            m_colorized;
        };
    }
}

#endif  // __havokmud_objects_Connection__
