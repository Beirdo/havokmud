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
 * @brief Player object
 */

#ifndef __havokmud_objects_Player__
#define __havokmud_objects_Player__

#include "objects/PlayerAttributes.hpp"
#include <string>
#include <map>

namespace havokmud {
    namespace objects {
        class Account;

        typedef std::map<std::string, boost::any> AttributeSubMap;
        typedef std::map<std::string, AttributeSubMap *> AttributeMap;

        class Player
        {
        public:
            Player(const std::string &name_, int accountId_, int id_ = -1) :
                    m_id(id_), m_accountId(accountId_), m_name(name_),
                    m_attributes(this) {};
            ~Player()  {};

            static Player *findPlayer(const std::string &name);
            static Player *findPlayer(int id);
            static Player *create(const std::string &jsonResponse);

            void load();
            void save();

            int id() const  { return m_id; };
            int accountId() const  { return m_accountId; };
            const std::string &name() const  { return m_name; };

            void setReroll(int rolls);

            PlayerAttributes &attributes()  { return m_attributes; };
            void rollAbilities();
            bool isImmortal()  { return false; };

        private:
            int m_id;
            int m_accountId;
            std::string m_name;

            PlayerAttributes m_attributes;
        };
    }
}

#endif  // __havokmud_objects_Player__
