/*
 ****************************************************************************
 * paginate.c
 * Part of DaleMUD 3.? Millenium Edition
 * Re-written and re-released by Chris Lack (psycho_driver@yahoo.com)
 * You are obliged to follow the DaleMUD and Diku licenses if you use any
 * portion of this source code.
 ****************************************************************************
 */

#include "config.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "protos.h"
#include "utils.h"

/*********************************************************************
* New Pagination Code
* Michael Buselli submitted the following code for an enhanced pager
* for CircleMUD.  All functions below are his.  --JE 8 Mar 96
*
*********************************************************************/

#define PAGE_LENGTH     25
#define PAGE_WIDTH      72

#ifndef NULL
#define NULL 0
#endif

char           *ParseAnsiColors(int UsingAnsi, char *txt);

/*
* Traverse down the string until the begining of the next page has
* been reached.  Return NULL if this is the last page of the
* string.
*/
char           *next_page(char *str)
{
    int             col = 1,
                    line = 1;

    for (;; str++) {
        /*
         * If end of string, return NULL.
         */
        if (*str == '\0') {
            return NULL;
        } else if (line > PAGE_LENGTH) {
            /*
             * If we're at the start of the next page, return this fact.
             */
            return str;
        }

        /*
         * Carriage return puts us in column one.
         */
        if (*str == '\r') {
            col = 1;
        } else if (*str == '\n') {
            /*
             * Newline puts us on the next line.
             */
            line++;
        } else if (*str == '$') {
            /*
             * Skip ANSI commands in the width calculations
             */
            str += 5;
        } else if (col++ > PAGE_WIDTH) {
            /*
             * We need to check here and see if we are over the page
             * width, and if so, compensate by going to the begining of
             * the next line.
             */
            col = 1;
            line++;
        }
    }
}

/*
 * Function that returns the number of pages in the string.
 */
int count_pages(char *str)
{
    int             pages = 1;

    while ((str = next_page(str)) != NULL) {
        pages++;
    }
    return pages;
}

/*
 * The call that displays the next page.
 */
void show_string(struct descriptor_data *d, char *input)
{
    char            bigbuf[MAX_STRING_LENGTH],
                    tmpbuf[64];
    int             diff;

    one_argument(input, tmpbuf);

    /*
     * Q is for quit. :)
     */
    if (LOWER(*tmpbuf) == 'q') {
        free(d->showstr_vector);
        d->showstr_count = 0;
        if (d->showstr_head) {
            free(d->showstr_head);
            d->showstr_head = NULL;
        }
        return;
    } else if (LOWER(*tmpbuf) == 'r') {
        /*
         *  R is for refresh, so back up one page internally so we
         * can display it again.
         */
        d->showstr_page = MAX(0, d->showstr_page - 1);
    } else if (LOWER(*tmpbuf) == 'b') {
        /*
         *  B is for back, so back up two pages internally so we can
         * display the correct page here.
         */
        d->showstr_page = MAX(0, d->showstr_page - 2);
    } else if (isdigit(*tmpbuf)) {
        /*
         *  Feature to 'goto' a page.  Just type the number of the
         * page and you are there!
         */
        d->showstr_page = MAX(0, MIN(atoi(tmpbuf) - 1, d->showstr_count - 1));
    }

    /*
     * If we're displaying the last page, just send it to the
     * character, and then free up the space we used.
     */
    if (d->showstr_page + 1 >= d->showstr_count) {
        SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
                                         USE_ANSI),
                                  d->showstr_vector[d->showstr_page]), d);

        free(d->showstr_vector);
        d->showstr_count = 0;
        if (d->showstr_head) {
            free(d->showstr_head);
            d->showstr_head = NULL;
        }
    } else {
        /*
         * Or if we have more to show....
         */
        diff = ((int) d->showstr_vector[d->showstr_page + 1]) -
               ((int) d->showstr_vector[d->showstr_page]);
        strncpy(bigbuf, d->showstr_vector[d->showstr_page], diff);
        bigbuf[diff] = '\0';

        SEND_TO_Q(ParseAnsiColors(IS_SET(d->character->player.user_flags,
                                         USE_ANSI), bigbuf), d);
        d->showstr_page++;
    }
}

/*
 * This function assigns all the pointers for showstr_vector for the
 * page_string function, after showstr_vector has been allocated and
 * showstr_count set.
 */
void paginate_string(char *str, struct descriptor_data *d)
{
    int             i;

    if (d->showstr_count) {
        *(d->showstr_vector) = str;
    }

    for (i = 1; i < d->showstr_count && str; i++) {
        str = next_page(str);
        d->showstr_vector[i] = str;
    }

    d->showstr_page = 0;
}

/*
 * The call that gets the paging ball rolling...
 */
void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
    if (!d) {
        return;
    }
    if (!str || !*str) {
        SEND_TO_Q("", d);
        return;
    }

    d->showstr_count = count_pages(str);
    CREATE(d->showstr_vector, char *, d->showstr_count);

    if (keep_internal) {
        d->showstr_head = strdup(str);
        paginate_string(d->showstr_head, d);
    } else {
        paginate_string(str, d);
    }

    show_string(d, "");
}

/*
 *  The following list_ functions are for building a very large
 * dynamic string buffer which is attached the a player's descriptor
 * and then sending it out to the character via page_string.
 */

/*
 *  Initialize the list_string pointer on the character's
 * descriptor.
 */
void list_init(struct descriptor_data *d)
{
    if (d->list_string != NULL) {
#if 0
        mprintf(line_log, ONE_LINE, LOG_BEEP | LOG_ASSERT, SEV_ALL,
        		"list_init called for %s when d->list_string already "
        		"allocated", d->account.account_name);
#endif
        Log("List_ijnit called before");
    }
    d->list_size = ONE_PAGE;
    CREATE(d->list_string, char, d->list_size);
}


void list_append(struct descriptor_data *d, char *fmt, ...)
{
    va_list         va;
    int             new_size,
                    slen,
                    tlen;
    char            listbuf[MAX_STRING_LENGTH],
                   *new_list;

    if (d->list_string == NULL) {
#if 0
        mprintf(line_log, ONE_LINE, LOG_BEEP | LOG_ASSERT, SEV_ALL,
         		"list_append called for %s before list_init",
    			d->account.account_name);
#endif
        Log("List_ijnit bedfore list_init");
    }

    *listbuf = '\0';
    va_start(va, fmt);
    if (vsnprintf(listbuf, MAX_STRING_LENGTH, fmt, va) > 0) {
        slen = strlen(d->list_string);
        tlen = strlen(listbuf);
        if ((new_size = (slen + tlen + 1)) > d->list_size) {
            CREATE(new_list, char, new_size + ONE_PAGE);
            memcpy(new_list, d->list_string, slen);
            free(d->list_string);
            d->list_string = new_list;
            d->list_size = new_size + ONE_PAGE;
        }
        memcpy(d->list_string + slen, listbuf, tlen);
        d->list_string[slen + tlen] = '\0';
    }
    va_end(va);
}

void list_end(struct descriptor_data *d)
{
    if (d->list_string == NULL) {
#if 0
        mprintf(line_log, ONE_LINE, LOG_BEEP | LOG_ASSERT, SEV_ALL,
        		"list_end called for %s before list_init",
     			d->account.account_name);
#endif
        Log("List_ijnit called before list_init");
    }

    page_string(d, d->list_string, TRUE);
    d->list_size = 0;
    free(d->list_string);
    d->list_string = NULL;
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
