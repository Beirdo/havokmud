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
 * @brief Text manipulation utility routines
 */

/* INCLUDE FILES */
#include "environment.h"
#include "interthread.h"
#include "memory.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "config.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";


/**
 * @brief Skip over all leading spaces in a string
 * @param string Input string
 * @return the input string less leading spaces (no copying) or NULL if it would
 *         be empty, or a NULL string was passed in
 */
char *skip_spaces(char *string)
{
    if( !string ) {
        return( NULL );
    }

    for (; *string && isspace((int)*string); string++) {
        /* 
         * Empty loop 
         */
    }

    if( !*string ) {
        return( NULL );
    }

    return (string);
}


/**
 * @brief Searches a NULL-terminated list of strings for a given string (case
 *        insensitive)
 * @param arg The string to search for
 * @param list The list of strings to search in.  NULL-terminated.
 * @param exact Whether to require exact full-word matches (TRUE) or 
 *              partial-word matches (FALSE)
 * @todo I'd like to get rid of all the string lists and replace them with
 *       balanced btrees for searching speed.
 */
int search_block(char *arg, char **list, bool exact)
{
    int length;
    int i;

    if (exact) {
        for( i = 0; *list[i] != '\n'; i++ ) {
            if( !strcasecmp( arg, list[i] ) ) {
                return( i );
            }
        }
    } else {
        length = strlen( arg );
        if( length == 0 ) {
            return( -1 );
        }

        for( i = 0; *list[i] != '\n'; i++ ) {
            if( !strncasecmp( arg, list[i], length ) ) {
                return( i );
            }
        }
    }

    return( -1 );
}


/**
 * @brief Strips carriage returns from a string
 * @param output Output buffer
 * @param input Input buffer
 */
void remove_cr(char *output, char *input)
{
    while (*input) {
        if( *input != '\r' ) {
            *output++ = *input;
        }
        input++;
    }
    *output = '\0';
}

/**
 * @brief Extracts the number from a string like "3.elf", and changes the string
 *        to be "elf" (in this example)
 * @param[in,out] name Pointer to the string to mangle, the de-numbered string
 *                    is pointed to on return (still in the same buffer)
 * @return the number from the beginning of the string.  1 if there was no
 *         number there, and 0 if what was before the "." was invalid
 */
int get_number(char **name)
{
    int             retval;
    char           *ppos;
    char           *number;


    if ((ppos = strchr(*name, '.')) && ppos[1]) {
        *ppos++ = '\0';
        number = strdup( *name );
        *name = ppos;

        retval = (int)strtol( number, &ppos, 10 );
        memfree( number );

        if( ppos ) {
            /*
             * Wasn't completely numeric, invalid
             */
            return( 0 );
        }

        return( retval );
    }

    /*
     * No number found, use a count of 1
     */
    return (1);
}



/*************************************************************************
 * Support for different platforms
 *************************************************************************/

#ifndef HAVE_STRNLEN
/* FreeBSD and Solaris seem to be missing strnlen */

size_t strnlen(const char *s, size_t maxlen) 
{
    size_t len;

    for( len = 0; *s && len < maxlen; s++, len++ ) {
        /* Empty loop */
    }

    return( len );
}

#endif

#ifndef HAVE_STRSEP
/* Solaris seems to be missing strsep */
char *strsep(char **stringp, const char *delim)
{
    char *start, *end, *delimstart = NULL, *del;

    if( !stringp || !delim || !*stringp ) {
        return( NULL );
    }

    start = *stringp;

    for( end = start, del = (char *)delim; *end && *del; end++ ) {
        if( *end == *del ) {
            if( del == delim ) {
                delimstart = end;
            }
            del++;
        } else {
            del = (char *)delim;
        }
    }

    if( !*end ) {
        /* It's the whole string */
        *stringp = NULL;
    } else {
        /* Found a token */
        *delimstart = '\0';
        *stringp = end;
    }

    return( start );
}
#endif

#ifndef HAVE_STRDUP
char           *strdup(const char *str)
{
    int             len;
    char           *copy;

    len = strlen(str) + 1;
    if (!(copy = CREATEN(char, len))) {
        return ((char *) NULL);
    }
    bcopy(str, copy, len);
    return (copy);
}
#endif

#ifndef HAVE_STRSTR
/*
 * Find the first occurrence of find in s.
 */
char           *strstr(register const char *s, register const char *find)
{
    register char   c,
                    sc;
    register size_t len;

    if ((c = *find++) != 0) {
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == 0) {
                    return (NULL);
                }
            } while (sc != c);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *) s);
}
#endif

#ifndef HAVE_STRNDUP
/* OSX seems to be missing strndup */
char *strndup(const char *s, size_t n)
{
    int     len;
    char   *copy;

    len = strnlen(s, n);

    if (!(copy = CREATEN(char, len+1))) {
        return ((char *) NULL);
    }
    bcopy(s, copy, len);
    copy[len] = '\0';
    return (copy);
}
#endif


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
