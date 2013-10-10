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
 * @brief Thread to handle database requests 
 */

#include <string>
#include <stdarg.h>

#include "thread/DatabaseThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"
#include "objects/Email.hpp"

namespace havokmud {
    namespace thread {

        DatabaseThread::DatabaseThread() : HavokThread("Database"),
                m_abort(false)
        {
            pro_initialize<DatabaseThread>();
        }

        void DatabaseThread::start()
        {

            while (!m_abort)
            {
                boost::shared_ptr<DatabaseRequest> request = m_queue.get();
                if (!request)
                    continue;

                boost::shared_ptr<DatabaseResponse> response =
                        handleRequest(request);
                if (request->callback)
                    request->callback(response);
            }
        }
    }
}

