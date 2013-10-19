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
            DatabaseResponse(const std::string &errMsg_, int errCode_,
                             const std::string &sqlState_) :
                    m_response(), m_insertId(-1), m_errMsg(errMsg_),
                    m_errCode(errCode_), m_sqlState(sqlState_)  {};
            ~DatabaseResponse()  {};

            const std::string response() { return m_response; };
            int insertId() const  { return m_insertId; };
            const std::string errMsg()  { return m_errMsg; };
            int errCode() const  { return m_errCode; };
            const std::string sqlState()  { return m_sqlState; };

        private:
            std::string m_response;
            int         m_insertId;
            std::string m_errMsg;
            int         m_errCode;
            std::string m_sqlState;
        };
    }
}

#endif  // __havokmud_objects_DatabaseResponse__
