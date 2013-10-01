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
 * @brief Thread to resolve IPs to hostnames asynchronously
 */

#ifndef __havokmud_thread_ResolveThread__
#define __havokmud_thread_ResolveThread__

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>

#include "thread/HavokThread.hpp"
#include "objects/LockingQueue.hpp"

namespace havokmud {
    namespace thread {

        using boost::asio::ip::tcp;
        using havokmud::thread::HavokThread;
        using havokmud::objects::LockingQueue;

        typedef boost::function<void (std::string hostname)> ResolveCallback;
        typedef std::pair<tcp::endpoint, ResolveCallback> ResolveItem;
        typedef std::pair<std::string, time_t> ResolveCacheItem;
        typedef std::map<boost::asio::ip::address, ResolveCacheItem> ResolveCache;

        class ResolveThread : public HavokThread
        {
        public:
            ResolveThread();
            ~ResolveThread()  {};

            virtual void start();
            void handle_stop()  {};

            void resolve(const tcp::endpoint &endpoint, ResolveCallback cb);
        private:
            void prv_resolve_request();
            void prv_handle_resolve(const boost::system::error_code &ec,
                                    tcp::resolver::iterator iterator,
                                    const ResolveItem &item);

            boost::asio::io_service     m_ioService;
            tcp::resolver               m_resolver;

            LockingQueue<ResolveItem>   m_queue;
            ResolveCache                m_cache;
        };
    }
}

extern havokmud::thread::ResolveThread *g_resolveThread;

#endif  // __havokmud_thread_ResolveThread__
