#include "config.h"
#include "environment.h"
#include "platform.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <arpa/telnet.h>
#include <stdlib.h>
#include <unistd.h>

#include "protos.h"
#include "externs.h"


/*
 * this is to determine the relative length of strings with color codes in
 * them by determining the number of $ signs used -Lennya
 */
int color_strlen(struct char_data *ch, char *arg)
{
    int             num = 0,
                    abs = 0,
                    rel = 0,
                    i = 0;

    if (!arg || !*arg) {
        return (0);
    }
    abs = strlen(arg);
    for (i = 0; i <= abs; i++) {
        if (arg[i] == '$') {
            num++;
        }
    }
    rel = abs - (6 * num);
    if (rel < 0) {
        Log("erps, oddness in color_strlen");
        return (0);
    }
    return (rel);
}

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

int find_direction(char *arg)
{
    int length;
    int i;

    length = strlen( arg );
    if( length == 0 ) {
        return( -1 );
    }

    for( i = 0; i < directionCount; i++ ) {
        if( !strncasecmp( arg, direction[i].dir, length ) ) {
            return( i );
        }
    }

    return( -1 );
}


int old_search_block(char *argument, int begin, int length, char **list,
                     int mode)
{
    int             guess,
                    found,
                    search;

    /*
     * If the word contain 0 letters, then a match is already found
     */
    found = (length < 1);

    guess = 0;

    /*
     * Search for a match
     */

    if (mode) {
        while (!found && *(list[guess]) != '\n') {
            found = (length == strlen(list[guess]));
            for (search = 0; (search < length && found); search++) {
                found = (*(argument + begin + search) ==
                        *(list[guess] + search));
            }
            guess++;
        }
    } else {
        while (!found && *(list[guess]) != '\n') {
            found = 1;
            for (search = 0; (search < length && found); search++) {
                found = (*(argument + begin + search) ==
                        *(list[guess] + search));
            }
            guess++;
        }
    }

    return (found ? guess : -1);
}


/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
