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
 * @brief ANSI color codes
 */

#ifndef __havokmud_objects_ThreadColors__
#define __havokmud_objects_ThreadColors__

#include <utility>      // std::pair

#define FG_BLACK      "\033[30m"
#define FG_RED        "\033[31m"
#define FG_GREEN      "\033[32m"
#define FG_BROWN      "\033[33m"
#define FG_BLUE       "\033[34m"
#define FG_MAGENTA    "\033[35m"
#define FG_CYAN       "\033[36m"
#define FG_LT_GRAY    "\033[37m"
#define FG_DK_GRAY    "\033[1;30m"
#define FG_LT_RED     "\033[1;31m"
#define FG_LT_GREEN   "\033[1;32m"
#define FG_YELLOW     "\033[1;33m"
#define FG_LT_BLUE    "\033[1;34m"
#define FG_LT_MAGENTA "\033[1;35m"
#define FG_LT_CYAN    "\033[1;36m"
#define FG_WHITE      "\033[1;37m"

#define BK_BLACK      "\033[0;40m"
#define BK_RED        "\033[0;41m"
#define BK_GREEN      "\033[0;42m"
#define BK_BROWN      "\033[0;43m"
#define BK_BLUE       "\033[0;44m"
#define BK_MAGENTA    "\033[0;45m"
#define BK_CYAN       "\033[0;46m"
#define BK_LT_GRAY    "\033[0;47m"

namespace havokmud {
    namespace objects {
        typedef std::pair<int, int> Color;

        class ThreadColors {
        public:
            explicit ThreadColors();
            ThreadColors(int bg, int fg)
                    { m_color = Color(bg, fg); };
            ~ThreadColors() {};
            const std::string &background() const
                    { return s_backgroundColors[m_color.first]; };
            const std::string &foreground() const
                    { return s_foregroundColors[m_color.second]; };
            const int backgroundNum() const { return m_color.first; };
            const int foregroundNum() const { return m_color.second; };
        private:
            static const std::string    s_backgroundColors[];
            static const std::string    s_foregroundColors[];
            static const Color s_badColors[];
            static const int s_badColorCount;
            static Color s_lastColor;

            Color   m_color;
        };
    }
}

extern havokmud::objects::ThreadColors g_defaultColor;

#endif  // __havokmud_objects_ThreadColors__
