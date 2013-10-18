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
#include "objects/Account.hpp"
#include "corefunc/Logging.hpp"
#include "thread/DatabaseThread.hpp"
#include "objects/Dice.hpp"

namespace havokmud {
    namespace objects {

        Player::Player(const std::string &name_, int accountId_, int id_) :
                    m_id(id_), m_name(name_), m_accountId(accountId_),
                    m_account(Account::findAccount(m_accountId)),
                    m_attributes(this)
        {
            //LogPrint(LG_INFO, "Id: %d", m_id);
        }

        Player::Player(const std::string &name_, Account *account_, int id_) :
                    m_id(id_), m_name(name_), m_account(account_),
                    m_attributes(this)
        {
            m_accountId = (m_account ? m_account->id() : -1);
            //LogPrint(LG_INFO, "Id: %d", m_id);
        }

        Player *Player::findPlayer(const std::string &name)
        {
            // Database read of player by name
            std::string jsonRequest = "{\"command\":\"find player\", \"data\":"
                                      "{\"name\":\"" + name + "\"}}";
            return create(g_databaseThread->doRequest(jsonRequest));
        }

        Player *Player::findPlayer(int id)
        {
            // Database read of player by id
            std::string jsonRequest = "{\"command\":\"find player id\", "
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
                                      "\"account_id\":"
                                    + std::to_string(m_accountId) + ", "
                                      "\"name\":\"" + m_name + "\"}}";
            std::string results = g_databaseThread->doRequest(jsonRequest);

            if (!results.empty())
            {
                LogPrint(LG_INFO, "Results: %s", results.c_str());
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
            }

            m_attributes.save();
        }

        void Player::rollAbilities()
        {
            boost::shared_ptr<Connection> connection(m_account->connection());
            int rerolls = m_attributes.get<int>("rerolls", "core-pc");
            if( !rerolls ) {
                connection->send("No rerolls remain.  Sorry.\n\r\n\r" );
                return;
            }

            m_attributes.set<int>("rerolls", "core-pc", --rerolls);
            
            connection->send("Rolling Ability scores - %d rerolls left.\n\r\n\r",
                             rerolls );

            LogPrint(LG_INFO, "Player %s, PC %s abilities roll.  %d remain.", 
                     m_account->email().c_str(), m_name.c_str(), rerolls );

            for (int i = 1; i <= 6; i++) {
                int roll = dice(4, 6, 0, 1);
                m_attributes.set<int>("roll" + std::to_string(i), "core-pc",
                                      roll);
            }
        }

        int Player::dice(int number, int size, int add, int discard)
        {
            Dice dice_;
            return dice_.roll(number, size, add, discard,
                              m_account->connection());
        }
    }
}

