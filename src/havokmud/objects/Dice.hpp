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

#ifndef __havokmud_objects_Dice__
#define __havokmud_objects_Dice__

#include <boost/shared_ptr.hpp>

namespace havokmud {
    namespace objects {
        class Connection;

        class Dice
        {
        public:
            Dice();
            ~Dice()  {};

            int roll(int number, int size, int add, int keep,
                     boost::shared_ptr<Connection> connection);
        };
    }
}

#endif  // __havokmud_objects_Dice__
