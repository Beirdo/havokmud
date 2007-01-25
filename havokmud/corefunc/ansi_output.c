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
#include "protos.h"
#include "externs.h"
#include "interthread.h"
#include "queue.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "ansi.h"

static char ident[] _UNUSED_ =
    "$Id$";

void str2ansi(char *p2, char *p1, int start, int stop);
char *ansi_parse(char *code);

/**
 * @todo rework this routine
 */
char           *ParseAnsiColors(int UsingAnsi, char *txt)
{
    static char     buf[MAX_STRING_LENGTH];
    char            tmp[MAX_INPUT_LENGTH + 40];

    register int    i,
                    l,
                    f = 0;

    buf[0] = '\0';
    for (i = 0, l = 0; *txt;) {
        if (*txt == '$' && 
            (toupper(*(txt + 1)) == 'C' || 
             (*(txt + 1) == '$' && toupper(*(txt + 2)) == 'C'))) {
            if (*(txt + 1) == '$') {
                txt += 3;
            } else {
                txt += 2;
            }
            str2ansi(tmp, txt, 0, 3);

            /*
             * if using ANSI 
             */
            if (UsingAnsi) {
                strcat(buf, ansi_parse(tmp));
            } else {
                /*
                 * if not using ANSI 
                 */
                strcat(buf, "");
            }

            txt += 4;
            l = strlen(buf);
            f++;
        } else {
            buf[l++] = *txt++;
        }
        buf[l] = 0;
    }
    if (f && UsingAnsi) {
        strcat(buf, ansi_parse("0007"));
    }

    return buf;
}


/**
 * @todo not sure of the reason for this function
 */
void str2ansi(char *p2, char *p1, int start, int stop)
{
    int             i,
                    j;

    if ((start > stop) || (start < 0)) {
        /* 
         * null terminate string 
         */
        p2[0] = '\0';
    } else {
        if (start == stop) {
            /* 
             * will copy only 1 char at pos=start 
             */
            p2[0] = p1[start];
            p2[1] = '\0';
        } else {
            j = 0;

            /*
             * start or (start-1) depends on start index 
             * if starting index for arrays is 0 then use start 
             * if starting index for arrays is 1 then use start-1 
             */

            for (i = start; i <= stop; i++) {
                p2[j++] = p1[i];
            }
            /* 
             * null terminate the string 
             */
            p2[j] = '\0';
        }
    }

    if (strlen(p2) + 1 > 5) {
        LogPrintNoArg( LOG_CRIT, "DOH!" );            
        /* 
         * remove this after test period 
         */
    }
}

/**
 * @todo This is a mess.  Rework
 *
 * $CMBFG, where M is modier, B is back group color and FG is fore $C0001
 * would be normal, black back, red fore. $C1411 would be bold, blue back, 
 * light yellow fore 
 */

char *ansi_parse(char *code)
{
    static char     m[MAX_STRING_LENGTH];       
    /* increased from 255 to
     * MAX 2-18 msw 
     */
    char            b[128],
                    f[128];

    if (!code) {
        return ("");            
        /* 
         * changed this from NULL to "" 2-18 msw 
         */
    }
    /*
     * do modifier 
     */
    switch (code[0]) {
    case '0':
        sprintf(m, "%s", MOD_NORMAL);
        break;
    case '1':
        sprintf(m, "%s", MOD_BOLD);
        break;
    case '2':
        sprintf(m, "%s", MOD_FAINT);
        break;
        /*
         * not used in ansi that I know of 
         */
    case '3':
        sprintf(m, "%s", MOD_NORMAL);
        break;
    case '4':
        sprintf(m, "%s", MOD_UNDERLINE);
        break;
    case '5':
        sprintf(m, "%s", MOD_BLINK);
        break;

    case '6':
        sprintf(m, "%s", MOD_REVERSE);
        break;

    default:
        sprintf(m, "%s", MOD_NORMAL);
        break;
    }

    /*
     * do back ground color 
     */
    switch (code[1]) {
    case '0':
        sprintf(b, "%s", BK_BLACK);
        break;
    case '1':
        sprintf(b, "%s", BK_RED);
        break;
    case '2':
        sprintf(b, "%s", BK_GREEN);
        break;
    case '3':
        sprintf(b, "%s", BK_BROWN);
        break;
    case '4':
        sprintf(b, "%s", BK_BLUE);
        break;
    case '5':
        sprintf(b, "%s", BK_MAGENTA);
        break;
    case '6':
        sprintf(b, "%s", BK_CYAN);
        break;
    case '7':
        sprintf(b, "%s", BK_LT_GRAY);
        break;
    default:
        sprintf(b, "%s", BK_BLACK);
        break;
    }

    /*
     * do foreground color $c0000 
     */

    switch (code[3]) {          /* 10-15 */
    case 'G':
    case 'Y':
    case 'B':
    case 'P':
    case 'C':
    case 'W':
        code[2] = '1';
    default:
        break;
    }

    switch (code[2]) {
    case '0':
        switch (code[3]) {      /* 00-09 */
        case 'X':
        case '0':
            sprintf(f, "%s", FG_BLACK);
            break;
        case 'r':
        case '1':
            sprintf(f, "%s", FG_RED);
            break;
        case 'g':
        case '2':
            sprintf(f, "%s", FG_GREEN);
            break;
        case 'y':
        case '3':
            sprintf(f, "%s", FG_BROWN);
            break;
        case 'b':
        case '4':
            sprintf(f, "%s", FG_BLUE);
            break;
        case 'p':
        case '5':
            sprintf(f, "%s", FG_MAGENTA);
            break;
        case 'c':
        case '6':
            sprintf(f, "%s", FG_CYAN);
            break;
        case 'w':
        case '7':
            sprintf(f, "%s", FG_LT_GRAY);
            break;
        case 'x':
        case '8':
            sprintf(f, "%s", FG_DK_GRAY);
            break;
        case 'R':
        case '9':
            sprintf(f, "%s", FG_LT_RED);
            break;
        default:
            sprintf(f, "%s", FG_DK_GRAY);
            break;
        }
        break;

    case '1':
        switch (code[3]) {      /* 10-15 */
        case 'G':
        case '0':
            sprintf(f, "%s", FG_LT_GREEN);
            break;
        case 'Y':
        case '1':
            sprintf(f, "%s", FG_YELLOW);
            break;
        case 'B':
        case '2':
            sprintf(f, "%s", FG_LT_BLUE);
            break;
        case 'P':
        case '3':
            sprintf(f, "%s", FG_LT_MAGENTA);
            break;
        case 'C':
        case '4':
            sprintf(f, "%s", FG_LT_CYAN);
            break;
        case 'W':
        case '5':
            sprintf(f, "%s", FG_WHITE);
            break;
        default:
            sprintf(f, "%s", FG_LT_GREEN);
            break;
        }
        break;

    default:
        sprintf(f, "%s", FG_LT_RED);
        break;
    }

    strcat(m, b);               /* add back ground */
    strcat(m, f);               /* add foreground */

    return (m);
}




/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */

