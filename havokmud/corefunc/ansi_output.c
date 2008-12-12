/*
 *  This file is part of the havokmud package
 *  Copyright (C) 2007 Gavin Hurlbut
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
 * $Id$
 *
 * Copyright 2007 Gavin Hurlbut
 * All rights reserved
 */

/**
 * @file
 * @brief Handles conversion of MUD color codes to ANSI for player connections
 */

#include "environment.h"
#include <pthread.h>
#include "oldexterns.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "ansi_vt100.h"

static char ident[] _UNUSED_ =
    "$Id$";

int ansi_parse(char *code, char *buf);

/**
 * @brief Copies the string, expanding any MUD color codes to ANSI codes in the
 *        process (or stripping them)
 * @param UsingAnsi Set to TRUE when a user wants ANSI, FALSE if not
 * @param txt The input text with MUD color codes
 * @param buf The output buffer (size of at least MAX_STRING_LENGTH) to write
 *            the expanded text into
 * @return length of the expanded string
 */
int ParseAnsiColors(bool UsingAnsi, char *txt, char *buf)
{
    char            ansibuf[255];
    char           *string;

    register int    i,
                    j,
                    found = 0;

    buf[0] = '\0';
    for (i = 0, j = 0; txt[i] && j < MAX_STRING_LENGTH; i++) {
        string = &txt[i];
        /*
         * Catch $c0001 and $$c0001 - ANSI colors
         */
        if( string[0] == '$' && 
            (toupper(string[1]) == 'C' ||
             (string[1] == '$' && toupper(string[2]) == 'C')) ) {
            /*
             * Skip over the $C or $$C
             */
            if( string[1] == '$' ) {
                i += 3;
            } else {
                i += 2;
            }
            
            if( UsingAnsi ) {
                j += ansi_parse( &txt[i], ansibuf );
                strcat( buf, ansibuf );
                found++;
            }

            /* 
             * Skip the rest of the MUD color code (4 chars, 3 here, 1 in loop)
             */
            i += 3;
        } else {
            buf[j++] = txt[i];
            /*
             * Keep it zero-terminated so strcat will work
             */
            buf[j] = '\0';
        }
    }

    if (found) {
        /*
         * Change back to normal light gray if there was a color change and
         * we are using ANSI (which is implicit in the found variable)
         */
        ansi_parse( "0007", ansibuf );
        strcat(buf, ansibuf);
    }

    return( strlen(buf) );
}


/**
 * Table of ANSI color modifiers
 */
static char *ansi_mod[] = {
    MOD_NORMAL, MOD_BOLD, MOD_FAINT, MOD_NORMAL, MOD_UNDERLINE, MOD_BLINK,
    MOD_REVERSE
};

/**
 * Table of ANSI background colors
 */
static char *ansi_bg[] = {
    BK_BLACK, BK_RED, BK_GREEN, BK_BROWN, BK_BLUE, BK_MAGENTA, BK_CYAN,
    BK_LT_GRAY
};

/**
 * Table of ANSI foreground colors without highlight
 */
static char *ansi_fg_nohighlight[] = {
    FG_BLACK, FG_RED, FG_GREEN, FG_BROWN, FG_BLUE, FG_MAGENTA, FG_CYAN,
    FG_LT_GRAY, FG_DK_GRAY, FG_LT_RED
};

/**
 * Table of ANSI foreground colors with highlight
 */
static char *ansi_fg_highlight[] = {
    FG_LT_GREEN, FG_YELLOW, FG_LT_BLUE, FG_LT_MAGENTA, FG_LT_CYAN, FG_WHITE
};

/**
 * Mapping string for highlighted foreground colors
 */
static char *ansi_highlight = "GYBPCW";

/**
 * Mapping string for non-highlighted foreground colors
 */
static char *ansi_nohighlight = "XrgybpcwxR";

/**
 * @brief Expands a MUD color code to the ANSI code
 * @param code MUD color code in form "$CMBFG", with the leading "$" stripped 
 *             off
 * @param buf Buffer to write the ANSI code into.  Buffer is assumed to be
 *            large enough to not be overrun.
 * @return length of the generated ANSI code
 *
 * The MUD color codes are in the form "$CMBFG", where "M" is a modifier,
 * "B" is the background color and "FG" is the foreground color, either by
 * numerical index or by using a letter for the color in the 4th position.
 * 
 * This function will expand these MUD color codes into ANSI codes to send to
 * the player.  It is thread-safe.  To prevent buffer overruns, it is suggested
 * that "buf" always be allocated at least 64 characters as this should hold
 * the longest possible generated ANSI string.
 */
int ansi_parse(char *code, char *buf)
{
    char           *loc;
    int             index;

    if( !buf ) {
        return( 0 );
    }

    *buf = '\0';

    if ( !code ) {
        return (0);            
    }

    /*
     * do modifier 
     */
    index = (int)(code[0]) - '0';
    if( index < 0 || index > 6 ) {
        index = 0;
    }
    strcat( buf, ansi_mod[index] );

    /*
     * do background color 
     */
    index = (int)(code[1]) - '0';
    if( index < 0 || index > 7 ) {
        index = 0;
    }
    strcat( buf, ansi_bg[index] );

    /*
     * do foreground color
     */
    loc = strchr( ansi_highlight, (int)code[3] );
    if( loc ) {
        /*
         * This color needs highlight
         */
        code[2] = '1';
    }

    switch (code[2]) {
    case '0':
        loc = strchr( ansi_nohighlight, (int)code[3] );
        if( loc ) {
            index = (int)(loc - ansi_nohighlight);
        } else {
            index = (int)(code[3]) - '0';
            if( index < 0 || index > 9 ) {
                /*
                 * map invalid colors to dark grey
                 */
                index = 8;
            }
        }
        strcat( buf, ansi_fg_nohighlight[index] );
        break;

    case '1':
        loc = strchr( ansi_highlight, (int)code[3] );
        if( loc ) {
            index = (int)(loc - ansi_highlight);
        } else {
            index = (int)(code[3]) - '0';
            if( index < 0 || index > 5 ) {
                /*
                 * map invalid colors to light green
                 */
                index = 0;
            }
        }
        strcat( buf, ansi_fg_highlight[index] );
        break;

    default:
        /*
         * Map totally invalid codes to light red
         */
        strcat( buf, FG_LT_RED );
        break;
    }

    return( strlen(buf) );
}

/**
 * @brief Reset and clear the screen
 */
void ScreenOff( PlayerStruct_t *player )
{
    SendOutput( player, VT_MARGSET, 0, player->charData->size - 1 );
    SendOutput( player, VT_HOMECLR );
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

