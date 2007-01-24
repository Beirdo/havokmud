#include "config.h"
#include "environment.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "protos.h"

/*
 * Sorted selection of the commands for quick lookup 
 */
struct radix_list radix_head[27];

/*
 * Quick reference hash table 
 */
byte            HashTable[256];

/*
 * Command list is allocated at run-time in the following order: 
 * ** name, command pointer, number, min_position, min_level, next, previous. 
 * ** the number can be anything, it's no longer really needed, although 
 * ** it is recommended that you keep them in numeric order to avoid confusion.
 * ** NOTE: next and previous MUST be defined as NULL to avoid any possible 
 * ** problems. 
 */


/*
 * This will search by name for a specific node and return a pointer to
 * it. 
 *
 * Passed is a pointer to the first node in the radix.  Any checking as to 
 * whether or not the node is valid should happen before entering here. 
 *
 * NOTE: This uses partial matching, change strncmp to strcmp for full 
 * matching 
 *
 * Return value is the node if it exists, or NULL if it does not. 
 */
NODE           *SearchForNodeByName(NODE * head, char *name, int len)
{
    register NODE  *i;

    i = head;
    while (i) {
        if (!(strncmp(i->name, name, len))) {
            return (i);
        }
        i = i->next;
    }

    return (NULL);
}


/*
 * some useful & stupid functions 
 */

int FindCommandNumber(char *cmd)
{
    int             i;
    NODE           *n;

    for (i = 0; i < 27; i++) {
        if (radix_head[i].number) {
            for (n = radix_head[i].next; n; n = n->next) {
                if (strcmp(cmd, n->name) == 0) {
                    return (n->number);
                }
            }
        }
    }

    for (i = 0; i < 27; i++) {
        if (radix_head[i].number) {
            for (n = radix_head[i].next; n; n = n->next) {
                if (is_abbrev(cmd, n->name)) {
                    return (n->number);
                }
            }
        }
    }

    return -1;
}

char           *FindCommandName(int num)
{
    int             i;
    NODE           *n;

    for (i = 0; i < 27; i++) {
        if (radix_head[i].number) {
            for (n = radix_head[i].next; n; n = n->next) {
                if (n->number == num) {
                    return n->name;
                }
            }
        }
    }

    return NULL;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
