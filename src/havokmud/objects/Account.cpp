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
 * @brief Account object
 */

#include <boost/foreach.hpp>
#include <openssl/md5.h>

#include "objects/Account.hpp"
#include "objects/Settings.hpp"
#include "corefunc/Logging.hpp"
#include "util/md5.hpp"
#include "thread/DatabaseThread.hpp"
#include "objects/Email.hpp"
#include "thread/SmtpThread.hpp"

#include <sys/time.h>

namespace havokmud {
    namespace objects {
        AccountMap Account::s_acctMap;

        Account::~Account()
        {
            if (!m_email.empty())
                s_acctMap.erase(m_email);
        }

        Account *Account::findAccount(const std::string &email)
        {
            // Database read of account/players by email address
            std::map<std::string, Account *>::iterator it =
                    s_acctMap.find(email);
            if (it != s_acctMap.end())
                return it->second;

            std::string jsonRequest = "{\"command\":\"load account\", \"data\":"
                                      "{\"email\":\"" + email + "\"}}";
            return create(g_databaseThread->doRequest(jsonRequest));
        }

        Account *Account::findAccount(int id)
        {
            // Database read of account/players by id
            BOOST_FOREACH(AccountMap::value_type &i, s_acctMap) {
                if (i.second->id() == id)
                    return i.second;
            }

            std::string jsonRequest = "{\"command\":\"load account id\", "
                                      "\"data\":{\"id\":\""
                                    + std::to_string(id) + "\"}}";
            return create(g_databaseThread->doRequest(jsonRequest));
        }

        Account *Account::create(const std::string &jsonResponse)
        {
            std::stringstream ss;
            ss << jsonResponse;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return NULL;
            }

            if (pt.get<int>("id", -1) == -1) {
                return NULL;
            }

            Account *acct = new Account(pt.get<int>("id", -1),
                    pt.get<std::string>("email", std::string()),
                    pt.get<std::string>("confcode", std::string()),
                    pt.get<bool>("confirmed", false),
                    pt.get<bool>("ansi", false),
                    pt.get<std::string>("passwd", std::string()));
            return acct;
        }

        void Account::save()
        {
            // Database save of account/players
            std::string jsonRequest = "{\"command\":\"save account\", \"data\":"
                                      "{\"email\":\"" + m_email + "\", "
                                      "\"id\":" + std::to_string(m_id) + ", "
                                      "\"passwd\":\"" + m_password + "\", "
                                      "\"ansi\":\"" + std::to_string(m_ansi)
                                    + "\", \"confirmed\":\"" 
                                    + std::to_string(m_confirmed) + "\", "
                                      "\"confcode\":\"" + m_confirmCode
                                    + "\"}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);
            //LogPrint(LG_INFO, "Results: %s", results.c_str());

            if (results.empty())
                return;

            std::stringstream ss;
            ss << results;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return;
            }

            if (pt.get<int>("insertId", -1) == -1) {
                return;
            }

            m_id = pt.get<int>("insertId");
        }

        void Account::createConfirmCode()
        {
            std::string     buffer;
            struct timeval  now;
            void           *ctx;
            char            digest[16];
            std::string     url;
            std::string     urlText;
            std::string     confcode;
            char           *temp;

            url = g_settings.get<std::string>("webBaseUrl");

            if( m_confirmCode.empty() ) {
                /* New email, create new confcode */
                gettimeofday( &now, NULL );
                buffer = "==!" + m_email + "!==!" + std::to_string(now.tv_sec) 
                       + "!==";

                /* MD5 it */
                ctx = opiemd5init();
                opiemd5update(ctx, (unsigned char *)buffer.c_str(),
                              buffer.length());
                opiemd5final((unsigned char *)digest, ctx);

                /* Convert the MD5 digest into English words */
                char words[40];
                opiebtoe(words, digest);

                m_confirmCode = std::string(words);
                m_confirmed = false;
                save();
            }

            if( !url.empty() ) {
                confcode = std::string(m_confirmCode);
                for(int i = 0; i < confcode.length(); i++) {
                    if( confcode[i] == ' ' ) {
                        confcode[i] = '+';
                    }
                }

                urlText = "Alternatively, you can confirm your email by clicking on "
                          "the following URL:\r\n\r\n"
                          "        " + url + "/confirm?code=" + confcode 
                        + "\r\n\r\r";
            }

            buffer = "\r\n\r\nThank you for joining Havokmud.\r\n\r\n"
                     "To confirm that this email is active, please login to your "
                     "account, and enter\r\n"
                     "the following confirmation code in the account menu:\r\n\r\n"
                     "        " + m_confirmCode + "\r\n\r\n"
                     "Please note that the order of the words is important, but not "
                     "the upper/lower\r\n"
                     "case of the letters.\r\n\r\n"
                   + (url.empty() ? "" : urlText) + "Thanks.\r\n\r\n";

            boost::shared_ptr<Email> email(new Email(m_email,
                    "Havokmud confirmation email", buffer));
            g_smtpThread->send(email);
        }

        std::string Account::hashPassword(const std::string &password)
        {
            static char     hex[] = "0123456789abcdef";
            unsigned char   md[16];
            
            if( m_email.empty() || password.empty() ) {
                return std::string();
            }

            std::string realm = g_settings.get<std::string>("gameRealm");
            if (realm.empty()) {
                realm = std::string("havokmud");
            }

            std::string buf = m_email + ":" + realm + ":" + password;

            MD5((const unsigned char *)buf.c_str(), buf.length(), md);

            std::string outbuf;
            for( int i = 0; i < 16; i++ ) {
                outbuf += hex[((md[i] & 0xF0) >> 4)];
                outbuf += hex[(md[i] & 0x0F)];
            }

            return( outbuf );
        }

        boost::regex Account::s_emailRegex("(?i)([a-z][a-z0-9._\\-]+@([a-z0-9\\-]+\\.)[a-z]+)");
        bool Account::checkEmail(const std::string &email)
        {
            boost::smatch match;
            return boost::regex_match(email, match, s_emailRegex);
        }

        void Account::loadPlayers()
        {
            std::string jsonRequest = "{\"command\":\"load player list\", "
                                      "\"data\":{\"id\":\""
                                    + std::to_string(m_id) + "\"}}";
            std::string jsonResponse = g_databaseThread->doRequest(jsonRequest);

            std::stringstream ss;
            ss << jsonResponse;
            boost::property_tree::ptree pt;
            try {
                boost::property_tree::read_json(ss, pt);
            } catch (std::exception const &e) {
                LogPrint(LG_CRIT, "Error: %s", e.what());
                return;
            }

            if (pt.empty()) {
                return;
            }

            Player *player;
            if (pt.front().first != "") {
                // Single row
                player = new Player(pt.get<std::string>("name"), m_id,
                                    pt.get<int>("id"));
                addPlayer(player);
            } else {
                BOOST_FOREACH(ptree::value_type &row, pt) {
                    player = new Player(row.second.get<std::string>("name"),
                                        m_id, row.second.get<int>("id"));
                    addPlayer(player);
                }
            }
        }
    }
}

