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
 * @brief Email object
 */

#ifndef __havokmud_thread_Email__
#define __havokmud_thread_Email__

#include <boost/enable_shared_from_this.hpp>
#include <string>

#include "thread/SmtpThread.hpp"


namespace havokmud {
    namespace thread {
        class SmtpThread;
    }
}

extern havokmud::thread::SmtpThread *g_smtpThread;

namespace havokmud {
    namespace objects {

        class Email : public boost::enable_shared_from_this<Email>
        {
        public:
            Email(const std::string &toAddr_, const std::string &subject_,
                  const std::string &body_,
                  const std::string &fromAddr_ = std::string()) :
                    m_toAddr(toAddr_), m_fromAddr(fromAddr_),
                    m_subject(subject_), m_body(body_), m_index(0)  {};
            ~Email()  {};

            const std::string &toAddr() const    { return m_toAddr; };
            const std::string &fromAddr() const  { return m_fromAddr; };
            const std::string &subject() const   { return m_subject; };
            const std::string &body() const      { return m_body; };
            
            void setIndex(int index)  { m_index = index; };
            int index() const  { return m_index; };

            void send()
            {
                if (g_smtpThread)
                    g_smtpThread->send(shared_from_this());
            };
        private:
            const std::string m_toAddr;
            const std::string m_fromAddr;
            const std::string m_subject;
            const std::string m_body;
            int m_index;
        };
    }
}

#endif  // __havokmud_thread_Email__
