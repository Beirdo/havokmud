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

#include <string>
#include <stdarg.h>

#include "thread/SmtpThread.hpp"
#include "corefunc/Logging.hpp"
#include "objects/Settings.hpp"
#include "objects/Email.hpp"

namespace havokmud {
    namespace thread {

        void event_cb(smtp_session_t session, int event_no, void *arg, ...)
        {
            // SmtpThread *thread = static_cast<SmtpThread *>(arg);
            va_list     alist;

            va_start(alist, arg);
            switch( event_no ) {
            case SMTP_EV_CONNECT:
            case SMTP_EV_MAILSTATUS:
            case SMTP_EV_MESSAGEDATA:
            case SMTP_EV_MESSAGESENT:
            case SMTP_EV_DISCONNECT:
                break;

            case SMTP_EV_WEAK_CIPHER:
            case SMTP_EV_STARTTLS_OK:
            case SMTP_EV_INVALID_PEER_CERTIFICATE:
            case SMTP_EV_NO_PEER_CERTIFICATE:
            case SMTP_EV_WRONG_PEER_CERTIFICATE:
            case SMTP_EV_NO_CLIENT_CERTIFICATE:
                break;

            default:
                break;
            }

#if 0
            LogPrint( LOG_INFO, "SMTP event %d", event_no );
#endif

            va_end(alist);
        }

        const char *message_cb(void **buf, int *len, void *arg)
        {
            Email      *email = static_cast<Email *>(arg);

            if( !len ) {
                /* This is a buffer rewind */
                email->setIndex(0);
                return( NULL );
            }

            int bodylen = email->body().length();
            if( email->index() >= bodylen ) {
                return( NULL );
            }

            email->setIndex(bodylen);
            *len = bodylen;
            return( (const char *)email->body().c_str() );
        }

        void print_recipient_status(smtp_recipient_t recipient,
                                    const char *mailbox, void *arg)
        {
            const smtp_status_t *status;
            status = smtp_recipient_status(recipient);
            LogPrint(LG_INFO, "SMTP: %s: %d %s", mailbox, status->code, 
                     status->text );
        }

        SmtpThread::SmtpThread() : HavokThread("SMTP"), m_abort(false)
        {
            pro_initialize<SmtpThread>();
        }

        void SmtpThread::start()
        {
            bool valid = true;
            m_hostname = g_settings.get<std::string>("smtpHostname");
            m_server   = g_settings.get<std::string>("smtpServer");
            m_fromAddr = g_settings.get<std::string>("smtpFrom");

            if (m_hostname.empty()) {
                LogPrint(LG_CRIT, "No SMTP Hostname defined!");
                valid = false;
            }

            if (m_server.empty()) {
                LogPrint(LG_CRIT, "No SMTP server defined!");
                valid = false;
            }

            if (m_fromAddr.empty()) {
                LogPrint(LG_CRIT, "No SMTP From Address defined!");
                valid = false;
            }

            if (valid) {
                m_session = smtp_create_session();
                smtp_set_server(m_session, m_server.c_str());
                smtp_set_hostname(m_session, m_hostname.c_str());

                /* Ignore SIGPIPE - TODO */

                smtp_set_eventcb(m_session, event_cb, this);

                char buffer[256];
                smtp_version(buffer, 256, 0);
                LogPrint(LG_INFO, "libESMTP Version %s", buffer);
            }

            while (!m_abort)
            {
                boost::shared_ptr<Email> email = m_queue.get();
                if (!email)
                    continue;

                smtp_message_t message = smtp_add_message(m_session);
                std::string fromAddr(email->fromAddr());
                if (fromAddr.empty())
                    fromAddr = m_fromAddr;

                smtp_set_reverse_path(message, fromAddr.c_str());
                smtp_recipient_t recipient = smtp_add_recipient(message,
                        email->toAddr().c_str());;
                smtp_set_header(message, "To", NULL, NULL);
                smtp_set_header(message, "Subject", email->subject().c_str());
                
                smtp_set_messagecb(message, message_cb, email.get());
                smtp_dsn_set_notify(recipient,
                        (notify_flags)(Notify_SUCCESS | Notify_FAILURE |
                                       Notify_DELAY));

                if (!smtp_start_session(m_session)) {
                    char buf[128];
                    LogPrint(LG_INFO, "SMTP Server problem: %s",
                             smtp_strerror(smtp_errno(), buf, 128));
                } else {
                    smtp_status_t *status =
                        (smtp_status_t *)smtp_message_transfer_status(message);
                    LogPrint(LG_INFO, "SMTP: %d %s", status->code,
                             (status->text ? status->text : ""));
                    smtp_enumerate_recipients(message, print_recipient_status,
                                              NULL);
                }
            }

            smtp_destroy_session(m_session);
        }

        void SmtpThread::send(const boost::shared_ptr<Email> email)
        {
            m_queue.add(email);
        }
    }
}

