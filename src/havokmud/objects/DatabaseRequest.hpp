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
 * @brief Database requests
 */

#ifndef __havokmud_objects_DatabaseRequest__
#define __havokmud_objects_DatabaseRequest__

#include <string>
#include <boost/thread.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace havokmud {
    namespace objects {
        class DatabaseResponse;

        typedef boost::shared_ptr<DatabaseResponse> ResponsePointer;

        typedef boost::function<void (ResponsePointer)> ResponseCallback;

        class DatabaseRequest
        {
        public:
            DatabaseRequest(const std::string &query_,
                            bool requiresResponse_ = false,
                            bool requiresInsertId_ = false) : m_query(query_),
                    m_requiresResponse(requiresResponse_),
                    m_requiresInsertId(requiresInsertId_), m_response()  {};

            ~DatabaseRequest()  {};

            boost::mutex &mutex() const    { return m_mutex; };
            bool requiresResponse() const  { return m_requiresResponse; };
            bool requiresInsertId() const  { return m_requiresInsertId; };

            const std::string &query()     { return m_query; };

            void setResponse(ResponsePointer response_)
                    { m_response = response; };
            ResponsePointer response()     { return m_response; };

        private:
            const std::string   m_query;
            bool                m_requiresResponse;
            bool                m_requiresInsertId;

            boost::mutex        m_mutex;
            ResponsePointer     m_response;
        };
    }
}

#endif  // __havokmud_objects_DatabaseRequest__
