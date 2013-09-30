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
#include <cstring>
#include <boost/asio/error.hpp>

#include "objects/Connection.hpp"
#include "objects/ConnectionManager.hpp"
#include "util/misc.hpp"
#include "thread/LoggingThread.hpp"

namespace havokmud {
    namespace objects {
        boost::regex Connection::s_lineRegex("[\\s\\n\\r]*(.+?)\\s*$",
                boost::regex_constants::no_mod_s);

        Connection::Connection(boost::asio::io_service &io_service,
                               unsigned int inBufferSize) :
            m_socket(io_service),
            m_inBufSize(inBufferSize),
            m_inBufRaw(new unsigned char[inBufferSize]),
            m_inBuf(boost::asio::buffer(m_inBufRaw, inBufferSize)),
            m_hostname("unknown")
        {
            // Force a power of two size
            assert(IS_POWER_2(inBufferSize));

            boost::system::error_code ec;
            tcp::endpoint endpoint = m_socket.remote_endpoint(ec);
            if (!ec) {
                // Send off the hostname for resolution
                //g_ResolveThread.resolve(endpoint,
                //        boost::bind(&Connection::setHostname,
                //                    shared_from_this(), _1));
            }

            // Set writing state to false
            m_writing = false;
        }

        void Connection::start()
        {
            LogPrint(LG_INFO, "Connection::start");
            // Start reading
            m_socket.async_read_some(boost::asio::buffer(m_inBuf),
                    boost::bind(&Connection::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        }

        void Connection::stop()
        {
            LogPrint(LG_INFO, "Connection::stop");
            m_socket.close();
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
                    std::string line = prv_splitLines(inBuffer);
                    if (line.empty()) {
                        LogPrint(LG_INFO, "done splitting");
                        break;
                    }

                    LogPrint(LG_INFO, "Split a line: %s", line.c_str());
                    // Dispatch the line
                    // parse(line);
                } while (true);

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
                g_connectionManager.stop(shared_from_this());
            }
        }

        void Connection::handle_write(const boost::system::error_code &e)
        {
            m_writing = false;
            if (!e) {
                if (boost::asio::buffer_size(m_outBufVector)) {
                    prv_sendBuffers();
                }
            }

            if (e != boost::asio::error::operation_aborted) {
                g_connectionManager.stop(shared_from_this());
            }
        }
                
#if 0
                boost::system::error_code ignored_ec;
                m_socket.shutdown(tcp::socket::shutdown_both, ignored_ec);
#endif

        void Connection::send(boost::asio::const_buffer buffer)
        {
            m_outBufVector.push_back(buffer);

            if (!m_writing) {
                prv_sendBuffers();
            }
        }

        void Connection::prv_sendBuffers()
        {
            m_writing = true;

            std::vector<unsigned char> data(boost::asio::buffer_size(m_outBufVector));
            std::vector<boost::asio::mutable_buffer> outBuffer(1);
            boost::asio::buffer_copy(outBuffer, m_outBufVector);
            m_outBufVector.clear();

            boost::asio::async_write(m_socket, outBuffer,
                    boost::bind(&Connection::handle_write,
                                shared_from_this(),
                                boost::asio::placeholders::error));
        }

        std::string Connection::prv_splitLines(boost::asio::mutable_buffer &inBuffer)
        {
            char *line = boost::asio::buffer_cast<char *>(inBuffer);
            int length = strlen(line);
            //LogPrint(LG_DEBUG, "inBuffer length: %d", length);
            boost::cmatch match;
            if (boost::regex_search(line, match, s_lineRegex)) {
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
                return inputLine;
            }

            inBuffer = boost::asio::buffer((void *)"", 0);
            return std::string();
        }
    }
}

