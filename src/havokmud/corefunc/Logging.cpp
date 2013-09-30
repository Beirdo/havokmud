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
 * @brief Logging Interface
 */

#include <string>
#include <stdarg.h>

#define _LogLevelNames_
#include "corefunc/Logging.hpp"
#include "objects/LoggingItem.hpp"
#include "objects/LockingQueue.hpp"

using havokmud::objects::LoggingItem;
havokmud::objects::LockingQueue<LoggingItem *> g_logQueue;

LogLevel g_LogLevel = LG_DEBUG;

#define LOGLINE_MAX 1024

void logPrintLine(int level, std::string file, int line,
                  std::string function, std::string format, ...)
{
    char message[LOGLINE_MAX+1];
    va_list arguments;

    va_start(arguments, format);
    vsnprintf(message, LOGLINE_MAX, format.c_str(), arguments);
    va_end(arguments);

    LoggingItem *item = new LoggingItem(level, file, line, function,
                                        std::string(message));
    g_logQueue.add(item);
}


