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
#include <stdlib.h>
#include "oldstructs.h"

/* CVS generated ID string */
static char ident[] _UNUSED_ = 
    "$Id$";

/**
 * @file
 * @brief Handles loading, saving of players
 */


/**
 * @todo replace this with a macro
 */
int HasClass(struct char_data *ch, int clss)
{
    return(IS_SET(ch->player.class, clss));
}

/**
 * @todo replace this with a macro
 * @todo implement the setting of class_count on char creation, char load
 */
int HowManyClasses( struct char_data *ch )
{
    return( ch->player.class_count );
}

/**
 * @todo replace this with a macro
 */
int pc_num_class(int clss)
{
    return( 1 << clss );
}


typedef struct {
    int     maxval;
    char   *desc;
} Descrip_t;

/**
 * Descriptions of the different alignment levels
 */
static Descrip_t align_desc[] = {
    { -900, "Chaotic Evil" },
    { -500, "Neutral Evil" },
    { -351, "Lawful Evil" },
    { -100, "Chaotic Evil" },
    {  100, "True Neutral" },
    {  350, "Lawful Neutral" },
    {  500, "Chaotic Good" },
    {  900, "Neutral Good" },
    { 1000, "Lawful Good" },
    {   -1, NULL }
};

/**
 * @brief Outputs a description as looked up in a table by the value <= maxval
 * @param value value to look up in the table
 * @param table a pointer to a Descrip_t table indicating maxval and 
 *        descriptions.  Table is terminated by a NULL desc.
 * @return The appropriate description text
 * @todo Maybe reimplement this using a btree or similar device to speed up 
 *       searching?
 *
 * The values in the table give a maximum value and a description.  The table 
 * is terminated by a NULL description.  The table MUST be in numerically
 * increasing maxval order or this code will give odd results.
 */
char *GetDescription( int value, Descrip_t *table )
{
    int             i;
    Descrip_t      *desc;
    static char    *empty = "No Description";

    for( i = 0; table[i].desc; i++ ) {
        desc = &table[i];
        if( value <= desc->maxval ) {
            return( desc->desc );
        }
    }

    return( empty );
}

/**
 * @brief Outputs the description of a character's alignment
 */
char *AlignDesc(int value)
{
    return( GetDescription( value, align_desc ) );
}


struct char_data *playerFindAll( bool (*cond_func)(struct char_data *, void *),
                             void *arg,
                             bool (*callback_func)(struct char_data *, void *), 
                             void *arg2, bool *stopAfterFirst )
{
#if 0
    extern struct char_data *character_list;
    struct char_data   *ch,
                       *tmp;
    bool                ret;

    if( !cond_func || !callback_func ) {
        return( NULL );
    }

    /**
     * @todo convert to using new LinkedList methods
     */

    for (ch = character_list; ch; ch = tmp) {
        tmp = ch->next;

        if( cond_func( ch, arg ) ) {
            ret = callback_func( ch, arg2 );
            if( ret && stopAfterFirst && *stopAfterFirst ) {
                return( ch );
            }
        }
    }

#endif
    return( NULL );
}

/*
 * vim:ts=4:sw=4:ai:et:si:sts=4
 */
