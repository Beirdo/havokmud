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
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * @file
 * @brief Database response
 */

#ifndef __havokmud_objects_DatabaseResponse__
#define __havokmud_objects_DatabaseResponse__

#include <string>

namespace havokmud {
    namespace objects {
        class DatabaseResponse
        {
        public:
            DatabaseResponse(const std::string &response_, int insertId_) :
                    m_response(response_), m_insertId(insertId_)  {};
            ~DatabaseResponse()  {};

            const std::string response() { return m_response; };
            int insertId() const  { return m_insertId; };

        private:
            std::string   m_response;
            int m_insertId;
        };
    }
}

#endif  // __havokmud_objects_DatabaseResponse__
