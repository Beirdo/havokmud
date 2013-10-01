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

#include <iostream>
#include <string>
#include <sys/time.h>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "thread/ResolveThread.hpp"
#include "corefunc/Logging.hpp"

namespace havokmud {
    namespace thread {
        using boost::asio::ip::tcp;

        ResolveThread::ResolveThread() : HavokThread("Resolve"),
                m_ioService(), m_resolver(m_ioService)
        {
            pro_initialize<ResolveThread>();
        }

        void ResolveThread::start()
        {
            prv_resolve_request();

            m_ioService.run();
        }

        void ResolveThread::prv_resolve_request()
        {
            bool repeat;

            do {
                ResolveItem item = m_queue.get();

                boost::asio::ip::address ip = item.first.address();
                LogPrint(LG_INFO, "Resolving %s", ip.to_string().c_str());

                ResolveCache::iterator it = m_cache.find(ip);
                if (it != m_cache.end()) {
                    struct timeval tv;
                    gettimeofday(&tv, NULL);

                    ResolveCacheItem cacheItem = it->second;
                    if (cacheItem.second <= tv.tv_sec) {
                        // expire it
                        LogPrint(LG_INFO, "Expiring %s from ResolveCache",
                                 ip.to_string().c_str());
                        m_cache.erase(ip);
                    } else {
                        std::string hostname = cacheItem.first;
                        LogPrint(LG_INFO, "Resolved (from cache) to %s",
                                 hostname.c_str());
                        // Call the callback
                        item.second(hostname);
                        repeat = true;
                        continue;
                    }
                }

                repeat = false;
                m_resolver.async_resolve(item.first,
                        boost::bind(&ResolveThread::prv_handle_resolve, this,
                                    boost::asio::placeholders::error,
                                    boost::asio::placeholders::iterator, item));
            } while (repeat);
        }

        void ResolveThread::prv_handle_resolve(const boost::system::error_code &e,
                                               tcp::resolver::iterator iterator,
                                               const ResolveItem &item)
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            time_t ttl = tv.tv_sec + 600;   // 5 min

            std::string hostname;
            boost::asio::ip::address ip = item.first.address();

            if (!e) {
                hostname = iterator->host_name();
                LogPrint(LG_INFO, "Resolved to %s", hostname.c_str());
            } else {
                hostname = ip.to_string();
                LogPrint(LG_INFO, "Not resolved: remains %s", hostname.c_str());
            }
            // Call the callback
            item.second(hostname);

            // Cache the reply
            ResolveCacheItem cacheItem(hostname, ttl);
            m_cache.insert(std::pair<boost::asio::ip::address, ResolveCacheItem>(ip, cacheItem));

            prv_resolve_request();
        }

        void ResolveThread::resolve(const tcp::endpoint &endpoint,
                                    ResolveCallback cb)
        {
            ResolveItem item(endpoint, cb);
            m_queue.add(item);
        }
    }
}

