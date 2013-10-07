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
 * @brief Connection Manager
 */

#ifndef __havokmud_objects_ConnectionManager__
#define __havokmud_objects_ConnectionManager__

#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <string>
#include <set>

#include "objects/Connection.hpp"

namespace havokmud {
    namespace objects {
        class ConnectionManager : private std::set<Connection::pointer>,
                                  private boost::noncopyable
        {
        public:
            void start(Connection::pointer c);
            void stop(Connection::pointer c);
            void stop_all();
        };
    }
}

extern havokmud::objects::ConnectionManager g_connectionManager;

#endif  // __havokmud_objects_ConnectionManager__
