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

#include "objects/Player.hpp"
#include "corefunc/Logging.hpp"
#include "thread/DatabaseThread.hpp"

namespace havokmud {
    namespace objects {

        Player *Player::findPlayer(const std::string &name)
        {
            // Database read of player by name
            std::string jsonRequest = "{\"command\":\"get player\", \"data\":"
                                      "{\"name\":\"" + name + "\"}}";
            return create(g_databaseThread->doRequest(jsonRequest));
        }

        Player *Player::findPlayer(int id)
        {
            // Database read of player by id
            std::string jsonRequest = "{\"command\":\"get player id\", "
                                      "\"data\":{\"id\":"
                                    + std::to_string(id) + "}}";
            return create(g_databaseThread->doRequest(jsonRequest));

        }

        Player *Player::create(const std::string &jsonResponse)
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

            Player *player = new Player(
                    pt.get<std::string>("name", std::string()),
                    pt.get<int>("account_id", -1),
                    pt.get<int>("id", -1));
            return player;
        }

        void Player::load()
        {
            // Database load of player - note name/id/account_id are loaded
            m_attributes.load();
        }

        void Player::save()
        {
            // Database save of player
            std::string jsonRequest = "{\"command\":\"save player\", \"data\":"
                                      "{\"id\":" + std::to_string(m_id) + ", "
                                      "\"account_id\":\""
                                    + std::to_string(m_accountId) + ", "
                                      "\"name\":\"" + m_name + "\"}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);

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

            if (pt.get<int>("insertId", -1) != -1) {
                m_id = pt.get<int>("insertId");
            }

            m_attributes.save();
        }

        void Player::rollAbilities()
        {
        }
    }
}

