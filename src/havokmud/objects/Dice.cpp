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
 * @brief Dice object
 */

#include "objects/Dice.hpp"
#include "objects/Connection.hpp"
#include "corefunc/Logging.hpp"

#include <sys/time.h>
#include <cstdlib>
#include <set>

namespace havokmud {
    namespace objects {

        Dice::Dice()
        {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            srandom(tv.tv_sec % tv.tv_usec);
        }

        int Dice::roll(int number, int size, int add, int discard,
                       boost::shared_ptr<Connection> connection)
        {
            if(number <= 0 || size <= 1 || discard < 0 || discard > number ||
               add <= (-1 * (number - discard) * size)) {
                return( 0 );
            }

            if (connection) {
                connection->send("Rolling %dd%d%c%d, discarding low %d dice\n\r",
                           number, size, (add < 0 ? '-' : '+'), add, discard );
                connection->send("Rolled: ");
            }

            std::multiset<int> rolls;
            for (int i = 0; i < number; i++) {
                int roll = (random() % size) + 1;
                rolls.insert(roll);
                if (connection) {
                    connection->send("%d ", roll);
                }
            }

            if( connection && discard ) {
                connection->send("  Kept: ");
            }

            int sum = 0;
            int keep = number - discard;
            int i = 0;
            for(std::multiset<int>::reverse_iterator it = rolls.rbegin();
                it != rolls.rend() && i < keep; ++it, i++ ) {
                if (connection && discard) {
                    connection->send("%d ", *it);
                }
                sum += *it;
            }

            sum += add;
            if (connection) {
                if (add)
                    connection->send("  Add: %d", add);
                connection->send("  Total: %d\n\r\n\r", sum);
            }

            return (sum);
        }
    }
}

