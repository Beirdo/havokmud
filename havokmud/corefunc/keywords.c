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
*
* Comments :
*
*--------------------------------------------------------*/

/* INCLUDE FILES */
#include "environment.h"
#include "memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "oldstructs.h"
#include "structs.h"
#include "oldutils.h"
#include "logging.h"
#include "interthread.h"
#include "balanced_btree.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Deals with Keywords_t objects
 */


char *KeywordsToString( Keywords_t *key, char *separator )
{
    char       *string;
    int         i;
    int         len;
    int         seplen;
    static char *defSeparator = "-";

    if( !key ) {
        return( NULL );
    }

    if( !separator ) {
        separator = defSeparator;
    }

    len = 0;
    seplen = strlen(separator);
    for( i = 0; i < key->count; i++ ) {
        if( key->words[i] ) {
            len += key->length[i] + seplen;
        }
    }

    string = CREATEN( char, len + 1 );
    string[0] = '\0';

    for( i = 0; i < key->count; i++ ) {
        if( key->words[i] ) {
            if( string[0] != '\0' ) {
                strcat( string, separator );
            }
            strcat( string, key->words[i] );
        }
    }

    return( string );
}

Keywords_t *StringToKeywords( char *string, Keywords_t *key )
{
    char               *tmp;
    int                 len;

    if( !string ) {
        return( NULL );
    }

    if( !key ) {
        key = CREATEN( Keywords_t, 1 );
    }
    memset(key, 0, sizeof(Keywords_t));

    for( ; string && *string; string = tmp ) {
        tmp = strpbrk( string, "- \t\n\r," );
        if( tmp == string ) {
            tmp++;
            continue;
        }

        key->count++;
        key->words = (char **)realloc(key->words, key->count * sizeof(char *));
        key->length = (int *)realloc(key->length, key->count * sizeof(int));
        key->found = NULL;

        if( tmp ) {
            len = tmp - string + 1;
            tmp++;
        } else {
            len = strlen( string );
            if( string[len-1] == '.' ) {
                key->exact = TRUE;
                string[len-1] = '\0';
                len--;
            } else {
                key->exact = FALSE;
            }
        }
        key->words[key->count-1] = strndup( string, len );
        key->length[key->count-1] = len;
    }

    return( key );
}

void FreeKeywords( Keywords_t *key, bool freeRoot )
{
    int         i;

    if( !key ) {
        return;
    }

    for( i = 0; i < key->count; i++ ) {
        if( key->words && key->words[i] ) {
            free( key->words[i] );
        }
    }

    if( key->words ) {
        free( key->words );
    }

    if( key->length ) {
        free( key->length );
    }

    if( key->found ) {
        free( key->found );
    }

    if( key->description ) {
        free( key->description );
    }
    
    if( freeRoot ) {
        free( key );
    }
}

/**
 * @bug may have incorrect results if keywords are nulled out
 */
bool KeywordsMatch(Keywords_t *tofind, Keywords_t *keywords)
{
    int             i,
                    j;
    int             res;

    if( !tofind || !keywords || !keywords->found ) {
        return( FALSE );
    }

    memset( keywords->found, 0, keywords->count * sizeof(int) );

    if( tofind->exact && tofind->count != keywords->count) {
        return( FALSE );
    }

    for (i = 0; i < tofind->count; i++) {
        for (j = 0; j < keywords->count; j++) {
            if (tofind->words[i] && keywords->words[j] && !keywords->found[j]) {
                if( tofind->partial ) {
                    res = strncasecmp( tofind->words[i],
                                       keywords->words[j],
                                       tofind->length[i] );
                } else { 
                    res = strcasecmp(tofind->words[i], keywords->words[j]);
                }

                if( !res ) {
                    /* Mark it so we don't match it repeatedly */
                    keywords->found[j] = TRUE;
                    break;
                }
            }
        }

        /* didn't find that keyword, no match */
        if (j >= keywords->count) {
            return( FALSE );
        }
    }

    return( TRUE );
}


char *find_ex_description(char *word, Keywords_t *list, int count)
{
    int             i;
    Keywords_t     *key;

    if( !word || !list || !count ) {
        return( NULL );
    }

    key = StringToKeywords( word, NULL );

    for (i = 0; i < count; i++, list++) {
        if (KeywordsMatch(key, list)) {
            FreeKeywords( key, TRUE );
            return (list->description);
        }
    }

    FreeKeywords( key, TRUE );
    return (NULL);
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
