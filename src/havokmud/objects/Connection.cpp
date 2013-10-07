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
 * @brief Connection handler
 */

#include <iostream>
#include <stdarg.h>
#include <cstring>
#include <boost/asio/error.hpp>
#include <boost/foreach.hpp>

#include "thread/ResolveThread.hpp"
#include "objects/Connection.hpp"
#include "objects/ConnectionManager.hpp"
#include "util/misc.hpp"
#include "corefunc/Logging.hpp"
#include "thread/InputThread.hpp"
#include "thread/LoginThread.hpp"
#include "thread/PlayingThread.hpp"


// Taken from arpa/telnet.h as IP collides with boost
#define	IAC	255		/* interpret as command: */
#define	WONT	252		/* I won't use option */
#define	WILL	251		/* I will use option */
#define TELOPT_ECHO	1	/* echo */

namespace havokmud {
    namespace objects {
        unsigned char Connection::echo_on[] =
                { IAC, WONT, TELOPT_ECHO, '\r', '\n', '\0' };
        unsigned char Connection::echo_off[] =
                { IAC, WILL, TELOPT_ECHO, '\0' };

        int Connection::s_nextId = 1;

        Connection::Connection(boost::asio::io_service &io_service,
                               unsigned int inBufferSize) :
            m_id(s_nextId++),
            m_socket(io_service),
            m_inBufSize(inBufferSize),
            m_inBufRaw(new unsigned char[inBufferSize]),
            m_inBuf(boost::asio::buffer(m_inBufRaw, inBufferSize)),
            m_hostname("unknown")
        {
            // Force a power of two size
            assert(IS_POWER_2(inBufferSize));

            // Set writing state to false
            m_writing = false;
        }

        void Connection::start()
        {
            // Resolve the IP
            boost::system::error_code ec;
            tcp::endpoint endpoint = m_socket.remote_endpoint(ec);
            if (!ec && g_resolveThread) {
                boost::asio::ip::address ip(endpoint.address());
                prv_set_ip(ip.to_string());

                // Send off the hostname for resolution
                g_resolveThread->resolve(endpoint,
                        boost::bind(&Connection::prv_handle_resolve,
                                    shared_from_this(), _1));
            }

            m_inputThread = g_loginThread;

            LogPrint(LG_INFO, "Connection::start: %s (%s)", m_ip.c_str(),
                     m_hostname.c_str());

            g_loginThread->initialize(shared_from_this());

            // Start reading
            m_socket.async_read_some(boost::asio::buffer(m_inBuf),
                    boost::bind(&Connection::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void Connection::stop()
        {
            if (m_socket.is_open()) {
                LogPrint(LG_INFO, "Connection::stop: %s (%s)", m_ip.c_str(),
                         m_hostname.c_str());
                m_socket.close();
                m_inputThread->removeConnection(shared_from_this());
                m_inputThread = NULL;
            }
        }

        void Connection::handle_read(const boost::system::error_code &e,
                                     std::size_t bytes_transferred)
        {
            if (!e)
            {
                std::vector<boost::asio::const_buffer> inBufVector;

                if (boost::asio::buffer_size(m_inBufRemain)) {
                    inBufVector.push_back(m_inBufRemain);
                }
                inBufVector.push_back(m_inBuf);

                std::vector<unsigned char> data(boost::asio::buffer_size(inBufVector));
                boost::asio::mutable_buffer inBuffer(boost::asio::buffer(data));
                boost::asio::buffer_copy(inBuffer, inBufVector);
                boost::asio::mutable_buffer remainBuf;

                memset(m_inBufRaw, 0x00, m_inBufSize);

                do {
                    std::string line;
                    if (!prv_splitLines(inBuffer, line)) {
                        break;
                    }

                    if ((unsigned char)line[0] == 0xFF) {
                        // Telnet control codes should be eaten.
                        break;
                    }

                    if ((unsigned char)line[0] == 0x0A ||
                        (unsigned char)line[0] == 0x0D) {
                        // blank line
                        line = std::string();
                    }

                    // Dispatch the line
                    //LogPrint(LG_INFO, "Enqueuing line (%d) - %s", m_id,
                    //         line.c_str());
                    if (m_inputThread) {
                        m_inputThread->enqueueInput(shared_from_this(), line);
                    }
                } while (true);

                //LogPrint(LG_INFO, "Done with lines, time to read again");

                if (boost::asio::buffer_size(inBuffer)) {
                    m_inBufRemain = inBuffer;
                } else {
                    m_inBufRemain = boost::asio::mutable_buffer((void *)"", 0);
                }

                m_socket.async_read_some(boost::asio::buffer(m_inBuf),
                            boost::bind(&Connection::handle_read,
                                shared_from_this(),
                                boost::asio::placeholders::error,
                                boost::asio::placeholders::bytes_transferred));
            } else if (e != boost::asio::error::operation_aborted) {
                LogPrint(LG_INFO, "Handle read: connection %d, e = %d",
                         m_id, *(int *)&e);
                g_connectionManager.stop(shared_from_this());
            }
        }

        void Connection::handle_write(const boost::system::error_code &e)
        {
            delete [] m_outBufRaw;
            delete m_outBuf;
            m_outBufRaw = NULL;
            m_writing = false;
            if (!e) {
                if (m_outBufQueue.size()) {
                    prv_sendBuffer();
                }
            } else if (e != boost::asio::error::operation_aborted) {
                LogPrint(LG_INFO, "Handle write: connection %d, e = %d",
                         m_id, *(int *)&e);
                g_connectionManager.stop(shared_from_this());
            }
        }
                
#if 0
                boost::system::error_code ignored_ec;
                m_socket.shutdown(tcp::socket::shutdown_both, ignored_ec);
#endif

        void Connection::send(boost::asio::mutable_buffer buffer)
        {
            int len = boost::asio::buffer_size(buffer);
            char *buf = new char[len];
            boost::asio::mutable_buffer
                *newBuffer(new boost::asio::mutable_buffer(buf, len));
            boost::asio::buffer_copy(*newBuffer, buffer);

            m_outBufQueue.push((boost::asio::const_buffer *)(newBuffer));

            if (!m_writing) {
                prv_sendBuffer();
            }
        }

#define LOGLINE_MAX 1024

        void Connection::send(std::string format, ...)
        {
            char message[LOGLINE_MAX+1];
            va_list arguments;

            va_start(arguments, format);
            vsnprintf(message, LOGLINE_MAX, format.c_str(), arguments);
            va_end(arguments);

            //LogPrint(LG_INFO, "Line to send to connection %d: %s",
            //         m_id, message);
            sendRaw((const unsigned char *)message, strlen(message));
        }

        void Connection::sendRaw(const unsigned char *data, int length)
        {
            boost::asio::mutable_buffer
                    buffer(boost::asio::buffer((void *)data, length));
            send(buffer);
        }

        void Connection::prv_sendBuffer()
        {
            m_writing = true;

            std::vector<boost::asio::const_buffer> outBufVector;
            m_outBuf = m_outBufQueue.front();
            m_outBufRaw = boost::asio::buffer_cast<const char *>(*m_outBuf);

            outBufVector.push_back(*m_outBuf);
            m_outBufQueue.pop();

            //LogPrint(LG_INFO, "Writing buffer to connection %d", m_id);
            boost::asio::async_write(m_socket, outBufVector,
                    boost::bind(&Connection::handle_write,
                                shared_from_this(),
                                boost::asio::placeholders::error));
        }

        boost::regex Connection::s_lineRegex("[ \\t]*((.+?)|(\\r?\\n))[\\s\\n\\r]*$",
                boost::regex_constants::no_mod_s);

        bool Connection::prv_splitLines(boost::asio::mutable_buffer &inBuffer,
                                        std::string &line)
        {
            char *linebuf = boost::asio::buffer_cast<char *>(inBuffer);
            int length = strlen(linebuf);
            //LogPrint(LG_DEBUG, "inBuffer length: %d", length);
            boost::cmatch match;
            if (boost::regex_search(linebuf, match, s_lineRegex)) {
                std::string inputLine(match[1].first, match[1].second);

                int offset = match[0].second - match[0].first;
                if (length > offset) {
                    inBuffer = inBuffer + offset;
                    length -= offset;
                    //LogPrint(LG_DEBUG, "Match: %d", offset);
                } else {
                    inBuffer = boost::asio::buffer((void *)"", 0);
                    length = 0;
                }
                //LogPrint(LG_DEBUG, "inBuffer length: %d", length);
                line = inputLine;
                return true;
            }

            inBuffer = boost::asio::buffer((void *)"", 0);
            line = std::string();
            return false;
        }

        void Connection::enterPlaying()
        {
            if (m_player->isImmortal()) {
                m_inputThread = g_immortalPlayingThread;
            } else {
                m_inputThread = g_playingThread;
            }
        }
    }
}

