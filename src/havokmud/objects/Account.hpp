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

#ifndef __havokmud_objects_Account__
#define __havokmud_objects_Account__

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <string>
#include "objects/Connection.hpp"

namespace havokmud {
    namespace objects {
        class Connection;
        class Player;

        class Account
        {
        public:
            Account(boost::shared_ptr<Connection> connection) : m_id(s_nextId++),
                    m_connection(connection), m_confirmed(false)  {};
            ~Account();

            static Account *findAccount(const std::string &email);
            static Account *findAccount(int id);
            static bool checkEmail(const std::string &email);
            void save();

            void addPlayer(Player *player);

            int id() const  { return m_id; };
            void setEmail(const std::string &email)  { m_email = email; };
            void createConfirmCode();
            void setAnsi(bool ansi)  { m_ansi = ansi; };
            void setConfirmed(bool confirmed)
            {
                m_confirmed = confirmed;
                if (confirmed)
                    m_confirmCode = std::string();
            };

            std::string hashPassword(const std::string &password);
            void setPassword()
            {
                m_password = m_newPassword;
                m_newPassword = std::string();
            };

            void setNewPassword(const std::string &password)
            {
                m_newPassword = hashPassword(password);
            };

            bool confirmPassword(const std::string &password)
            {
                return (!m_newPassword.empty() &&
                        m_newPassword == hashPassword(password));
            };

            bool checkPassword(const std::string &password)
            {
                return (!m_password.empty() &&
                        m_password == hashPassword(password));
            }

            const std::string &email() const   { return m_email; };
            const std::string &confirmCode() const  { return m_confirmCode; };

            bool checkConfirmCode(const std::string &code) const
            {
                return boost::iequals(code, m_confirmCode);
            };

            bool confirmed() const  { return m_confirmed; };
            bool ansi() const  { return m_ansi; };
            boost::shared_ptr<Connection> connection() const
            {
                return m_connection;
            };

        private:
            int m_id;
            boost::shared_ptr<Connection> m_connection;
            std::string m_email;
            std::string m_confirmCode;
            bool m_confirmed;
            bool m_ansi;
            std::string m_password;
            std::string m_newPassword;

            static int s_nextId;
            static boost::regex s_emailRegex;
        };
    }
}

#endif  // __havokmud_objects_Account__
