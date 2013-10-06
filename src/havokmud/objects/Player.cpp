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

namespace havokmud {
    namespace objects {
        int Player::s_nextId = 0;

        Player *Player::findPlayer(const std::string &name)
        {
            // Database read of player by name
            return NULL;
        }

        Player *Player::findPlayer(int id)
        {
            // Database read of player by id
            return NULL;
        }

        void Player::load()
        {
            // Database load of player
        }

        void Player::save()
        {
            // Database save of player
        }

        void Player::rollAbilities()
        {
        }

    }
}

