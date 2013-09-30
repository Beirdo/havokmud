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
 * @brief HavokMud Thread base class
 */

#include <string>
#include <iostream>

#include "thread/ThreadColors.hpp"
#include "util/misc.hpp"

havokmud::thread::ThreadColors g_defaultColor(0, 1);

namespace havokmud {
    namespace thread {
        const std::string ThreadColors::s_backgroundColors[] = 
                { BK_BLACK, BK_LT_GRAY, BK_RED, BK_GREEN,
                  BK_BROWN, BK_BLUE, BK_MAGENTA, BK_CYAN };
        const std::string ThreadColors::s_foregroundColors[] =
                { FG_BLACK, FG_LT_GRAY, FG_RED, FG_GREEN,
                  FG_BROWN, FG_BLUE, FG_MAGENTA, FG_CYAN,
                  FG_DK_GRAY, FG_LT_RED, FG_LT_GREEN, FG_YELLOW,
                  FG_LT_BLUE, FG_LT_MAGENTA, FG_LT_CYAN, FG_WHITE };

        const Color ThreadColors::s_badColors[] =
                { Color(1, 0), Color(1, 10), Color(1, 11) };
        const int ThreadColors::s_badColorCount =
                NELEMS(ThreadColors::s_badColors);

        Color ThreadColors::s_lastColor = Color(0, 0);

        ThreadColors::ThreadColors()
        {
            Color color = s_lastColor;
            int bg = color.first;
            int fg = color.second;
            bool badColor = true;

            while (badColor) {
                fg++;
                if (fg == 16) {
                    bg++;
                    fg = 0;
                }

                badColor = ((bg == fg) || (fg == 15 && bg == 1) ||
                            ((fg <= 14 && fg >= 9) && (bg == fg - 7)));

                for (int i = 0; i < s_badColorCount && !badColor; i++) {
                    badColor |= (s_badColors[i].first  == bg &&
                                 s_badColors[i].second == fg);
                }
            }
            color = Color(bg, fg);
            m_color = color;
            s_lastColor = color;
        }
    }
}

