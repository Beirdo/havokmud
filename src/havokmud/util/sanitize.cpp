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
 * @brief Player Attributes object
 */

#include <boost/regex.hpp>
#include <string>

#include "corefunc/Logging.hpp"
#include "util/sanitize.hpp"

namespace havokmud {
    namespace util {

        std::string sanitize(const std::string &inString)
        {
            // Change all " to \\" for the database, gets saved as \", then
            // on readback, it goes back to ".  All fricking odd.
            boost::regex regex("([\"])");
            const char *format = "(?1\\\\\\\\\\\\\\\\\\\\\")";

            //LogPrint(LG_INFO, "In string: %s", inString.c_str());
            std::string outString = boost::regex_replace(inString, regex,
                                                         format,
                                                         boost::format_all);
            //LogPrint(LG_INFO, "Out string: %s", outString.c_str());
            return outString;
        }
    }
}

