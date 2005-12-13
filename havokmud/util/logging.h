/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2005 Gavin Hurlbut
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

/*HEADER---------------------------------------------------
* $Id: logging.h 41 2004-12-12 01:15:22Z gjhurlbu $
*
* Copyright 2005 Gavin Hurlbut
* All rights reserved
*
* Comments :
*
*
*--------------------------------------------------------*/
#ifndef logging_h_
#define logging_h_

/* CVS generated ID string (optional for h files) */
static char logging_h_ident[] _UNUSED_ = 
    "$Id: logging.h 41 2004-12-12 01:15:22Z gjhurlbu $";

/* Define the log levels (lower number is higher priority) */

typedef enum
{
    LOG_EMERG = 0,
    LOG_ALERT,
    LOG_CRIT,
    LOG_ERR,
    LOG_WARNING,
    LOG_NOTICE,
    LOG_INFO,
    LOG_DEBUG,
    LOG_UNKNOWN
} LogLevel_t;

#ifdef _LogLevelNames_
char *LogLevelNames[] =
{
    "LOG_EMERG",
    "LOG_ALERT",
    "LOG_CRIT",
    "LOG_ERR",
    "LOG_WARNING",
    "LOG_NOTICE",
    "LOG_INFO",
    "LOG_DEBUG",
    "LOG_UNKNOWN"
};
int LogLevelNameCount = NELEMENTS(LogLevelNames);
#else
extern char *LogLevelNames[];
extern int LogLevelNameCount;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define LogPrint(level, format, ...) \
    LogPrintLine(level, __FILE__, __LINE__, __FUNCTION__, format, \
                 ## __VA_ARGS__)

#define LogPrintNoArg(level, string) \
    LogPrintLine(level, __FILE__, __LINE__, __FUNCTION__, string)

/* Define the external prototype */
void LogPrintLine( LogLevel_t level, char *file, int line, char *function, 
                   char *format, ... );
bool LogFileAdd( char * filename );
bool LogStdoutAdd( void );
bool LogSyslogAdd( int facility );
bool LogFileRemove( char *filename );
bool LogTcpAdd( int fd );
bool LogTcpRemove( int fd );

#ifdef __cplusplus
}
#endif

#endif

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
