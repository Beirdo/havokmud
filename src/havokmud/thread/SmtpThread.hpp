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
 * @brief Thread to send emails via SMTP
 */

#ifndef __havokmud_thread_SmtpThread__
#define __havokmud_thread_SmtpThread__

#include <string>
#include <boost/thread.hpp>

#include "thread/HavokThread.hpp"
#include "objects/LockingQueue.hpp"

extern "C" {
#include <libesmtp.h>
}

namespace havokmud {
    namespace objects {
        class Email;
    }

    namespace thread {

        using havokmud::thread::HavokThread;
        using havokmud::objects::LockingQueue;
        using havokmud::objects::Email;

        class SmtpThread : public HavokThread
        {
        public:
            SmtpThread();
            ~SmtpThread()  {};

            virtual void start();
            void handle_stop()  { m_abort = true; };

            void send(const boost::shared_ptr<Email> email);

        private:
            smtp_session_t      m_session;

            std::string         m_hostname;
            std::string         m_server;
            std::string         m_fromAddr;

            LockingQueue<boost::shared_ptr<Email> > m_queue;
            bool                m_abort;
        };
    }
}

extern havokmud::thread::SmtpThread *g_smtpThread;

#endif  // __havokmud_thread_SmtpThread__
